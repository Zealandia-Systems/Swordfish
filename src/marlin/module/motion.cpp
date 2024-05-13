/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * motion.cpp
 */

#include <Eigen/Core>

#include <swordfish/Controller.h>

using namespace Eigen;

using namespace swordfish;
using namespace swordfish::math;
using namespace swordfish::motion;
using namespace swordfish::status;

#include "motion.h"
#include "endstops.h"
#include "stepper.h"
#include "planner.h"
#include "temperature.h"

#include "../gcode/gcode.h"

#include "../inc/MarlinConfig.h"

#if IS_SCARA
#	include "../libs/buzzer.h"
#	include "../lcd/marlinui.h"
#endif

#if HAS_BED_PROBE
#	include "probe.h"
#endif

#if HAS_LEVELING
#	include "../feature/bedlevel/bedlevel.h"
#endif

#if ENABLED(BLTOUCH)
#	include "../feature/bltouch.h"
#endif

#if HAS_DISPLAY
#	include "../lcd/marlinui.h"
#endif

#if HAS_FILAMENT_SENSOR
#	include "../feature/runout.h"
#endif

#if ENABLED(SENSORLESS_HOMING)
#	include "../feature/tmc_util.h"
#endif

#if ENABLED(FWRETRACT)
#	include "../feature/fwretract.h"
#endif

#if ENABLED(BABYSTEP_DISPLAY_TOTAL)
#	include "../feature/babystep.h"
#endif

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../core/debug_out.h"

/**
 * axis_homed
 *   Flags that each linear axis was homed.
 *   XYZ on cartesian, ABC on delta, ABZ on SCARA.
 *
 * axis_trusted
 *   Flags that the position is trusted in each linear axis. Set when homed.
 *   Cleared whenever a stepper powers off, potentially losing its position.
 */
uint8_t axis_homed, axis_trusted; // = 0

// Relative Mode. Enable with G91, disable with G90.
bool relative_mode; // = false;

/**
 * Cartesian Current Position
 *   Used to track the native machine position as moves are queued.
 *   Used by 'line_to_current_position' to do a move after changing it.
 *   Used by 'sync_plan_position' to update 'planner.position'.
 */
Vector6f32 current_position { X_HOME_POS, Y_HOME_POS, Z_HOME_POS, 0.0, 0.0, 0.0 };

/**
 * Cartesian Destination
 *   The destination for a move, filled in by G-code movement commands,
 *   and expected by functions like 'prepare_line_to_destination'.
 *   G-codes can set destination using 'get_destination_from_command'
 */
Vector6f32 destination { 0, 0, 0, 0, 0, 0 };

// G60/G61 Position Save and Return
#if SAVED_POSITIONS
uint8_t saved_slots[(SAVED_POSITIONS + 7) >> 3];
xyz_pos_t stored_position[SAVED_POSITIONS];
#endif

// The active extruder (tool). Set with T<extruder> command.
#if HAS_MULTI_EXTRUDER || HAS_TOOL_CHANGE
uint8_t active_extruder = 0; // = 0
#endif

#if ENABLED(LCD_SHOW_E_TOTAL)
float e_move_accumulator; // = 0
#endif

// Extruder offsets
#if HAS_HOTEND_OFFSET
xyz_pos_t hotend_offset[HOTENDS]; // Initialized by settings.load()
void reset_hotend_offsets() {
	constexpr float tmp[XYZ][HOTENDS] = { HOTEND_OFFSET_X, HOTEND_OFFSET_Y, HOTEND_OFFSET_Z };
	static_assert(
			!tmp[X_AXIS][0] && !tmp[Y_AXIS][0] && !tmp[Z_AXIS][0],
			"Offsets for the first hotend must be 0.0.");
	// Transpose from [XYZ][HOTENDS] to [HOTENDS][XYZ]
	HOTEND_LOOP()
	LOOP_XYZ(a)
	hotend_offset[e][a] = tmp[a][e];
#	if ENABLED(DUAL_X_CARRIAGE)
	hotend_offset[1].x = _MAX(X2_HOME_POS, X2_MAX_POS);
#	endif
}
#endif

// The feedrate for the current move, often used as the default if
// no other feedrate is specified. Overridden for special moves.
// Set by the last G0 through G5 command's "F" parameter.
// Functions that override this for custom moves *must always* restore it!
FeedRate feedrate_mm_s = FeedRate::MillimetersPerSecond(MMM_TO_MMS(homing_feedrate_mm_m.x));

int16_t feedrate_percentage = 100;

// Cartesian conversion result goes here:
Vector6f32 cartes;

#if IS_KINEMATIC

abc_pos_t delta;

#	if HAS_SCARA_OFFSET

abc_pos_t scara_home_offset;
#	endif

#	if HAS_SOFTWARE_ENDSTOPS
float delta_max_radius, delta_max_radius_2;
#	elif IS_SCARA
constexpr float delta_max_radius = SCARA_PRINTABLE_RADIUS,
								delta_max_radius_2 = sq(SCARA_PRINTABLE_RADIUS);
#	else // DELTA
constexpr float delta_max_radius = DELTA_PRINTABLE_RADIUS,
								delta_max_radius_2 = sq(DELTA_PRINTABLE_RADIUS);
#	endif

#endif

/**
 * The workspace can be offset by some commands, or
 * these offsets may be omitted to save on computation.
 */
#if HAS_POSITION_SHIFT
// The distance that XYZ has been offset by G92. Reset by G28.
// xyz_pos_t position_shift{0};
#endif
#if HAS_HOME_OFFSET
// This offset is added to the configured home position.
// Set by M206, M428, or menu item. Saved to EEPROM.
// xyz_pos_t home_offset{0};
#endif
#if HAS_WORKSPACE_OFFSET
// The above two are combined to save on computes
// xyz_pos_t workspace_offset{0};
#endif

#if HAS_ABL_NOT_UBL
feedRate_t xy_probe_feedrate_mm_s = MMM_TO_MMS(XY_PROBE_SPEED);
#endif

/**
 * Output the current position to serial
 */

inline void report_more_positions() {
	stepper.report_positions();
	TERN_(IS_SCARA, scara_report_positions());
}

// Report the logical position for a given machine position
inline void report_logical_position(const Vector6f32& rpos) {
	const Vector6f32 lpos = toLogical(rpos);
	SERIAL_ECHOPAIR_P(X_LBL, lpos.x(), SP_Y_LBL, lpos.y(), SP_Z_LBL, lpos.z());
}

// Report the real current position according to the steppers.
// Forward kinematics and un-leveling are applied.
void report_real_position() {
	get_cartesian_from_steppers();
	Vector6f32 npos = cartes;

#if HAS_POSITION_MODIFIERS
	planner.unapply_modifiers(npos, true);
#endif

	report_logical_position(npos);
	report_more_positions();
}

inline void report_position() {
	gcode.report_state();
}

// Report the logical current position according to the most recent G-code command
void report_current_position() {
	report_position();
}

/**
 * Report the logical current position according to the most recent G-code command.
 * The planner.position always corresponds to the last G-code too. This makes M114
 * suitable for debugging kinematics and leveling while avoiding planner sync that
 * definitively interrupts the printing flow.
 */
