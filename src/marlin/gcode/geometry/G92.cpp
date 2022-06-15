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

#include <Eigen/Core>

#include "../gcode.h"
#include "../../module/motion.h"
#include "../../module/stepper.h"

#if ENABLED(I2C_POSITION_ENCODERS)
#	include "../../feature/encoder_i2c.h"
#endif

#include <swordfish/Controller.h>
#include <swordfish/core/Console.h>

using namespace Eigen;

using namespace swordfish;
using namespace swordfish::core;
using namespace swordfish::motion;

/**
 * G92: Set current position to given X Y Z E
 */
void GcodeSuite::G92(bool report) {
	auto& motionManager = MotionModule::getInstance();

#if ENABLED(USE_GCODE_SUBCODES)
	const uint8_t subcode_G92 = parser.subcode;
#else
	constexpr uint8_t subcode_G92 = 0;
#endif

	switch (subcode_G92) {
		default:
			break;
#if ENABLED(CNC_COORDINATE_SYSTEMS)
		case 1: {
			// Zero the G92 values and restore current position
			motionManager.setWorkOffset({ 0, 0, 0 });
		}
			return;
#endif

		case 0: {
			Vector3f workOffset { 0, 0, 0 };

			LOOP_XYZA(i) {
				if (parser.seenval(axis_codes[i])) {
					const float l = parser.value_axis_units((AxisEnum) i),
											v = toNative(l, (AxisEnum) i),
											d = v - current_position[i];
					if (!NEAR_ZERO(d)) {
						workOffset(i) += d; // Other axes simply offset the coordinate space
					}
				}
			}

			motionManager.setWorkOffset(workOffset);

		} break;
	}

	sync_plan_position();

#if DISABLED(DIRECT_STEPPING)
	if (report) {
		report_current_position();
	}
#endif
}
