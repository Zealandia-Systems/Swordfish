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
 * temperature.h - temperature controller
 */

#include "thermistor/thermistors.h"

#include "../inc/MarlinConfig.h"

// Heater identifiers. Positive values are hotends. Negative values are other heaters.
typedef enum : int8_t {
	INDEX_NONE = -5,
	H_PROBE,
	H_REDUNDANT,
	H_CHAMBER,
	H_BED,
	H_E0,
	H_E1,
	H_E2,
	H_E3,
	H_E4,
	H_E5,
	H_E6,
	H_E7
} heater_id_t;

// PID storage
typedef struct {
	float Kp, Ki, Kd;
} PID_t;
typedef struct {
	float Kp, Ki, Kd, Kc;
} PIDC_t;
typedef struct {
	float Kp, Ki, Kd, Kf;
} PIDF_t;
typedef struct {
	float Kp, Ki, Kd, Kc, Kf;
} PIDCF_t;

typedef
#if BOTH(PID_EXTRUSION_SCALING, PID_FAN_SCALING)
		PIDCF_t
#elif ENABLED(PID_EXTRUSION_SCALING)
		PIDC_t
#elif ENABLED(PID_FAN_SCALING)
		PIDF_t
#else
		PID_t
#endif
				hotend_pid_t;

class Temperature {
public:
	void init();
	static void tick();

	/**
	 * Call periodically to manage heaters
	 */
	static void manage_heater() OPT_O2; // Added _O2 to work around a compiler error
};

extern Temperature thermalManager;