void report_current_position_projected() {
	report_position();
}

/**
 * sync_plan_position
 *
 * Set the planner/stepper positions directly from current_position with
 * no kinematic translation. Used for homing axes and cartesian/core syncing.
 */
void sync_plan_position() {
	if (DEBUGGING(LEVELING))
		DEBUG_POS("sync_plan_position", current_position);
	planner.set_position_mm(current_position);
}

void sync_plan_position_e() {
	planner.set_e_position_mm(0);
}

/**
 * Get the stepper positions in the cartes[] array.
 * Forward kinematics are applied for DELTA and SCARA.
 *
 * The result is in the current coordinate space with
 * leveling applied. The coordinates need to be run through
 * unapply_leveling to obtain the "ideal" coordinates
 * suitable for current_position, etc.
 */
void get_cartesian_from_steppers() {
#if ENABLED(DELTA)
	forward_kinematics_DELTA(planner.get_axis_positions_mm());
#else
#	if IS_SCARA
	forward_kinematics_SCARA(
			planner.get_axis_position_degrees(A_AXIS),
			planner.get_axis_position_degrees(B_AXIS));
#	else
	cartes.x() = planner.get_axis_position_mm(Axis::X());
	cartes.y() = planner.get_axis_position_mm(Axis::Y());
#	endif
	cartes.z() = planner.get_axis_position_mm(Axis::Z());
#endif
}

/**
 * Set the current_position for an axis based on
 * the stepper positions, removing any leveling that
 * may have been applied.
 *
 * To prevent small shifts in axis position always call
 * sync_plan_position after updating axes with this.
 *
 * To keep hosts in sync, always call report_current_position
 * after updating the current_position.
 */
void set_current_from_steppers_for_axis(const AxisValue axis) {
	get_cartesian_from_steppers();

	Vector6f32 pos = cartes;

#if HAS_POSITION_MODIFIERS
	planner.unapply_modifiers(pos, true);
#endif

	if (axis == AxisValue::All) {
		current_position = pos;
	} else {
		current_position[(u8)axis] = pos[(u8)axis];
	}
}

/**
 * Move the planner to the current position from wherever it last moved
 * (or from wherever it has been told it is located).
 */
void line_to_current_position(const MachineState machine_state, const FeedRate& feed_rate /*=feedrate_mm_s*/) {
	planner.buffer_line(current_position, feed_rate, active_extruder, machine_state);
}

/**
 * Do a fast or normal move to 'destination' with an optional FR.
 *  - Move at normal speed regardless of feedrate percentage.
 *  - Extrude the specified length regardless of flow percentage.
 */
void _internal_move_to_destination(const MachineState machine_state, const std::optional<FeedRate>& feed_rate ) {
	const FeedRate old_feedrate = feedrate_mm_s;

	if (feed_rate.has_value()) {
		feedrate_mm_s = feed_rate.value();
	}

	const uint16_t old_pct = feedrate_percentage;
	feedrate_percentage = 100;

#if EXTRUDERS
	const float old_fac = planner.e_factor[active_extruder];
	planner.e_factor[active_extruder] = 1.0f;
#endif

#if IS_KINEMATIC
	if (is_fast)
		prepare_fast_move_to_destination();
	else
#endif
		prepare_line_to_destination(machine_state);

	feedrate_mm_s = old_feedrate;
	feedrate_percentage = old_pct;
#if EXTRUDERS
	planner.e_factor[active_extruder] = old_fac;
#endif
}

/**
 * Plan a move to (X, Y, Z) and set the current_position
 */
void do_blocking_move_to(const MachineState machine_state, const Vector6f32& target, const std::optional<FeedRate>& feed_rate) {
	DEBUG_SECTION(log_move, "do_blocking_move_to", DEBUGGING(LEVELING));
	if (DEBUGGING(LEVELING))
		DEBUG_XYZ("> ", target.x(), target.y(), target.z());

	/*
	const FeedRate z_feedrate = feed_rate ? *feed_rate : homing_feedrate(Axis::Z());
	const FeedRate xy_feedrate = feed_rate ? *feed_rate : FeedRate::MillimetersPerSecond(XY_PROBE_FEEDRATE_MM_S);
	*/

	const FeedRate z_feedrate = feed_rate ? *feed_rate : feedrate_mm_s;
	const FeedRate xy_feedrate = feed_rate ? *feed_rate : feedrate_mm_s;

	// If Z needs to raise, do it before moving XY
	if (current_position.z() < target.z()) {
		current_position.z() = target.z();
		line_to_current_position(machine_state, z_feedrate);
	}

	current_position.x() = target.x();
	current_position.y() = target.y();

	line_to_current_position(machine_state, xy_feedrate);

	// If Z needs to lower, do it after moving XY
	if (current_position.z() > target.z()) {
		current_position.z() = target.z();
		line_to_current_position(machine_state, z_feedrate);
	}
	planner.synchronize();
}


void do_z_clearance(const MachineState machine_state, const float& zclear, const bool z_trusted /*=true*/, const bool raise_on_untrusted /*=true*/, const bool lower_allowed /*=false*/) {
	const bool rel = raise_on_untrusted && !z_trusted;
	float zdest = zclear + (rel ? current_position.z() : 0.0f);
	if (!lower_allowed)
		NOLESS(zdest, current_position.z());

	do_blocking_move_to_z(machine_state, _MIN(zdest, Z_MAX_POS), TERN(HAS_BED_PROBE, z_probe_fast_mm_s, homing_feedrate(Axis::Z())));
}

//
// Prepare to do endstop or probe moves with custom feedrates.
//  - Save / restore current feedrate and multiplier
//
static FeedRate saved_feedrate_mm_s = FeedRate::MillimetersPerSecond(0);
static int16_t saved_feedrate_percentage;
void remember_feedrate_and_scaling() {
	saved_feedrate_mm_s = feedrate_mm_s;
	saved_feedrate_percentage = feedrate_percentage;
}
void remember_feedrate_scaling_off() {
	remember_feedrate_and_scaling();
	feedrate_percentage = 100;
}
void restore_feedrate_and_scaling() {
	feedrate_mm_s = saved_feedrate_mm_s;
	feedrate_percentage = saved_feedrate_percentage;
}

#if !UBL_SEGMENTED

FORCE_INLINE void segment_idle(millis_t& next_idle_ms) {
	const millis_t ms = millis();
	if (ELAPSED(ms, next_idle_ms)) {
		next_idle_ms = ms + 200UL;
		return idle();
	}
	thermalManager.manage_heater(); // Returns immediately on most calls
}

#	if IS_KINEMATIC

#		if IS_SCARA
/**
 * Before raising this value, use M665 S[seg_per_sec] to decrease
 * the number of segments-per-second. Default is 200. Some deltas
 * do better with 160 or lower. It would be good to know how many
 * segments-per-second are actually possible for SCARA on AVR.
 *
 * Longer segments result in less kinematic overhead
 * but may produce jagged lines. Try 0.5mm, 1.0mm, and 2.0mm
 * and compare the difference.
 */
