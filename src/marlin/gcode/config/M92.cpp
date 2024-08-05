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
#include "../../module/planner.h"

void report_M92(const bool echo=true) {
  if (echo) SERIAL_ECHO_START(); else SERIAL_CHAR(' ');
  SERIAL_ECHOPAIR_P(PSTR(" M92 X"), parser.linear_value_to_mm(planner.settings.axis_steps_per_unit[Axis::X()]));
	SERIAL_ECHOPAIR_P(SP_Y_STR, parser.linear_value_to_mm(planner.settings.axis_steps_per_unit[Axis::Y()]));
	SERIAL_ECHOPAIR_P(SP_Z_STR, parser.linear_value_to_mm(planner.settings.axis_steps_per_unit[Axis::Z()]));
	SERIAL_ECHOPAIR_P(SP_A_STR, parser.radial_value_to_degrees(planner.settings.axis_steps_per_unit[Axis::A()]));

	SERIAL_EOL();
}

/**
 * M92: Set axis steps-per-unit for one or more axes, X, Y, Z, and E.
 *      (Follows the same syntax as G92)
 *
 *      With multiple extruders use T to specify which one.
 *
 *      If no argument is given print the current values.
 *
 *    With MAGIC_NUMBERS_GCODE:
 *      Use 'H' and/or 'L' to get ideal layer-height information.
 *      'H' specifies micro-steps to use. We guess if it's not supplied.
 *      'L' specifies a desired layer height. Nearest good heights are shown.
 */
void GcodeSuite::M92() {
  // No arguments? Show M92 report.
  if (!parser.seen("XYZABC")) {
		return report_M92(true);
	}

  for (auto axis : all_axes) {
    if (parser.seenval(axis.to_char())) {
      planner.settings.axis_steps_per_unit[axis] = parser.value_per_axis_units(axis);
    }
  }

  planner.refresh_positioning();
}
