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

#include "../../inc/MarlinConfig.h"

#include "../gcode.h"

#include "../../module/stepper.h"
#include "../../module/endstops.h"

#include "../../module/probe.h"

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../../core/debug_out.h"

#include <swordfish/Controller.h>

using namespace swordfish;
using namespace swordfish::tools;
using namespace swordfish::motion;
using namespace swordfish::status;

#if ENABLED(QUICK_HOME)

static void quick_home_xy() {

	// Pretend the current position is 0,0
	current_position.set(0.0, 0.0);
	sync_plan_position();

	const int x_axis_home_dir = x_home_dir(active_extruder);

	const float mlx = max_length(X_AXIS),
							mly = max_length(Y_AXIS),
							mlratio = mlx > mly ? mly / mlx : mlx / mly,
							fr_mm_s = _MIN(homing_feedrate(X_AXIS), homing_feedrate(Y_AXIS)) * SQRT(sq(mlratio) + 1.0);

#	if ENABLED(SENSORLESS_HOMING)
	sensorless_t stealth_states {
		tmc_enable_stallguard(stepperX), tmc_enable_stallguard(stepperY), false, false
#		if AXIS_HAS_STALLGUARD(X2)
																																								 || tmc_enable_stallguard(stepperX2)
#		endif
																																								 ,
				false
#		if AXIS_HAS_STALLGUARD(Y2)
						|| tmc_enable_stallguard(stepperY2)
#		endif
	};
#	endif

	do_blocking_move_to_xy(1.5 * mlx * x_axis_home_dir, 1.5 * mly * home_dir(Y_AXIS), fr_mm_s);

	endstops.validate_homing_move();

	current_position.set(0.0, 0.0);

#	if ENABLED(SENSORLESS_HOMING)
	tmc_disable_stallguard(stepperX, stealth_states.x);
	tmc_disable_stallguard(stepperY, stealth_states.y);
#		if AXIS_HAS_STALLGUARD(X2)
	tmc_disable_stallguard(stepperX2, stealth_states.x2);
#		endif
#		if AXIS_HAS_STALLGUARD(Y2)
	tmc_disable_stallguard(stepperY2, stealth_states.y2);
#		endif
#	endif
}

#endif // QUICK_HOME

#if ENABLED(Z_SAFE_HOMING)

inline void home_z_safely() {
	DEBUG_SECTION(log_G28, "home_z_safely", DEBUGGING(LEVELING));

	// Disallow Z homing if X or Y homing is needed
	if (homing_needed_error(_BV(X_AXIS) | _BV(Y_AXIS))) {
		return;
	}

	sync_plan_position();

	/**
	 * Move the Z probe (or just the nozzle) to the safe homing point
	 * (Z is already at the right height)
	 */
	constexpr xy_float_t safe_homing_xy = { Z_SAFE_HOMING_X_POINT, Z_SAFE_HOMING_Y_POINT };
#	if HAS_HOME_OFFSET
	xy_float_t okay_homing_xy = safe_homing_xy;
	okay_homing_xy -= home_offset;
#	else
	constexpr xy_float_t okay_homing_xy = safe_homing_xy;
#	endif

	destination.set(okay_homing_xy, current_position.z);

	TERN_(HOMING_Z_WITH_PROBE, destination -= probe.offset_xy);

	if (position_is_reachable(destination)) {

		if (DEBUGGING(LEVELING))
			DEBUG_POS("home_z_safely", destination);

		// Free the active extruder for movement
		TERN_(DUAL_X_CARRIAGE, idex_set_parked(false));

		TERN_(SENSORLESS_HOMING, safe_delay(500)); // Short delay needed to settle

		do_blocking_move_to_xy(destination);
		homeaxis(Z_AXIS);
	} else {
		LCD_MESSAGEPGM(MSG_ZPROBE_OUT);
		SERIAL_ECHO_MSG(STR_ZPROBE_OUT_SER);
	}
}

#endif // Z_SAFE_HOMING