#			define SCARA_MIN_SEGMENT_LENGTH 0.5f
#		endif

/**
 * Prepare a linear move in a DELTA or SCARA setup.
 *
 * Called from prepare_line_to_destination as the
 * default Delta/SCARA segmenter.
 *
 * This calls planner.buffer_line several times, adding
 * small incremental moves for DELTA or SCARA.
 *
 * For Unified Bed Leveling (Delta or Segmented Cartesian)
 * the ubl.line_to_destination_segmented method replaces this.
 *
 * For Auto Bed Leveling (Bilinear) with SEGMENT_LEVELED_MOVES
 * this is replaced by segmented_line_to_destination below.
 */
inline bool line_to_destination_kinematic() {

	// Get the top feedrate of the move in the XY plane
	const float scaled_fr_mm_s = MMS_SCALED(feedrate_mm_s);

	const xyze_float_t diff = destination - current_position;

	// If the move is only in Z/E don't split up the move
	if (!diff.x && !diff.y) {
		planner.buffer_line(destination, scaled_fr_mm_s, active_extruder);
		return false; // caller will update current_position
	}

	// Fail if attempting move outside printable radius
	if (!position_is_reachable(destination))
		return true;

	// Get the linear distance in XYZ
	float cartesian_mm = diff.magnitude();

	// If the move is very short, check the E move distance
	if (UNEAR_ZERO(cartesian_mm))
		cartesian_mm = ABS(diff.e);

	// No E move either? Game over.
	if (UNEAR_ZERO(cartesian_mm))
		return true;

	// Minimum number of seconds to move the given distance
	const float seconds = cartesian_mm / scaled_fr_mm_s;

	// The number of segments-per-second times the duration
	// gives the number of segments
	uint16_t segments = delta_segments_per_second * seconds;

// For SCARA enforce a minimum segment size
#		if IS_SCARA
	NOMORE(segments, cartesian_mm * RECIPROCAL(SCARA_MIN_SEGMENT_LENGTH));
#		endif

	// At least one segment is required
	NOLESS(segments, 1U);

	// The approximate length of each segment
	const float inv_segments = 1.0f / float(segments),
							cartesian_segment_mm = cartesian_mm * inv_segments;
	const xyze_float_t segment_distance = diff * inv_segments;

#		if ENABLED(SCARA_FEEDRATE_SCALING)
	const float inv_duration = scaled_fr_mm_s / cartesian_segment_mm;
#		endif

	/*
	SERIAL_ECHOPAIR("mm=", cartesian_mm);
	SERIAL_ECHOPAIR(" seconds=", seconds);
	SERIAL_ECHOPAIR(" segments=", segments);
	SERIAL_ECHOPAIR(" segment_mm=", cartesian_segment_mm);
	SERIAL_EOL();
	//*/

	// Get the current position as starting point
	xyze_pos_t raw = current_position;

	// Calculate and execute the segments
	millis_t next_idle_ms = millis() + 200UL;
	while (--segments) {
		segment_idle(next_idle_ms);
		raw += segment_distance;
		if (!planner.buffer_line(raw, scaled_fr_mm_s, active_extruder, cartesian_segment_mm
#		if ENABLED(SCARA_FEEDRATE_SCALING)
		                         ,
		                         inv_duration
#		endif
		                         ))
			break;
	}

	// Ensure last segment arrives at target location.
	planner.buffer_line(destination, scaled_fr_mm_s, active_extruder, cartesian_segment_mm
#		if ENABLED(SCARA_FEEDRATE_SCALING)
	                    ,
	                    inv_duration
#		endif
	);

	return false; // caller will update current_position
}

#	else // !IS_KINEMATIC

#		if ENABLED(SEGMENT_LEVELED_MOVES)

/**
 * Prepare a segmented move on a CARTESIAN setup.
 *
 * This calls planner.buffer_line several times, adding
 * small incremental moves. This allows the planner to
 * apply more detailed bed leveling to the full move.
 */
inline void segmented_line_to_destination(const feedRate_t& fr_mm_s, const float segment_size = LEVELED_SEGMENT_LENGTH) {

	const xyze_float_t diff = destination - current_position;

	// If the move is only in Z/E don't split up the move
	if (!diff.x && !diff.y) {
		planner.buffer_line(destination, fr_mm_s, active_extruder);
		return;
	}

	// Get the linear distance in XYZ
	// If the move is very short, check the E move distance
	// No E move either? Game over.
	float cartesian_mm = diff.magnitude();
	if (UNEAR_ZERO(cartesian_mm))
		cartesian_mm = ABS(diff.e);
	if (UNEAR_ZERO(cartesian_mm))
		return;

	// The length divided by the segment size
	// At least one segment is required
	uint16_t segments = cartesian_mm / segment_size;
	NOLESS(segments, 1U);

	// The approximate length of each segment
	const float inv_segments = 1.0f / float(segments),
							cartesian_segment_mm = cartesian_mm * inv_segments;
	const xyze_float_t segment_distance = diff * inv_segments;

#			if ENABLED(SCARA_FEEDRATE_SCALING)
	const float inv_duration = scaled_fr_mm_s / cartesian_segment_mm;
#			endif

	// SERIAL_ECHOPAIR("mm=", cartesian_mm);
	// SERIAL_ECHOLNPAIR(" segments=", segments);
	// SERIAL_ECHOLNPAIR(" segment_mm=", cartesian_segment_mm);

	// Get the raw current position as starting point
	xyze_pos_t raw = current_position;

	// Calculate and execute the segments
	millis_t next_idle_ms = millis() + 200UL;
	while (--segments) {
		segment_idle(next_idle_ms);
		raw += segment_distance;
		if (!planner.buffer_line(raw, fr_mm_s, active_extruder, cartesian_segment_mm
#			if ENABLED(SCARA_FEEDRATE_SCALING)
		                         ,
		                         inv_duration
#			endif
		                         ))
			break;
	}

	// Since segment_distance is only approximate,
	// the final move must be to the exact destination.
	planner.buffer_line(destination, fr_mm_s, active_extruder, cartesian_segment_mm
#			if ENABLED(SCARA_FEEDRATE_SCALING)
	                    ,
	                    inv_duration
#			endif
	);
}

#		endif // SEGMENT_LEVELED_MOVES

/**
 * Prepare a linear move in a Cartesian setup.
 *
 * When a mesh-based leveling system is active, moves are segmented
 * according to the configuration of the leveling system.
 *
 * Return true if 'current_position' was set to 'destination'
 */
inline bool line_to_destination_cartesian(const swordfish::status::MachineState machine_state, const float32_t accel_mm_s2) {
	FeedRate scaled_feed_rate = feedrate_mm_s;

	if (parser.feedrate_type == FeedRateType::MillimetersPerSecond) {
	  scaled_feed_rate = MMS_SCALED(feedrate_mm_s);
	}

	planner.buffer_line(
			destination,
			scaled_feed_rate,
			active_extruder,
			machine_state,
			0.0,
			accel_mm_s2);

	return false; // caller will update current_position
}

