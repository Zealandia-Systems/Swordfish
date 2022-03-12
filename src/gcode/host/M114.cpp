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
#include "../../module/motion.h"
#include "../../module/stepper.h"

/**
 * M114: Report the current position to host.
 *       Since steppers are moving, the count positions are
 *       projected by using planner calculations.
 *   D - Report more detail. This syncs the planner. (Requires M114_DETAIL)
 *   E - Report E stepper position (Requires M114_DETAIL)
 *   R - Report the realtime position instead of projected.
 */
void GcodeSuite::M114() {

  #if ENABLED(M114_DETAIL)
    if (parser.seen('D')) {
      #if DISABLED(M114_LEGACY)
        planner.synchronize();
      #endif
      report_current_position();
      report_current_position_detail();
      return;
    }
    if (parser.seen('E')) {
      SERIAL_ECHOLNPAIR("Count E:", stepper.position(E_AXIS));
      return;
    }
  #endif

  #if ENABLED(M114_REALTIME)
    if (parser.seen('R')) { report_real_position(); return; }
  #endif

  TERN_(M114_LEGACY, planner.synchronize());
  report_current_position_projected();
}
