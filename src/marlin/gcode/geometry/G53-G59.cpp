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

#include "../gcode.h"
#include "../../module/motion.h"

#if ENABLED(CNC_COORDINATE_SYSTEMS)

#	include "../../module/stepper.h"

#	include <swordfish/Controller.h>

using namespace swordfish;
using namespace swordfish::motion;

//#define DEBUG_M53

/**
 * Select a coordinate system and update the workspace offset.
 * System index -1 is used to specify machine-native.
 */
/*bool GcodeSuite::select_coordinate_system(const int8_t _new) {
  auto& motionManager = controller.motionManager();
  auto& activeSystem = motionManager.activeCoordinateSystem();

  if (activeSystem.index() == _new) {
    return false;
  }

  auto* newSystem = motionManager.workCoordinateSystems().get(_new);

  if(!newSystem) {
    return false;
  }

  motionManager.activeCoordinateSystem(*newSystem);

  Vector3f offset = newSystem->getOffset();

  LOOP_XYZ(i) {
    if (position_shift[i] != offset.values[i]) {
      position_shift[i] = offset.values[i];
      update_workspace_offset((AxisEnum)i);
    }
  }

  return true;
}*/

/**
 * G53: Apply native workspace to the current move
 *
 * In CNC G-code G53 is a modifier.
 * It precedes a movement command (or other modifiers) on the same line.
 * This is the first command to use parser.chain() to make this possible.
 *
 * Marlin also uses G53 on a line by itself to go back to native space.
 */
void GcodeSuite::G53() {
	auto& motionManager = MotionModule::getInstance();
	auto& activeSystem = motionManager.getActiveCoordinateSystem();
	auto& mcs = motionManager.getMachineCoordiateSystem();

	motionManager.setActiveCoordinateSystem(mcs);

#	ifdef DEBUG_M53
	SERIAL_ECHOLNPGM("Go to native space");
	report_current_position();
#	endif

	if (parser.chain()) { // Command to chain?
		process_parsed_command(); // ...process the chained command

		motionManager.setActiveCoordinateSystem(activeSystem);

#	ifdef DEBUG_M53
		SERIAL_ECHOLNPAIR("Go back to workspace ", old_system);
		report_current_position();
#	endif
	}
}

/**
 * G54-G59.3: Select a new workspace
 *
 * A workspace is an XYZ offset to the machine native space.
 * All workspaces default to 0,0,0 at start, or with EEPROM
 * support they may be restored from a previous session.
 *
 * G92 is used to set the current workspace's offset.
 */

void GcodeSuite::G54() {
	auto& motionManager = MotionModule::getInstance();
	auto& activeSystem = motionManager.getActiveCoordinateSystem();

	const int8_t index = (parser.codenum - 54) * 10 + parser.subcode;

#	ifdef DEBUG_M53
	SERIAL_ECHOLNPAIR("wcs: ", index);
#	endif

	auto* wcs = motionManager.getWorkCoordinateSystems().get(index);

	if (wcs) {
		motionManager.setActiveCoordinateSystem(*wcs);
	}

	if (parser.chain()) { // Command to chain?
		process_parsed_command(); // ...process the chained command

		motionManager.setActiveCoordinateSystem(activeSystem);

#	ifdef DEBUG_M53
		SERIAL_ECHOLNPAIR("Go back to workspace ", old_system);
		report_current_position();
#	endif
	}
}

void GcodeSuite::G55() {
	G54();
}
void GcodeSuite::G56() {
	G54();
}
void GcodeSuite::G57() {
	G54();
}
void GcodeSuite::G58() {
	G54();
}
void GcodeSuite::G59() {
	G54();
}

#endif // CNC_COORDINATE_SYSTEMS