#	endif // !IS_KINEMATIC
#endif // !UBL_SEGMENTED

#if HAS_DUPLICATION_MODE
bool extruder_duplication_enabled;
#	if ENABLED(MULTI_NOZZLE_DUPLICATION)
uint8_t duplication_e_mask; // = 0
#	endif
#endif

#if ENABLED(DUAL_X_CARRIAGE)

DualXMode dual_x_carriage_mode = DEFAULT_DUAL_X_CARRIAGE_MODE;
float inactive_extruder_x = X2_MAX_POS, // Used in mode 0 & 1
		duplicate_extruder_x_offset = DEFAULT_DUPLICATION_X_OFFSET; // Used in mode 2
xyz_pos_t raised_parked_position; // Used in mode 1
bool active_extruder_parked = false; // Used in mode 1 & 2
millis_t delayed_move_time = 0; // Used in mode 1
int16_t duplicate_extruder_temp_offset = 0; // Used in mode 2
bool idex_mirrored_mode = false; // Used in mode 3

float x_home_pos(const uint8_t extruder) {
	if (extruder == 0)
		return base_home_pos(X_AXIS);
	else
		/**
		 * In dual carriage mode the extruder offset provides an override of the
		 * second X-carriage position when homed - otherwise X2_HOME_POS is used.
		 * This allows soft recalibration of the second extruder home position
		 * without firmware reflash (through the M218 command).
		 */
		return hotend_offset[1].x > 0 ? hotend_offset[1].x : X2_HOME_POS;
}

void idex_set_mirrored_mode(const bool mirr) {
	idex_mirrored_mode = mirr;
	stepper.set_directions();
}

void set_duplication_enabled(const bool dupe, const int8_t tool_index /*=-1*/) {
	extruder_duplication_enabled = dupe;
	if (tool_index >= 0)
		active_extruder = tool_index;
	stepper.set_directions();
}

void idex_set_parked(const bool park /*=true*/) {
	delayed_move_time = 0;
	active_extruder_parked = park;
	if (park)
		raised_parked_position = current_position; // Remember current raised toolhead position for use by unpark
}

/**
 * Prepare a linear move in a dual X axis setup
 *
 * Return true if current_position[] was set to destination[]
 */
inline bool dual_x_carriage_unpark() {
	if (active_extruder_parked) {
		switch (dual_x_carriage_mode) {

			case DXC_FULL_CONTROL_MODE:
				break;

			case DXC_AUTO_PARK_MODE: {
				if (current_position.e == destination.e) {
					// This is a travel move (with no extrusion)
					// Skip it, but keep track of the current position
					// (so it can be used as the start of the next non-travel move)
					if (delayed_move_time != 0xFFFFFFFFUL) {
						current_position = destination;
						NOLESS(raised_parked_position.z, destination.z);
						delayed_move_time = millis() + 1000UL;
						return true;
					}
				}
				//
				// Un-park the active extruder
				//
				const feedRate_t fr_zfast = planner.settings.max_feedrate_mm_s[Z_AXIS];
#	define CURPOS current_position
#	define RAISED raised_parked_position
				//  1. Move to the raised parked XYZ. Presumably the tool is already at XY.
				if (planner.buffer_line(RAISED.x, RAISED.y, RAISED.z, CURPOS.e, fr_zfast, active_extruder)) {
					//  2. Move to the current native XY and raised Z. Presumably this is a null move.
					if (planner.buffer_line(CURPOS.x, CURPOS.y, RAISED.z, CURPOS.e, PLANNER_XY_FEEDRATE(), active_extruder)) {
						//  3. Lower Z back down
						line_to_current_position(fr_zfast);
					}
				}
				stepper.set_directions();

				idex_set_parked(false);
				if (DEBUGGING(LEVELING))
					DEBUG_ECHOLNPGM("idex_set_parked(false)");
			} break;

			case DXC_MIRRORED_MODE:
			case DXC_DUPLICATION_MODE:
				if (active_extruder == 0) {
					xyze_pos_t new_pos = current_position;
					if (dual_x_carriage_mode == DXC_DUPLICATION_MODE)
						new_pos.x += duplicate_extruder_x_offset;
					else
						new_pos.x = inactive_extruder_x;
					// Move duplicate extruder into correct duplication position.
					if (DEBUGGING(LEVELING))
						DEBUG_ECHOLNPAIR("Set planner X", inactive_extruder_x, " ... Line to X", new_pos.x);
					planner.set_position_mm(inactive_extruder_x, current_position.y, current_position.z, current_position.e);
					if (!planner.buffer_line(new_pos, planner.settings.max_feedrate_mm_s[X_AXIS], 1))
						break;

					planner.synchronize();
					sync_plan_position();

					set_duplication_enabled(true);
					idex_set_parked(false);
					if (DEBUGGING(LEVELING))
						DEBUG_ECHOLNPGM("set_duplication_enabled(true)\nidex_set_parked(false)");
				} else if (DEBUGGING(LEVELING))
					DEBUG_ECHOLNPGM("Active extruder not 0");
				break;
		}
	}
	return false;
}

#endif // DUAL_X_CARRIAGE

/**
 * Prepare a single move and get ready for the next one
 *
 * This may result in several calls to planner.buffer_line to
 * do smaller moves for DELTA, SCARA, mesh moves, etc.
 *
 * Make sure current_position.e and destination.e are good
 * before calling or cold/lengthy extrusion may get missed.
 *
 * Before exit, current_position is set to destination.
 */
void prepare_line_to_destination(const swordfish::status::MachineState machine_state, const float32_t accel_mm_s2 /* = 0.0 */) {
	auto& motionModule = MotionModule::getInstance();
	auto& limits = motionModule.getLimits();

	Vector3f32 dest = { destination.x(), destination.y(), destination.z() };

	limits.throwIfOutside(dest);

	debug()("current_position x: ", current_position.x(), ", y: ", current_position.y(), ", z: ", current_position.z(), ", a: ", current_position.a(), ", b: ", current_position.b(), ", c: ", current_position.c());
	debug()("destination x: ", destination.x(), ", y: ", destination.y(), ", z: ", destination.z(), ", a: ", destination.a(), ", b: ", destination.b(), ", c: ", destination.c());

	if (line_to_destination_cartesian(machine_state, accel_mm_s2)) {
		return;
	}

	current_position = destination;
}

uint8_t axes_should_home(uint8_t axis_bits /*=0x07*/) {
#define SHOULD_HOME(A) TERN(HOME_AFTER_DEACTIVATE, axis_is_trusted, axis_was_homed)(A)
	// Clear test bits that are trusted
	if (TEST(axis_bits, Axis::X()) && SHOULD_HOME(Axis::X()))
		CBI(axis_bits, Axis::X());
	if (TEST(axis_bits, Axis::Y()) && SHOULD_HOME(Axis::Y()))
		CBI(axis_bits, Axis::Y());
	if (TEST(axis_bits, Axis::Z()) && SHOULD_HOME(Axis::Z()))
		CBI(axis_bits, Axis::Z());
	return axis_bits;
}

