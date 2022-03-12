/*
 * M6.cpp
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

#include <swordfish/Controller.h>

#include "../../inc/MarlinConfig.h"

#include "../gcode.h"

#include "../../module/motion.h"
#include "../../MarlinCore.h"     // for wait_for_user_response()
#include "../../feature/host_actions.h"
#include "../../feature/spindle_laser.h"

using namespace swordfish::tools;

void GcodeSuite::M6() {
	if(homing_needed_error(_BV(X_AXIS)|_BV(Y_AXIS)|_BV(Z_AXIS))) {
		return;
	}
	
	ToolsModule::getInstance().change();
}