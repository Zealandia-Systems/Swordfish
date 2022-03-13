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
#pragma once

/**
 * feature/spindle_laser.h
 * Support for Laser Power or Spindle Power & Direction
 */

#include "../inc/MarlinConfig.h"

#include "spindle_laser_types.h"

#if USE_BEEPER
  #include "../libs/buzzer.h"
#endif

#if ENABLED(LASER_POWER_INLINE)
  #include "../module/planner.h"
#endif

enum class SpindleDirection {
	FORWARD = 0,
	REVERSE = 1
};

class SpindleLaser {
private:
	static bool _enabled;
	static uint16_t _override;
	
	static uint16_t _target_frequency;
	static SpindleDirection _target_direction;
	static uint16_t _target_rpm;
	
	static uint16_t read_frequency();
	static uint8_t read_status();
	static uint16_t calculate_target_freq();
	static void wait_for_spindle(uint16_t target_freq);
	static void apply();
	static void refresh();
	
public:
  static void init();
	static void refresh_current() {
		if(!_enabled) {
			return;
		}
		
		static millis_t next_refresh = 0;
			
		auto ms = millis();
			
		if(ELAPSED(ms, next_refresh)) {
			refresh();
				
			next_refresh = ms + 100;
		}
	}
			
	static uint16_t current_frequency;
	static SpindleDirection current_direction; // 0 = forward, 1 = reverse
	static uint16_t current_rpm;
	static uint16_t minimum_frequency;
	static uint16_t maximum_frequency;

	static void enabled(const bool enabled) {
		const auto changed = _enabled != enabled;
		
		_enabled = enabled;
		
		if(changed) {
			apply();
		}
	}
	
	static bool enabled() { return _enabled; }

	static void rpm(const uint16_t rpm) {
		const auto changed = _target_rpm != rpm;
		
		_target_rpm = rpm;
		
		if(changed) {
			apply();
		}
	}
	
	static inline uint16_t rpm() {
		return _target_rpm;
	}
	
	static void direction(const SpindleDirection direction) {
		const auto changed = _target_direction != direction;
		
		_target_direction = direction;
		
		if(changed) {
			apply();
		}
	}

	static SpindleDirection direction() { return _target_direction; }

	static inline uint16_t override() {
		return _override;
	}
	
	static inline void override(const uint16_t override) {
		_override = override;
	}
};

extern SpindleLaser cutter;