bool homing_needed_error(uint8_t axis_bits /*=0x07*/) {
	if ((axis_bits = axes_should_home(axis_bits))) {
		throw NotHomedException {};
	}
	return false;
}

/**
 * Homing bump feedrate (mm/s)
 */
FeedRate get_homing_bump_feedrate(const Axis axis) {
#if HOMING_Z_WITH_PROBE
	if (axis == Z_AXIS)
		return MMM_TO_MMS(Z_PROBE_SPEED_SLOW);
#endif
	static const uint8_t homing_bump_divisor[] PROGMEM = HOMING_BUMP_DIVISOR;
	uint8_t hbd = pgm_read_byte(&homing_bump_divisor[axis]);
	if (hbd < 1) {
		hbd = 10;
		SERIAL_ECHO_MSG("Warning: Homing Bump Divisor < 1");
	}
	return homing_feedrate(axis) / float(hbd);
}

#if ENABLED(SENSORLESS_HOMING)
/**
 * Set sensorless homing if the axis has it, accounting for Core Kinematics.
 */
sensorless_t start_sensorless_homing_per_axis(const AxisEnum axis) {
	sensorless_t stealth_states { false };

	switch (axis) {
		default:
			break;
#	if X_SENSORLESS
		case X_AXIS:
			stealth_states.x = tmc_enable_stallguard(stepperX);
#		if AXIS_HAS_STALLGUARD(X2)
			stealth_states.x2 = tmc_enable_stallguard(stepperX2);
#		endif
#		if EITHER(CORE_IS_XY, MARKFORGED_XY) && Y_SENSORLESS
			stealth_states.y = tmc_enable_stallguard(stepperY);
#		elif CORE_IS_XZ && Z_SENSORLESS
			stealth_states.z = tmc_enable_stallguard(stepperZ);
#		endif
			break;
#	endif
#	if Y_SENSORLESS
		case Y_AXIS:
			stealth_states.y = tmc_enable_stallguard(stepperY);
#		if AXIS_HAS_STALLGUARD(Y2)
			stealth_states.y2 = tmc_enable_stallguard(stepperY2);
#		endif
#		if EITHER(CORE_IS_XY, MARKFORGED_XY) && X_SENSORLESS
			stealth_states.x = tmc_enable_stallguard(stepperX);
#		elif CORE_IS_YZ && Z_SENSORLESS
			stealth_states.z = tmc_enable_stallguard(stepperZ);
#		endif
			break;
#	endif
#	if Z_SENSORLESS
		case Z_AXIS:
			stealth_states.z = tmc_enable_stallguard(stepperZ);
#		if AXIS_HAS_STALLGUARD(Z2)
			stealth_states.z2 = tmc_enable_stallguard(stepperZ2);
#		endif
#		if AXIS_HAS_STALLGUARD(Z3)
			stealth_states.z3 = tmc_enable_stallguard(stepperZ3);
#		endif
#		if AXIS_HAS_STALLGUARD(Z4)
			stealth_states.z4 = tmc_enable_stallguard(stepperZ4);
#		endif
#		if CORE_IS_XZ && X_SENSORLESS
			stealth_states.x = tmc_enable_stallguard(stepperX);
#		elif CORE_IS_YZ && Y_SENSORLESS
			stealth_states.y = tmc_enable_stallguard(stepperY);
#		endif
			break;
#	endif
	}

#	if ENABLED(SPI_ENDSTOPS)
	switch (axis) {
		case X_AXIS:
			if (ENABLED(X_SPI_SENSORLESS))
				endstops.tmc_spi_homing.x = true;
			break;
		case Y_AXIS:
			if (ENABLED(Y_SPI_SENSORLESS))
				endstops.tmc_spi_homing.y = true;
			break;
		case Z_AXIS:
			if (ENABLED(Z_SPI_SENSORLESS))
				endstops.tmc_spi_homing.z = true;
			break;
		default:
			break;
	}
#	endif

	TERN_(IMPROVE_HOMING_RELIABILITY, sg_guard_period = millis() + default_sg_guard_duration);

	return stealth_states;
}

void end_sensorless_homing_per_axis(const AxisEnum axis, sensorless_t enable_stealth) {
	switch (axis) {
		default:
			break;
#	if X_SENSORLESS
		case X_AXIS:
			tmc_disable_stallguard(stepperX, enable_stealth.x);
#		if AXIS_HAS_STALLGUARD(X2)
			tmc_disable_stallguard(stepperX2, enable_stealth.x2);
#		endif
#		if EITHER(CORE_IS_XY, MARKFORGED_XY) && Y_SENSORLESS
			tmc_disable_stallguard(stepperY, enable_stealth.y);
#		elif CORE_IS_XZ && Z_SENSORLESS
			tmc_disable_stallguard(stepperZ, enable_stealth.z);
#		endif
			break;
#	endif
#	if Y_SENSORLESS
		case Y_AXIS:
			tmc_disable_stallguard(stepperY, enable_stealth.y);
#		if AXIS_HAS_STALLGUARD(Y2)
			tmc_disable_stallguard(stepperY2, enable_stealth.y2);
#		endif
#		if EITHER(CORE_IS_XY, MARKFORGED_XY) && X_SENSORLESS
			tmc_disable_stallguard(stepperX, enable_stealth.x);
#		elif CORE_IS_YZ && Z_SENSORLESS
			tmc_disable_stallguard(stepperZ, enable_stealth.z);
#		endif
			break;
#	endif
#	if Z_SENSORLESS
		case Z_AXIS:
			tmc_disable_stallguard(stepperZ, enable_stealth.z);
#		if AXIS_HAS_STALLGUARD(Z2)
			tmc_disable_stallguard(stepperZ2, enable_stealth.z2);
#		endif
#		if AXIS_HAS_STALLGUARD(Z3)
			tmc_disable_stallguard(stepperZ3, enable_stealth.z3);
#		endif
#		if AXIS_HAS_STALLGUARD(Z4)
			tmc_disable_stallguard(stepperZ4, enable_stealth.z4);
#		endif
#		if CORE_IS_XZ && X_SENSORLESS
			tmc_disable_stallguard(stepperX, enable_stealth.x);
#		elif CORE_IS_YZ && Y_SENSORLESS
			tmc_disable_stallguard(stepperY, enable_stealth.y);
#		endif
			break;
#	endif
	}

#	if ENABLED(SPI_ENDSTOPS)
	switch (axis) {
		case X_AXIS:
			if (ENABLED(X_SPI_SENSORLESS))
				endstops.tmc_spi_homing.x = false;
			break;
		case Y_AXIS:
			if (ENABLED(Y_SPI_SENSORLESS))
				endstops.tmc_spi_homing.y = false;
			break;
		case Z_AXIS:
			if (ENABLED(Z_SPI_SENSORLESS))
				endstops.tmc_spi_homing.z = false;
			break;
		default:
			break;
	}
#	endif
}

#endif // SENSORLESS_HOMING

/**
 * Home an individual linear axis
 */
