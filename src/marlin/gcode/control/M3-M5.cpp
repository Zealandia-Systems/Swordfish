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
#include "../../module/planner.h"

#include <swordfish/Controller.h>

using namespace swordfish::core;
using namespace swordfish::tools;
using namespace swordfish::status;

/**
 * Laser:
 *  M3 - Laser ON/Power (Ramped power)
 *  M4 - Laser ON/Power (Continuous power)
 *
 * Spindle:
 *  M3 - Spindle ON (Clockwise)
 *  M4 - Spindle ON (Counter-clockwise)
 *
 * Parameters:
 *  S<power> - Set power. S0 will turn the spindle/laser off, except in relative mode.
 *  O<ocr>   - Set power and OCR (oscillator count register)
 *
 *  If no PWM pin is defined then M3/M4 just turns it on.
 *
 *  At least 12.8KHz (50Hz * 256) is needed for Spindle PWM.
 *  Hardware PWM is required on AVR. ISRs are too slow.
 *
 * NOTE: WGM for timers 3, 4, and 5 must be either Mode 1 or Mode 5.
 *       No other settings give a PWM signal that goes from 0 to 5 volts.
 *
 *       The system automatically sets WGM to Mode 1, so no special
 *       initialization is needed.
 *
 *       WGM bits for timer 2 are automatically set by the system to
 *       Mode 1. This produces an acceptable 0 to 5 volt signal.
 *       No special initialization is needed.
 *
 * NOTE: A minimum PWM frequency of 50 Hz is needed. All prescaler
 *       factors for timers 2, 3, 4, and 5 are acceptable.
 *
 *  SPINDLE_LASER_ENA_PIN needs an external pullup or it may power on
 *  the spindle/laser during power-up or when connecting to the host
 *  (usually goes through a reset which sets all I/O pins to tri-state)
 *
 *  PWM duty cycle goes from 0 (off) to 255 (always on).
 */
void GcodeSuite::M3_M4(const bool is_M4) {
	TemporaryState state(MachineState::SpindleRamp);

	auto& toolsModule = ToolsModule::getInstance();
	auto& driver = toolsModule.getCurrentDriver();

	planner.synchronize();

	if (driver.hasDirection()) {
		driver.setTargetDirection(is_M4 ? Direction::Reverse : Direction::Forward);
	}

	if (parser.seenval('S')) {
		driver.setTargetPower(round(parser.value_float()));
	}

	driver.setEnabled(true);
	driver.apply();
}

/**
 * M5 - Cutter OFF (when moves are complete)
 */
void GcodeSuite::M5() {
	TemporaryState state(MachineState::SpindleRamp);

	auto& toolsModule = ToolsModule::getInstance();
	auto& driver = toolsModule.getCurrentDriver();

	planner.synchronize();

	driver.setTargetPower(0);
	driver.setEnabled(false);
	driver.apply();
}
