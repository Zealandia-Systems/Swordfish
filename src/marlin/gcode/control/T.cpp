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

#if EITHER(HAS_MULTI_EXTRUDER, DEBUG_LEVELING_FEATURE)
#	include "../../module/motion.h"
#endif

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../../core/debug_out.h"

#include <swordfish/Controller.h>

using namespace swordfish::tools;

/**
 * T0-T<n>: Switch tool, usually switching extruders
 *
 *   F[units/min] Set the movement feedrate
 *   S1           Don't move the tool in XY after change
 */
void GcodeSuite::T(const int16_t tool_index) {

	DEBUG_SECTION(log_T, "T", DEBUGGING(LEVELING));
	if (DEBUGGING(LEVELING))
		DEBUG_ECHOLNPAIR("...(", tool_index, ")");

	// Count this command as movement / activity
	reset_stepper_timeout();

	ToolsModule::getInstance().setNextToolIndex(tool_index - 1);

	if (parser.chain()) { // Command to chain?
		process_parsed_command(); // ...process the chained command
	}
}