#if ENABLED(IMPROVE_HOMING_RELIABILITY)

slow_homing_t begin_slow_homing() {
	slow_homing_t slow_homing { 0 };
	slow_homing.acceleration.set(planner.settings.max_acceleration_mm_per_s2[X_AXIS],
	                             planner.settings.max_acceleration_mm_per_s2[Y_AXIS]);
	planner.settings.max_acceleration_mm_per_s2[X_AXIS] = 100;
	planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = 100;
#	if HAS_CLASSIC_JERK
	slow_homing.jerk_xy = planner.max_jerk;
	planner.max_jerk.set(0, 0);
#	endif
	planner.reset_acceleration_rates();
	return slow_homing;
}

void end_slow_homing(const slow_homing_t& slow_homing) {
	planner.settings.max_acceleration_mm_per_s2[X_AXIS] = slow_homing.acceleration.x;
	planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = slow_homing.acceleration.y;
	TERN_(HAS_CLASSIC_JERK, planner.max_jerk = slow_homing.jerk_xy);
	planner.reset_acceleration_rates();
}

#endif // IMPROVE_HOMING_RELIABILITY

/**
 * G28: Home all axes according to settings
 *
 * Parameters
 *
 *  None  Home to all axes with no parameters.
 *        With QUICK_HOME enabled XY will home together, then Z.
 *
 *  O   Home only if position is unknown
 *
 *  Rn  Raise by n mm/inches before homing
 *
 * Cartesian/SCARA parameters
 *
 *  X   Home to the X endstop
 *  Y   Home to the Y endstop
 *  Z   Home to the Z endstop
 */
