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

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../../core/debug_out.h"

#include <swordfish/Controller.h>

using namespace swordfish;
using namespace swordfish::tools;
using namespace swordfish::motion;
using namespace swordfish::status;

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
void GcodeSuite::G28(const int8_t subcode) {
	auto& toolsModule = ToolsModule::getInstance();
	auto& motionModule = MotionModule::getInstance();

	auto referencing = subcode == 2;
	auto homing = subcode == 0;

	if (homing && axes_should_home()) {
		throw CommandException("One or more axes need to be recovered.");
	}

	planner.synchronize(); // Wait for planner moves to finish!

	TemporaryState temp(MachineState::Homing);

	// Reset to the XY plane
	workspace_plane = PLANE_XY;

	// Count this command as movement / activity
	reset_stepper_timeout();

	endstops.enable(true); // Enable endstops for next homing move

	toolsModule.ensureClearOfCaddy();

	remember_feedrate_scaling_off();


	const bool doX = parser.seen('X');
	const bool doY = parser.seen('Y');
	const bool forceZ = parser.seen('Z');
	const bool doA = parser.seen('A');

	const bool doZ = doX || doY || doA || forceZ;

	if (doZ) {
		if (referencing && (!axis_is_trusted(Axis::Z()) || forceZ)) {
			reference_linear_axis(Axis::Z());
		} else {
			do_blocking_move_to_z(MachineState::Homing, home_dir(Axis::Z()) > 0 ? Z_MAX_POS : Z_MIN_POS);
		}
	}

	if (doA && motionModule.shouldHomeFourth()) {
		if (referencing) {
			reference_rotary_axis(Axis::A());
		} else {
			debug()("home A");

			do_blocking_move_to_a(MachineState::Homing, 0);
		}
	}

	if (doY) {
		if (referencing) {
			reference_linear_axis(Axis::Y());
		} else {
			do_blocking_move_to_y(MachineState::Homing, home_dir(Axis::Y()) > 0 ? Y_MAX_POS : Y_MIN_POS);
		}
	}

	if (doX) {
		if (referencing) {
			reference_linear_axis(Axis::X());
		} else {
			do_blocking_move_to_x(MachineState::Homing, home_dir(Axis::X()) > 0 ? X_MAX_POS : X_MIN_POS);
		}
	}

	sync_plan_position();

	endstops.not_homing();

	restore_feedrate_and_scaling();

	if (toolsModule.isAutomatic()) {
		// move to X min
		do_blocking_move_to_x(MachineState::Homing, motionModule.getLimits().getMin().x());
	}

	report_current_position();
}