void do_homing_move(const Axis axis, const float distance, const std::optional<FeedRate> feed_rate = std::nullopt, [[maybe_unused]] const bool final_approach = true) {
	DEBUG_SECTION(log_move, "do_homing_move", DEBUGGING(LEVELING));

	const FeedRate homing_feed_rate = feed_rate ? *feed_rate : homing_feedrate(axis);

	if (DEBUGGING(LEVELING)) {
		DEBUG_ECHOPAIR("...(", axis.to_char(), ", ", distance, ", ");
		if (feed_rate.has_value()) {
			DEBUG_ECHO(feed_rate.value().value());
		} else {
			DEBUG_ECHOPAIR("[", homing_feed_rate.value(), "]");
		}
		DEBUG_ECHOLNPGM(")");
	}

	// Only do some things when moving towards an endstop
	const int8_t axis_home_dir = TERN0(DUAL_X_CARRIAGE, axis == X_AXIS)
	                                 ? x_home_dir(active_extruder)
	                                 : home_dir(axis);
	const bool is_home_dir = (axis_home_dir > 0) == (distance > 0);

#if ENABLED(SENSORLESS_HOMING)
	sensorless_t stealth_states;
#endif

	if (is_home_dir) {

		if (TERN0(HOMING_Z_WITH_PROBE, axis == Z_AXIS)) {
#if ALL(HAS_HEATED_BED, WAIT_FOR_BED_HEATER)
			// Wait for bed to heat back up between probing points
			thermalManager.wait_for_bed_heating();
#endif

			TERN_(HAS_QUIET_PROBING, if (final_approach) probe.set_probing_paused(true));
		}

		// Disable stealthChop if used. Enable diag1 pin on driver.
		TERN_(SENSORLESS_HOMING, stealth_states = start_sensorless_homing_per_axis(axis));
	}

#if IS_SCARA
	// Tell the planner the axis is at 0
	current_position[axis] = 0;
	sync_plan_position();
	current_position[axis] = distance;
	line_to_current_position(home_fr_mm_s);
#else
	// Get the ABC or XYZ positions in mm
	Vector6f32 target = planner.get_axis_positions_mm();

	target[axis] = 0; // Set the single homing axis to 0
	planner.set_machine_position_mm(target); // Update the machine position

#	if HAS_DIST_MM_ARG
	const xyze_float_t cart_dist_mm { 0 };
#	endif

	// Set delta/cartesian axes directly
	target[axis] = distance; // The move will be towards the endstop
	planner.buffer_segment(target
#	if HAS_DIST_MM_ARG
	                       ,
	                       cart_dist_mm
#	endif
	                       ,
	                       homing_feed_rate, active_extruder, MachineState::Homing);
#endif

	planner.synchronize();

	if (is_home_dir) {

#if HOMING_Z_WITH_PROBE && HAS_QUIET_PROBING
		if (axis == Z_AXIS && final_approach)
			probe.set_probing_paused(false);
#endif

		endstops.validate_homing_move();

		// Re-enable stealthChop if used. Disable diag1 pin on driver.
		TERN_(SENSORLESS_HOMING, end_sensorless_homing_per_axis(axis, stealth_states));
	}
}

/**
 * Set an axis' current position to its home position (after homing).
 *
 * For Core and Cartesian robots this applies one-to-one when an
 * individual axis has been homed.
 *
 * DELTA should wait until all homing is done before setting the XYZ
 * current_position to home, because homing is a single operation.
 * In the case where the axis positions are trusted and previously
 * homed, DELTA could home to X or Y individually by moving either one
 * to the center. However, homing Z always homes XY and Z.
 *
 * SCARA should wait until all XY homing is done before setting the XY
 * current_position to home, because neither X nor Y is at home until
 * both are at home. Z can however be homed individually.
 *
 * Callers must sync the planner position after calling this!
 */
void set_axis_is_at_home(const Axis axis) {
	auto& motionManager = MotionModule::getInstance();

	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR(">>> set_axis_is_at_home(", axis.to_char(), ")");

	set_axis_trusted(axis);
	set_axis_homed(axis);

#if ENABLED(DUAL_X_CARRIAGE)
	if (axis == X_AXIS && (active_extruder == 1 || dual_x_carriage_mode == DXC_DUPLICATION_MODE)) {
		current_position.x = x_home_pos(active_extruder);
		return;
	}
#endif

#if ENABLED(MORGAN_SCARA)
	scara_set_axis_is_at_home(axis);
#elif ENABLED(DELTA)
	current_position[axis] = (axis == Z_AXIS) ? delta_height - TERN0(HAS_BED_PROBE, probe.offset.z) : base_home_pos(axis);
#else
	current_position[axis] = base_home_pos(axis);
#endif

/**
 * Z Probe Z Homing? Account for the probe's Z offset.
 */
#if HAS_BED_PROBE && Z_HOME_DIR < 0
	if (axis == Z_AXIS) {
#	if HOMING_Z_WITH_PROBE

		current_position.z -= probe.offset.z;

		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPAIR("*** Z HOMED WITH PROBE (Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN) ***\n> probe.offset.z = ", probe.offset.z);

#	else

		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPGM("*** Z HOMED TO ENDSTOP ***");

#	endif
	}
#endif

	TERN_(I2C_POSITION_ENCODERS, I2CPEM.homed(axis));

	TERN_(BABYSTEP_DISPLAY_TOTAL, babystep.reset_total(axis));

#if HAS_POSITION_SHIFT
	/*		Vector3f workOffset = motionManager.getWorkOffset();

	    workOffset[axis] = 0;

	    motionManager.setWorkOffset(workOffset);*/
#endif

	if (DEBUGGING(LEVELING)) {
#if HAS_HOME_OFFSET
		DEBUG_ECHOLNPAIR("> home_offset[", axis.to_char(), "] = ", motionManager.getHomeOffset()[axis]);
#endif
		DEBUG_POS("", current_position);
		DEBUG_ECHOLNPAIR("<<< set_axis_is_at_home(", axis.to_char(), ")");
	}
}

/**
 * Set an axis to be unhomed.
 */
void set_axis_never_homed(const Axis axis) {
	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR(">>> set_axis_never_homed(", axis.to_char(), ")");

	set_axis_untrusted(axis);
	set_axis_unhomed(axis);

	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR("<<< set_axis_never_homed(", axis.to_char(), ")");

	TERN_(I2C_POSITION_ENCODERS, I2CPEM.unhomed(axis));
}

#ifdef TMC_HOME_PHASE
/**
 * Move the axis back to its home_phase if set and driver is capable (TMC)
 *
 * Improves homing repeatability by homing to stepper coil's nearest absolute
 * phase position. Trinamic drivers use a stepper phase table with 1024 values
 * spanning 4 full steps with 256 positions each (ergo, 1024 positions).
 */
