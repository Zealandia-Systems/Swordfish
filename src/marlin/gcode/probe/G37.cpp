/*
 * G37.cpp
 *
 * Created: 10/05/2021 12:38:30 pm
 *  Author: smohekey
 *
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

#include <swordfish/debug.h>

#include <swordfish/Controller.h>

#if HAS_TOOL_PROBE
#	include "../gcode.h"

#	include "../../module/endstops.h"
#	include "../../module/motion.h"
#	include "../../module/planner.h"

using namespace swordfish;
using namespace swordfish::motion;
using namespace swordfish::status;

#	define TOOL_PROBE_X (X_MIN_POS + 35)
#	define TOOL_PROBE_Y (Y_MAX_POS - 10)

inline bool do_single_probe(EndstopEnum endstop /*, const uint8_t move_value*/) {
	auto& motionModule = MotionModule::getInstance();

	auto limitsEnabled = motionModule.areLimitsEnabled();
	motionModule.setLimitsEnabled(false);

	endstops.enable(true);

	prepare_line_to_destination(MachineState::Probing);
	planner.synchronize();

	bool triggered = endstops.trigger_state() & (1 << endstop);

	endstops.hit_on_purpose();

	set_current_from_steppers_for_axis(ALL_AXES);
	sync_plan_position();

	endstops.enable(false);

	motionModule.setLimitsEnabled(limitsEnabled);

	return triggered;
}

void backoff(xyz_float_t& retract_mm) {
	REMEMBER(fr, feedrate_mm_s, MMM_TO_MMS(G0_FEEDRATE));

	// Move away by the retract distance
	destination = current_position + retract_mm;

	prepare_line_to_destination(MachineState::Probing);
	planner.synchronize();
}

bool run_probe(AxisEnum axis, EndstopEnum endstop, float distance, float retract) {
	bool triggered = false;

	debug()("probe distance: ", distance);

	xyz_pos_t delta = { 0, 0, 0 };

	delta[axis] = distance;

	destination = current_position + delta;

#	if MULTIPLE_PROBING > 1
	xyz_float_t retract_mm = { 0, 0, 0 };
	retract_mm[axis] = /* home_bump_mm(axis) */ retract * (distance > 0 ? -1 : 1);
#	endif

	planner.synchronize(); // wait until the machine is idle

// Move flag value
#	if ENABLED(G38_PROBE_AWAY)
	[[maybe_unused]] const uint8_t move_value = parser.subcode;
#	else
	constexpr uint8_t move_value = 1;
#	endif

	// Move until destination reached or target hit

	if (do_single_probe(endstop /*, move_value*/)) {
		triggered = true;

#	if MULTIPLE_PROBING > 1
		auto i = MULTIPLE_PROBING - 1;

		while (i--) {
			backoff(retract_mm);

			REMEMBER(fr, feedrate_mm_s, feedrate_mm_s * 0.25);

			// Bump the target more slowly
			destination = current_position - retract_mm * 2;

			do_single_probe(endstop /*, move_value*/);
		}
#	endif
	}

	endstops.not_homing();

	return triggered;
}

/**
 * G37 Probe Tool Offset
 */
void GcodeSuite::G37() {
	if (homing_needed_error(_BV(X_AXIS) | _BV(Y_AXIS) | _BV(Z_AXIS))) {
		return;
	}

	remember_feedrate_scaling_off();

	feedrate_mm_s = parser.seenval('F') ? parser.value_feedrate() : (homing_feedrate(Z_AXIS) * 0.3);
	float retract = parser.seenval('R') ? parser.value_float() : 5;

	const auto delta = -abs(home_dir(Z_AXIS) > 0 ? Z_MAX_POS - Z_MIN_POS : Z_MIN_POS - Z_MAX_POS);

	endstops.enable_tool_probe(true);

	run_probe(Z_AXIS, TOOL_PROBE, delta, retract);

	endstops.enable_tool_probe(false);

	restore_feedrate_and_scaling();
}

#endif