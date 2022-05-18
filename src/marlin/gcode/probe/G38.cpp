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

#if ENABLED(G38_PROBE_TARGET)

#	include "../gcode.h"

#	include "../../module/motion.h"
#	include "../../module/endstops.h"

extern bool run_probe(AxisEnum axis, EndstopEnum endstop, float distance, float retract);

/**
 * G38 Probe Target
 *
 *  G38.2 - Probe toward workpiece, stop on contact, signal error if failure
 *  G38.3 - Probe toward workpiece, stop on contact
 *
 * With G38_PROBE_AWAY:
 *
 *  G38.4 - Probe away from workpiece, stop on contact break, signal error if failure
 *  G38.5 - Probe away from workpiece, stop on contact break
 */
void GcodeSuite::G38(const int8_t subcode) {
	KEEPALIVE_STATE(PROBING);

	remember_feedrate_scaling_off();

	[[maybe_unused]] const bool error_on_fail =
#	if ENABLED(G38_PROBE_AWAY)
			!TEST(subcode, 0)
#	else
			(subcode == 2)
#	endif
			;

	// If any axis has enough movement, do the move
	LOOP_XYZ(i) {
		if (parser.seenval(XYZ_CHAR(i))) {
			const float v = parser.value_axis_units((AxisEnum) i);
			const float dest = axis_is_relative((AxisEnum) i) ? current_position[i] + v : toNative(v, (AxisEnum) i);

			if (ABS(dest - current_position[i]) >= G38_MINIMUM_MOVE) {
				// destination[i] = dest;

				endstops.enable_work_probe((AxisEnum) i, true);

				feedrate_mm_s = parser.seenval('F') ? parser.value_feedrate() : (homing_feedrate((AxisEnum) i) * 0.5);
				float retract = parser.seenval('R') ? parser.value_float() : 5;

				if (!run_probe((AxisEnum) i, WORK_PROBE, v, retract))
					SERIAL_ERROR_MSG("Failed to reach target");

				endstops.enable_work_probe((AxisEnum) i, false);

				break;
			}
		}
	}

	restore_feedrate_and_scaling();
}

#endif // G38_PROBE_TARGET
