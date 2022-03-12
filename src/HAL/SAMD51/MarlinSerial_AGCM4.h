/**
 * Marlin 3D Printer Firmware
 *
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 * SAMD51 HAL developed by Giuliano Zaro (AKA GMagician)
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
#pragma once

#if ENABLED(EMERGENCY_PARSER)
	#include <swordfish/debug.h>
	#include "../../feature/e_parser.h"
#endif

class EmergencySerial : public Serial_ {
	public:
		EmergencySerial(USBDeviceClass &usb) : Serial_(usb), emergency_state(EmergencyParser::State::EP_RESET) { }
		
		#if ENABLED(EMERGENCY_PARSER)
		int read(void) override {
			int c = Serial_::read();
			
			emergency_parser.update(emergency_state, c);
			
			return c; // do not discard character
		}

		EmergencyParser::State emergency_state;
		static inline bool emergency_parser_enabled() { return true; }
		#endif
	};

#if ENABLED(EMERGENCY_PARSER)
extern EmergencySerial ESerial;
#endif

extern Uart Serial2;
extern Uart Serial3;
extern Uart Serial4;