void backout_to_tmc_homing_phase(const AxisEnum axis) {
	const xyz_long_t home_phase = TMC_HOME_PHASE;

	// check if home phase is disabled for this axis.
	if (home_phase[axis] < 0)
		return;

	int16_t phasePerUStep, // TMC µsteps(phase) per Marlin µsteps
			phaseCurrent, // The TMC µsteps(phase) count of the current position
			effectorBackoutDir, // Direction in which the effector mm coordinates move away from endstop.
			stepperBackoutDir; // Direction in which the TMC µstep count(phase) move away from endstop.

#	define PHASE_PER_MICROSTEP(N) (256 / _MAX(1, N##_MICROSTEPS))

	switch (axis) {
#	ifdef X_MICROSTEPS
		case X_AXIS:
			phasePerUStep = PHASE_PER_MICROSTEP(X);
			phaseCurrent = stepperX.get_microstep_counter();
			effectorBackoutDir = -X_HOME_DIR;
			stepperBackoutDir = INVERT_X_DIR ? effectorBackoutDir : -effectorBackoutDir;
			break;
#	endif
#	ifdef Y_MICROSTEPS
		case Y_AXIS:
			phasePerUStep = PHASE_PER_MICROSTEP(Y);
			phaseCurrent = stepperY.get_microstep_counter();
			effectorBackoutDir = -Y_HOME_DIR;
			stepperBackoutDir = INVERT_Y_DIR ? effectorBackoutDir : -effectorBackoutDir;
			break;
#	endif
#	ifdef Z_MICROSTEPS
		case Z_AXIS:
			phasePerUStep = PHASE_PER_MICROSTEP(Z);
			phaseCurrent = stepperZ.get_microstep_counter();
			effectorBackoutDir = -Z_HOME_DIR;
			stepperBackoutDir = INVERT_Z_DIR ? effectorBackoutDir : -effectorBackoutDir;
			break;
#	endif
		default:
			return;
	}

	// Phase distance to nearest home phase position when moving in the backout direction from endstop(may be negative).
	int16_t phaseDelta = (home_phase[axis] - phaseCurrent) * stepperBackoutDir;

	// Check if home distance within endstop assumed repeatability noise of .05mm and warn.
	if (ABS(phaseDelta) * planner.steps_to_mm[axis] / phasePerUStep < 0.05f)
		SERIAL_ECHOLNPAIR("Selected home phase ", home_phase[axis],
		                  " too close to endstop trigger phase ", phaseCurrent,
		                  ". Pick a different phase for ", axis.to_char());

	// Skip to next if target position is behind current. So it only moves away from endstop.
	if (phaseDelta < 0)
		phaseDelta += 1024;

	// Convert TMC µsteps(phase) to whole Marlin µsteps to effector backout direction to mm
	const float mmDelta = int16_t(phaseDelta / phasePerUStep) * effectorBackoutDir * planner.steps_to_mm[axis];

	// Optional debug messages
	if (DEBUGGING(LEVELING)) {
		DEBUG_ECHOLNPAIR(
				"Endstop ", axis.to_char(), " hit at Phase:", phaseCurrent,
				" Delta:", phaseDelta, " Distance:", mmDelta);
	}

	if (mmDelta != 0) {
		// Retrace by the amount computed in mmDelta.
		do_homing_move(axis, mmDelta, get_homing_bump_feedrate(axis));
	}
}
#endif

/**
 * Home an individual "raw axis" to its endstop.
 * This applies to XYZ on Cartesian and Core robots, and
 * to the individual ABC steppers on DELTA and SCARA.
 *
 * At the end of the procedure the axis is marked as
 * homed and the current position of that axis is updated.
 * Kinematic robots should wait till all axes are homed
 * before updating the current position.
 */

