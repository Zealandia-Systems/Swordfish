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

#include <swordfish/Controller.h>

using namespace swordfish;
using namespace swordfish::motion;
using namespace swordfish::status;

#include "../gcode.h"
#include "../../module/motion.h"
#include "../../module/planner.h"

#include "../../MarlinCore.h"
#include <swordfish/Controller.h>

#if ENABLED(VARIABLE_G0_FEEDRATE)
  FeedRate rapidrate_mm_s = FeedRate::UnitsPerSecond(MMM_TO_MMS(G0_FEEDRATE));
#endif

#if HAS_FAST_MOVES
	int16_t rapidrate_percentage = 100;
#endif

/**
 * G0, G1: Coordinated movement of X Y Z E axes
 */
void GcodeSuite::G0_G1(const bool rapid_move/* = false*/) {
	if (IsRunning()
    #if ENABLED(NO_MOTION_BEFORE_HOMING)
      && !homing_needed_error(
          (parser.seen('X') ? _BV(X_AXIS) : 0)
        | (parser.seen('Y') ? _BV(Y_AXIS) : 0)
        | (parser.seen('Z') ? _BV(Z_AXIS) : 0) )
    #endif
  ) {

    FeedRate old_feedrate = feedrate_mm_s;
		MachineState machine_state = MachineState::FeedMove;

    get_destination_from_command(rapid_move);                 // Get X Y Z E F (and set cutter power)

    if (rapid_move) {
			machine_state = MachineState::RapidMove;

      rapidrate_mm_s = feedrate_mm_s;       // Save feedrate for the next G0

			feedrate_mm_s = rapidrate_mm_s * 0.01f * rapidrate_percentage;
    }

		debug()("accel_mm_s2: ", rapid_move ? planner.settings.travel_acceleration : planner.settings.acceleration);
		debug()("feed_rate: type=", (i32) feedrate_mm_s.type(), ", value=", feedrate_mm_s.value());

    prepare_line_to_destination(machine_state, rapid_move ? planner.settings.travel_acceleration : planner.settings.acceleration);

		// Restore the motion mode feedrate
    if (rapid_move) {
			feedrate_mm_s = old_feedrate;
		}
  }
}
