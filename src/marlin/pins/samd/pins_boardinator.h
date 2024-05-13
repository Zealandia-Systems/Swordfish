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
 * AGCM4 with RAMPS v1.4.4 pin assignments
 */

#if NOT_TARGET(ARDUINO_GRAND_CENTRAL_M4)
  #error "Oops! Select 'Adafruit Grand Central M4' in 'Tools > Board.'"
#endif

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "AGCM4 Boardinator"
#endif

//
// EEPROM
//
#define QSPI_EEPROM                             // Use AGCM4 on board QSPI EEPROM (Uses 4K of RAM)
//#define I2C_EEPROM                                // EEPROM on I2C-0
#define MARLIN_EEPROM_SIZE     0x7A1200  // 8MB (GD25Q64)

//
// Limit Switches
//

#define X_MIN_PIN              70
#if MACHINE_TYPE == 5 || MACHINE_TYPE == 6 || MACHINE_TYPE == 7 || MACHINE_TYPE == 8 || MACHINE_TYPE == 9
#define Y_MIN_PIN 71
#define Y_MAX_PIN -1
#define Y2_MIN_PIN 72
#else
#define Y_MAX_PIN              71
#define Y_MIN_PIN              -1
#define Y2_MAX_PIN             72
#endif
#define Z_MAX_PIN              73
#define A_MAX_PIN              57

#define HAS_TOOL_PROBE          1
#define TOOL_PROBE_PIN          4

#define HAS_WORK_PROBE          1
#define WORK_PROBE_PIN          5

#define HAS_ESTOP               1
#define ESTOP_PIN               7

#define HAS_ATC                 1
#define ATC_LOCK_PIN           49
#define ATC_SEAL_PIN           53
#define ATC_EJECT_SENSOR_PIN    6
#define ATC_LOCK_SENSOR_PIN    18
#define ATC_MANUAL_SWITCH_PIN  19
#define ATC_DOCK_SENSOR_PIN    38

#define ATC_LOCK_PIN_INVERTED						false
#define ATC_SEAL_PIN_INVERTED						false
#define ATC_EJECT_SENSOR_PIN_INVERTED		false
#define ATC_LOCK_SENSOR_PIN_INVERTED		false
#define ATC_DOCK_SENSOR_PIN_INVERTED		true
#define ATC_MANUAL_SWITCH_PIN_INVERTED	false

#define SPINDLE_FAN_PIN                 26
#define SPINDLE_FAN_PIN_INVERTED        false
//
// Steppers
//
#define X_STEP_PIN             59
#define X_DIR_PIN              58
#define X_ENABLE_PIN           44
#define X_CS_PIN               74

#define Y_STEP_PIN             60
#define Y_DIR_PIN              61
#define Y_ENABLE_PIN           45
#define Y_CS_PIN               54

#define Y2_STEP_PIN            37
#define Y2_DIR_PIN             36
#define Y2_ENABLE_PIN          46
#define Y2_CS_PIN              55

#define Z_STEP_PIN             35
#define Z_DIR_PIN              34
#define Z_ENABLE_PIN           47
#define Z_CS_PIN               56
#define Z_BRAKE_PIN            12
#define Z_BRAKE_PIN_INVERTED    0

#define A_STEP_PIN             31
#define A_DIR_PIN              30
#define A_ENABLE_PIN           48
#define A_CS_PIN               57

#define COOLANT_AIR_PIN        9
#define COOLANT_MIST_PIN       3

//
// Misc. Functions
//
#define LED_PIN                16
#define PS_ON_PIN              10

#if MACHINE_TYPE == 5
	#define LED_COUNT            50
#else
	#define LED_COUNT            1
#endif

//
// SD Support
//
#ifndef SDCARD_CONNECTION
  #define SDCARD_CONNECTION              ONBOARD
#endif

#define CLCD_HW_SPI_SPEED 1000000

#if SD_CONNECTION_IS(ONBOARD)
  #undef SDSS
  #define SDSS                                83
  #undef SD_DETECT_PIN
  #define SD_DETECT_PIN                       95
	#undef SD_DETECT_STATE
	#define SD_DETECT_STATE HIGH
#endif