void GcodeSuite::G28() {
	auto& toolsModule = ToolsModule::getInstance();
	auto& motionModule = MotionModule::getInstance();

	DEBUG_SECTION(log_G28, "G28", DEBUGGING(LEVELING));
	if (DEBUGGING(LEVELING))
		log_machine_info();

	TERN_(LASER_MOVE_G28_OFF, cutter.set_inline_enabled(false)); // turn off laser

	bool homing = axes_should_home() || parser.seen('A');

	// Home (O)nly if position is unknown
	if (!homing && parser.boolval('O')) {
		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPGM("> homing not needed, skip");
		return;
	}

	planner.synchronize(); // Wait for planner moves to finish!

	TemporaryState temp(MachineState::Homing);

// Disable the leveling matrix before homing
#if HAS_LEVELING
	const bool leveling_restore_state = parser.boolval('L', TERN(RESTORE_LEVELING_AFTER_G28, planner.leveling_active, ENABLED(ENABLE_LEVELING_AFTER_G28)));
	IF_ENABLED(PROBE_MANUALLY, g29_in_progress = false); // Cancel the active G29 session
	set_bed_leveling_enabled(false);
#endif

	// Reset to the XY plane
	TERN_(CNC_WORKSPACE_PLANES, workspace_plane = PLANE_XY);

	// Count this command as movement / activity
	reset_stepper_timeout();

	endstops.enable(true); // Enable endstops for next homing move

	toolsModule.ensureClearOfCaddy();

	remember_feedrate_scaling_off();

	const bool homeZ = parser.seen('Z'),
						 needX = homeZ && TERN0(Z_SAFE_HOMING, axes_should_home(_BV(X_AXIS))),
						 needY = homeZ && TERN0(Z_SAFE_HOMING, axes_should_home(_BV(Y_AXIS))),
						 homeX = needX || parser.seen('X'), homeY = needY || parser.seen('Y'),
						 home_all = homeX == homeY && homeX == homeZ, // All or None
			doX = home_all || homeX, doY = home_all || homeY, doZ = home_all || homeZ;

#if ENABLED(HOME_Z_FIRST)

	if (doZ) {
		if (homing) {
			homeaxis(Z_AXIS);
		} else {
			do_blocking_move_to_z(MachineState::Homing, home_dir(Z_AXIS) > 0 ? Z_MAX_POS : Z_MIN_POS);
		}
	}

#endif

	const float z_homing_height = TERN1(UNKNOWN_Z_NO_RAISE, axis_is_trusted(Z_AXIS))
	                                  ? (parser.seenval('R') ? parser.value_linear_units() : Z_HOMING_HEIGHT)
	                                  : 0;

	if (z_homing_height && (doX || doY || TERN0(Z_SAFE_HOMING, doZ))) {
		// Raise Z before homing any other axes and z is not already high enough (never lower z)
		if (DEBUGGING(LEVELING))
			DEBUG_ECHOLNPAIR("Raise Z (before homing) by ", z_homing_height);
		do_z_clearance(MachineState::Homing, z_homing_height, axis_is_trusted(Z_AXIS), DISABLED(UNKNOWN_Z_NO_RAISE));
	}

#if ENABLED(QUICK_HOME)

	if (doX && doY) {
		if (homing) {
			quick_home_xy();
		} else {
			do_blocking_move_to_xy(
					home_dir(X_AXIS) > 0 ? X_MAX_POS : X_MIN_POS,
					home_dir(Y_AXIS) > 0 ? Y_MAX_POS : Y_MIN_POS);
		}
	}

#endif

	// Home Y (before X)
	if (ENABLED(HOME_Y_BEFORE_X) && (doY || TERN0(CODEPENDENT_XY_HOMING, doX)))
		homeaxis(Y_AXIS);

	// Home X
	if (doX || (doY && ENABLED(CODEPENDENT_XY_HOMING) && DISABLED(HOME_Y_BEFORE_X))) {

#if ENABLED(DUAL_X_CARRIAGE)

		// Always home the 2nd (right) extruder first
		active_extruder = 1;
		homeaxis(X_AXIS);

		// Remember this extruder's position for later tool change
		inactive_extruder_x = current_position.x;

		// Home the 1st (left) extruder
		active_extruder = 0;
		homeaxis(X_AXIS);

		// Consider the active extruder to be in its "parked" position
		idex_set_parked();

#else

		if (homing) {
			homeaxis(X_AXIS);
		} else {
			do_blocking_move_to_x(MachineState::Homing, home_dir(X_AXIS) > 0 ? X_MAX_POS : X_MIN_POS);
		}

#endif
	}

	// Home Y (after X)
	if (DISABLED(HOME_Y_BEFORE_X) && doY) {
		if (homing) {
			homeaxis(Y_AXIS);
		} else {
			do_blocking_move_to_y(MachineState::Homing, home_dir(Y_AXIS) > 0 ? Y_MAX_POS : Y_MIN_POS);
		}
	}

	TERN_(IMPROVE_HOMING_RELIABILITY, end_slow_homing(slow_homing));

// Home Z last if homing towards the bed
#if DISABLED(HOME_Z_FIRST)
	if (doZ) {
#	if EITHER(Z_MULTI_ENDSTOPS, Z_STEPPER_AUTO_ALIGN)
		stepper.set_all_z_lock(false);
		stepper.set_separate_multi_axis(false);
#	endif

		TERN_(BLTOUCH, bltouch.init());
		TERN(Z_SAFE_HOMING, home_z_safely(), homeaxis(Z_AXIS));
		probe.move_z_after_homing();
	}
#endif

	sync_plan_position();

	endstops.not_homing();

	// Clear endstop state for polled stallGuard endstops
	TERN_(SPI_ENDSTOPS, endstops.clear_endstop_state());

#if BOTH(DELTA, DELTA_HOME_TO_SAFE_ZONE)
	// move to a height where we can use the full xy-area
	do_blocking_move_to_z(delta_clip_start_height);
#endif

	TERN_(HAS_LEVELING, set_bed_leveling_enabled(leveling_restore_state));

	restore_feedrate_and_scaling();

	if (toolsModule.isAutomatic()) {
		// move to X min
		do_blocking_move_to_x(MachineState::Homing, motionModule.getLimits().getMin().x());
	}

	report_current_position();
}