void homeaxis(const Axis axis) {
	/*#if ENABLED(TMC_DEBUG)
	  REMEMBER(tmc_serial_port,report_tmc_status_serial_port,1);
	  REMEMBER(tmc_interval,report_tmc_status_interval,10);
	#endif
	*/

	#define _CAN_HOME(A) (axis == _AXIS(A) && (ENABLED(A##_SPI_SENSORLESS) || (_AXIS(A) == Axis::Z() && ENABLED(HOMING_Z_WITH_PROBE)) || (A##_MIN_PIN > -1 && A##_HOME_DIR < 0) || (A##_MAX_PIN > -1 && A##_HOME_DIR > 0)))

	if (!_CAN_HOME(X) && !_CAN_HOME(Y) && !_CAN_HOME(Z))
		return;

	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR(">>> homeaxis(", axis.to_char(), ")");

	const int axis_home_dir = home_dir(axis);

	const bool use_probe_bump = false;
	const float bump = axis_home_dir * home_bump_mm(axis);

	//
	// Fast move towards endstop until triggered
	//
	const float move_length = 1.5f * max_length(TERN(DELTA, Z_AXIS, axis)) * axis_home_dir;

	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR("Home Fast: ", move_length, "mm");
	do_homing_move(axis, move_length, homing_feedrate(axis), !use_probe_bump);

	// If a second homing move is configured...
	if (bump) {
		// Move away from the endstop by the axis HOMING_BUMP_MM
		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPAIR("Move Away: ", -bump, "mm");
		do_homing_move(axis, -bump, homing_feedrate(axis), false);

#if ENABLED(DETECT_BROKEN_ENDSTOP)
		// Check for a broken endstop
		EndstopEnum es;
		switch (axis) {
			default:
			case X_AXIS:
				es = X_ENDSTOP;
				break;
			case Y_AXIS:
				es = Y_ENDSTOP;
				break;
			case Z_AXIS:
				es = Z_ENDSTOP;
				break;
		}
		if (TEST(endstops.state(), es)) {
			SERIAL_ECHO_MSG("Bad ", axis.to_char(), " Endstop?");
			kill(GET_TEXT(MSG_KILL_HOMING_FAILED));
		}
#endif

		switch (axis.value()) {
			TERN_(X_DUAL_ENDSTOPS, case AxisValue::X:)
			TERN_(Y_DUAL_ENDSTOPS, case AxisValue::Y:)
			TERN_(Z_MULTI_ENDSTOPS, case AxisValue::Z:)
			stepper.set_separate_multi_axis(true);
			default:
				break;
		}

		// Slow move towards endstop until triggered
		const float rebump = bump * 4;
		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPAIR("Re-bump: ", rebump, "mm");
		do_homing_move(axis, rebump, get_homing_bump_feedrate(axis), true);

		switch (axis.value()) {
			TERN_(X_DUAL_ENDSTOPS, case AxisValue::X:)
			TERN_(Y_DUAL_ENDSTOPS, case AxisValue::Y:)
			TERN_(Z_MULTI_ENDSTOPS, case AxisValue::Z:)
			stepper.set_separate_multi_axis(false);
			default:
				break;
		}
	}

	// move 1mm away from the sensors
	do_homing_move(axis, -1.0 * axis_home_dir, homing_feedrate(axis), !use_probe_bump);

#if HAS_EXTRA_ENDSTOPS
	// Set flags for X, Y, Z motor locking
	switch (axis.value()) {
		TERN_(X_DUAL_ENDSTOPS, case AxisValue::X:)
		TERN_(Y_DUAL_ENDSTOPS, case AxisValue::Y:)
		TERN_(Z_MULTI_ENDSTOPS, case AxisValue::Z:)
		stepper.set_separate_multi_axis(true);
		default:
			break;
	}

	const bool pos_dir = axis_home_dir > 0;
#	if ENABLED(X_DUAL_ENDSTOPS)
	if (axis == X_AXIS) {
		const float adj = ABS(endstops.x2_endstop_adj);
		if (adj) {
			if (pos_dir ? (endstops.x2_endstop_adj > 0) : (endstops.x2_endstop_adj < 0))
				stepper.set_x_lock(true);
			else
				stepper.set_x2_lock(true);
			do_homing_move(axis, pos_dir ? -adj : adj);
			stepper.set_x_lock(false);
			stepper.set_x2_lock(false);
		}
	}
#	endif
#	if ENABLED(Y_DUAL_ENDSTOPS)
	if (axis == Axis::Y()) {
		const float adj = ABS(endstops.y2_endstop_adj);
		if (adj) {
			if (pos_dir ? (endstops.y2_endstop_adj > 0) : (endstops.y2_endstop_adj < 0))
				stepper.set_y_lock(true);
			else
				stepper.set_y2_lock(true);
			do_homing_move(axis, pos_dir ? -adj : adj);
			stepper.set_y_lock(false);
			stepper.set_y2_lock(false);
		}
	}
#	endif

#	if ENABLED(Z_MULTI_ENDSTOPS)
	if (axis == Z_AXIS) {

#		if NUM_Z_STEPPER_DRIVERS == 2

		const float adj = ABS(endstops.z2_endstop_adj);
		if (adj) {
			if (pos_dir ? (endstops.z2_endstop_adj > 0) : (endstops.z2_endstop_adj < 0))
				stepper.set_z1_lock(true);
			else
				stepper.set_z2_lock(true);
			do_homing_move(axis, pos_dir ? -adj : adj);
			stepper.set_z1_lock(false);
			stepper.set_z2_lock(false);
		}

#		else

		// Handy arrays of stepper lock function pointers

		typedef void (*adjustFunc_t)(const bool);

		adjustFunc_t lock[] = {
			stepper.set_z1_lock,
			stepper.set_z2_lock,
			stepper.set_z3_lock
#			if NUM_Z_STEPPER_DRIVERS >= 4
			,
			stepper.set_z4_lock
#			endif
		};
		float adj[] = {
			0,
			endstops.z2_endstop_adj,
			endstops.z3_endstop_adj
#			if NUM_Z_STEPPER_DRIVERS >= 4
			,
			endstops.z4_endstop_adj
#			endif
		};

		adjustFunc_t tempLock;
		float tempAdj;

		// Manual bubble sort by adjust value
		if (adj[1] < adj[0]) {
			tempLock = lock[0], tempAdj = adj[0];
			lock[0] = lock[1], adj[0] = adj[1];
			lock[1] = tempLock, adj[1] = tempAdj;
		}
		if (adj[2] < adj[1]) {
			tempLock = lock[1], tempAdj = adj[1];
			lock[1] = lock[2], adj[1] = adj[2];
			lock[2] = tempLock, adj[2] = tempAdj;
		}
#			if NUM_Z_STEPPER_DRIVERS >= 4
		if (adj[3] < adj[2]) {
			tempLock = lock[2], tempAdj = adj[2];
			lock[2] = lock[3], adj[2] = adj[3];
			lock[3] = tempLock, adj[3] = tempAdj;
		}
		if (adj[2] < adj[1]) {
			tempLock = lock[1], tempAdj = adj[1];
			lock[1] = lock[2], adj[1] = adj[2];
			lock[2] = tempLock, adj[2] = tempAdj;
		}
#			endif
		if (adj[1] < adj[0]) {
			tempLock = lock[0], tempAdj = adj[0];
			lock[0] = lock[1], adj[0] = adj[1];
			lock[1] = tempLock, adj[1] = tempAdj;
		}

		if (pos_dir) {
			// normalize adj to smallest value and do the first move
			(*lock[0])(true);
			do_homing_move(axis, adj[1] - adj[0]);
			// lock the second stepper for the final correction
			(*lock[1])(true);
			do_homing_move(axis, adj[2] - adj[1]);
#			if NUM_Z_STEPPER_DRIVERS >= 4
			// lock the third stepper for the final correction
			(*lock[2])(true);
			do_homing_move(axis, adj[3] - adj[2]);
#			endif
		} else {
#			if NUM_Z_STEPPER_DRIVERS >= 4
			(*lock[3])(true);
			do_homing_move(axis, adj[2] - adj[3]);
#			endif
			(*lock[2])(true);
			do_homing_move(axis, adj[1] - adj[2]);
			(*lock[1])(true);
			do_homing_move(axis, adj[0] - adj[1]);
		}

		stepper.set_z1_lock(false);
		stepper.set_z2_lock(false);
		stepper.set_z3_lock(false);
#			if NUM_Z_STEPPER_DRIVERS >= 4
		stepper.set_z4_lock(false);
#			endif

#		endif
	}
#	endif

	// Reset flags for X, Y, Z motor locking
	switch (axis.value()) {
		default:
			break;
			TERN_(X_DUAL_ENDSTOPS, case AxisValue::X:)
			TERN_(Y_DUAL_ENDSTOPS, case AxisValue::Y:)
			TERN_(Z_MULTI_ENDSTOPS, case AxisValue::Z:)
			stepper.set_separate_multi_axis(false);
	}
#endif

#ifdef TMC_HOME_PHASE
	// move back to homing phase if configured and capable
	backout_to_tmc_homing_phase(axis);
#endif

	set_axis_is_at_home(axis);
	sync_plan_position();

	destination[axis] = current_position[axis];

	if (DEBUGGING(LEVELING))
		DEBUG_POS("> AFTER set_axis_is_at_home", current_position);

	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR("<<< homeaxis(", axis.to_char(), ")");
} // homeaxis()

float32_t toNative(const float32_t value, Axis axis) {
	return MotionModule::getInstance().toNative(axis, value);
}

float32_t toLogical(const float32_t value, Axis axis) {
	return MotionModule::getInstance().toLogical(axis, value);
}

Vector2f32 toLogical(const Vector2f32& raw) {
	return MotionModule::getInstance().toLogical(raw);
}

Vector3f32 toLogical(const Vector3f32& raw) {
	return MotionModule::getInstance().toLogical(raw);
}

Vector4f32 toLogical(const Vector4f32& raw) {
	return  MotionModule::getInstance().toLogical(raw);
}

Vector5f32 toLogical(const Vector5f32& raw) {
	return MotionModule::getInstance().toLogical(raw);
}

Vector6f32 toLogical(const Vector6f32& raw) {
	return MotionModule::getInstance().toLogical(raw);
}

Vector2f32 toNative(const Vector2f32& raw) {
	return MotionModule::getInstance().toNative(raw);
}

Vector3f32 toNative(const Vector3f32& raw) {
	return MotionModule::getInstance().toNative(raw);
}

Vector4f32 toNative(const Vector4f32& raw) {
	return MotionModule::getInstance().toNative(raw);
}

Vector5f32 toNative(const Vector5f32& raw) {
	return MotionModule::getInstance().toNative(raw);
}

Vector6f32 toNative(const Vector6f32& raw) {
	return MotionModule::getInstance().toNative(raw);
}