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

/**
 * feature/spindle_laser.cpp
 */

#include "../inc/MarlinConfig.h"

#include "../core/serial.h"
#include "../gcode/gcode.h"

#if HAS_CUTTER

#include "spindle_laser.h"

#include "../module/estop.h"
#include "../libs/modbus.h"
	
SpindleLaser cutter;

bool SpindleLaser::_enabled;
uint16_t SpindleLaser::_override = 100;
uint16_t SpindleLaser::_target_frequency = 0;
SpindleDirection SpindleLaser::_target_direction;
uint16_t SpindleLaser::_target_rpm;

uint16_t SpindleLaser::current_frequency;
SpindleDirection SpindleLaser::current_direction;
uint16_t SpindleLaser::current_rpm;
uint16_t SpindleLaser::minimum_frequency;
uint16_t SpindleLaser::maximum_frequency;

#define SPINDLE_DEBUG(x) do { if(DEBUGGING(SPINDLE)) { x; } } while(0)

enum {
	STATUS_RUNNING = (1<<0),
	STATUS_JOGGING = (1<<1),
	STATUS_REVERSE = (1<<2),
	STATUS_IN_RUNNING = (1<<3),
	STATUS_IN_JOGGING = (1<<4),
	STATUS_IN_REVERSE = (1<<5),
	STATUS_IN_BRAKING = (1<<6),
	STATUS_TRACKING = (1<<7)
};
//
// Init the cutter to a safe OFF state
//
void SpindleLaser::init() {
	modbus::init();
		
	modbus::write_holding_register(SPINDLE_MODBUS_ADDRESS, 0x0200, 0x00);
}


void SpindleLaser::wait_for_spindle(uint16_t target_freq) {
	while(!estop_engaged()) {
		refresh();
		
		GcodeSuite::host_keepalive();
		
		SPINDLE_DEBUG(SERIAL_ECHOLNPAIR("actual: ", current_frequency, ", target: ", target_freq));
		
		if(current_frequency == target_freq) {
			break;
		}
		
		safe_delay(10);
	}
}

uint16_t SpindleLaser::read_frequency() {
	uint16_t frequency;
	
	SPINDLE_DEBUG(SERIAL_ECHOLN("Retrieving current frequency from vfd"));
	
	if(1 != modbus::read_input_registers(SPINDLE_MODBUS_ADDRESS, 0x0000, 1, &frequency)) {
		SPINDLE_DEBUG(SERIAL_ERROR_MSG("Couldn't read current frequency from spindle."));
		
		return 0;
	}
	
	return frequency;
}

uint8_t SpindleLaser::read_status() {
	uint16_t status;
	
	SPINDLE_DEBUG(SERIAL_ECHOLN("Retrieving current direction from vfd"));
	
	if(1 != modbus::read_holding_registers(SPINDLE_MODBUS_ADDRESS, 0x0210, 1, &status)) {
		SPINDLE_DEBUG(SERIAL_ERROR_MSG("Couldn't read current rotating direction from spindle."));
			
		return 0;
	}
	
	return (uint8_t)(status & 0xFF);
}

uint16_t SpindleLaser::calculate_target_freq() {
	// calculate target frequency
	float overidden_rpm = (_target_rpm * 0.01f) * _override;
	
	uint16_t target_frequency = (3000.0f/SPEED_POWER_MAX) * overidden_rpm;

	SPINDLE_DEBUG(SERIAL_ECHOLN("Retrieving minimum frequency parameter from vfd"));

	if(1 != modbus::read_holding_registers(SPINDLE_MODBUS_ADDRESS, 11, 1, &minimum_frequency) || minimum_frequency < 1200 || minimum_frequency > 3000) {
		minimum_frequency = 1200;
	}
	
	SPINDLE_DEBUG(SERIAL_ECHOLN("Retrieving maximum frequency from vfd"));

	if(1 != modbus::read_holding_registers(SPINDLE_MODBUS_ADDRESS, 5, 1, &maximum_frequency) || maximum_frequency < 1200 || maximum_frequency > 3000) {
		maximum_frequency = 3000;
	}
		
	if (DEBUGGING(SPINDLE)) {
		SERIAL_ECHOLNPAIR("minimum_freq: ", minimum_frequency);
		SERIAL_ECHOLNPAIR("maximum_freq: ", maximum_frequency);
	}
	
	target_frequency = minimum_frequency > target_frequency ? minimum_frequency : target_frequency;
	target_frequency = target_frequency > maximum_frequency ? maximum_frequency : target_frequency;
	target_frequency = _enabled ? target_frequency : 0;
	
	return target_frequency;
}

void SpindleLaser::refresh() {
	current_frequency = read_frequency();
	current_rpm = (((SPEED_POWER_MAX) * 1.0f)/3000.0f) * current_frequency;
	
	uint8_t status = read_status();
	
	current_direction = (status & STATUS_REVERSE) ? SpindleDirection::REVERSE : SpindleDirection::FORWARD;
}

void SpindleLaser::apply() {
	_target_frequency = calculate_target_freq();

	refresh();
	
	if(current_frequency == _target_frequency && current_direction == _target_direction) {
		return;
	}
	
	if(DEBUGGING(SPINDLE)) {
		SERIAL_ECHOLNPAIR("current_frequency:", current_frequency);
		SERIAL_ECHOLNPAIR("current_direction:", current_direction == SpindleDirection::REVERSE ? "reverse" : "forward");
		SERIAL_ECHOLNPAIR("target_frequency: ", _target_frequency);
		SERIAL_ECHOLNPAIR("target_direction:", _target_direction == SpindleDirection::REVERSE ? "reverse" : "forward");
	}
	
	if(_enabled) {
		if(current_direction != _target_direction) {
			SPINDLE_DEBUG(SERIAL_ECHOLN("Changing direction"));
			
			SPINDLE_DEBUG(SERIAL_ECHOLN("Setting frequency to 0"));
			modbus::write_holding_register(SPINDLE_MODBUS_ADDRESS, 0x0201, 0x0000);
			
			SPINDLE_DEBUG(SERIAL_ECHOLN("Setting operation to stop"));
			modbus::write_parameter(SPINDLE_MODBUS_ADDRESS, 0x0048, 0x0000);
			
			wait_for_spindle(0);
		}
		
		SPINDLE_DEBUG(SERIAL_PRINTF("Setting direction to %s\n", _target_direction == SpindleDirection::FORWARD ? "forward" : "reverse"));
		modbus::write_parameter(SPINDLE_MODBUS_ADDRESS, 0x0049, _target_direction == SpindleDirection::FORWARD ? 0xFF00 : 0x0000);
		modbus::write_parameter(SPINDLE_MODBUS_ADDRESS, 0x004A, _target_direction == SpindleDirection::REVERSE ? 0xFF00 : 0x0000);
		
		current_direction = _target_direction;
	}
	
	SPINDLE_DEBUG(SERIAL_PRINTF("Setting target frequency to %d\n", _target_frequency));
	modbus::write_holding_register(SPINDLE_MODBUS_ADDRESS, 0x0201, _target_frequency * 10);
		
	SPINDLE_DEBUG(SERIAL_PRINTF("Setting operation to %s\n", _enabled ? "run" : "stop"));
	modbus::write_parameter(SPINDLE_MODBUS_ADDRESS, 0x0048, _enabled ? 0xFF00 : 0x0000);
		
	wait_for_spindle(_target_frequency);
}

#endif // HAS_CUTTER
