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

//
// Prepare a list of protected pins for M42/M43
//

#if PIN_EXISTS(X_MIN)
  #define _X_MIN X_MIN_PIN,
#else
  #define _X_MIN
#endif
#if PIN_EXISTS(X_MAX)
  #define _X_MAX X_MAX_PIN,
#else
  #define _X_MAX
#endif
#if PIN_EXISTS(X_CS)
  #define _X_CS X_CS_PIN,
#else
  #define _X_CS
#endif
#if PIN_EXISTS(X_MS1)
  #define _X_MS1 X_MS1_PIN,
#else
  #define _X_MS1
#endif
#if PIN_EXISTS(X_MS2)
  #define _X_MS2 X_MS2_PIN,
#else
  #define _X_MS2
#endif
#if PIN_EXISTS(X_MS3)
  #define _X_MS3 X_MS3_PIN,
#else
  #define _X_MS3
#endif

#define _X_PINS X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, _X_MIN _X_MAX _X_MS1 _X_MS2 _X_MS3 _X_CS

#if PIN_EXISTS(Y_MIN)
  #define _Y_MIN Y_MIN_PIN,
#else
  #define _Y_MIN
#endif
#if PIN_EXISTS(Y_MAX)
  #define _Y_MAX Y_MAX_PIN,
#else
  #define _Y_MAX
#endif
#if PIN_EXISTS(Y_CS)
  #define _Y_CS Y_CS_PIN,
#else
  #define _Y_CS
#endif
#if PIN_EXISTS(Y_MS1)
  #define _Y_MS1 Y_MS1_PIN,
#else
  #define _Y_MS1
#endif
#if PIN_EXISTS(Y_MS2)
  #define _Y_MS2 Y_MS2_PIN,
#else
  #define _Y_MS2
#endif
#if PIN_EXISTS(Y_MS3)
  #define _Y_MS3 Y_MS3_PIN,
#else
  #define _Y_MS3
#endif

#define _Y_PINS Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, _Y_MIN _Y_MAX _Y_MS1 _Y_MS2 _Y_MS3 _Y_CS

#if PIN_EXISTS(Z_MIN)
  #define _Z_MIN Z_MIN_PIN,
#else
  #define _Z_MIN
#endif
#if PIN_EXISTS(Z_MAX)
  #define _Z_MAX Z_MAX_PIN,
#else
  #define _Z_MAX
#endif
#if PIN_EXISTS(Z_CS)
  #define _Z_CS Z_CS_PIN,
#else
  #define _Z_CS
#endif
#if PIN_EXISTS(Z_MS1)
  #define _Z_MS1 Z_MS1_PIN,
#else
  #define _Z_MS1
#endif
#if PIN_EXISTS(Z_MS2)
  #define _Z_MS2 Z_MS2_PIN,
#else
  #define _Z_MS2
#endif
#if PIN_EXISTS(Z_MS3)
  #define _Z_MS3 Z_MS3_PIN,
#else
  #define _Z_MS3
#endif

#define _Z_PINS Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, _Z_MIN _Z_MAX _Z_MS1 _Z_MS2 _Z_MS3 _Z_CS


#if PIN_EXISTS(A_MIN)
  #define _A_MIN A_MIN_PIN,
#else
  #define _A_MIN
#endif
#if PIN_EXISTS(A_MAX)
  #define _A_MAX A_MAX_PIN,
#else
  #define _A_MAX
#endif
#if PIN_EXISTS(A_CS)
  #define _A_CS A_CS_PIN,
#else
  #define _A_CS
#endif
#if PIN_EXISTS(A_MS1)
  #define _A_MS1 A_MS1_PIN,
#else
  #define _A_MS1
#endif
#if PIN_EXISTS(A_MS2)
  #define _A_MS2 A_MS2_PIN,
#else
  #define _A_MS2
#endif
#if PIN_EXISTS(A_MS3)
  #define _A_MS3 A_MS3_PIN,
#else
  #define _A_MS3
#endif

#define _A_PINS A_STEP_PIN, A_DIR_PIN, A_ENABLE_PIN, _A_MIN _A_MAX _A_MS1 _A_MS2 _A_MS3 _A_CS


//
// Dual X, Dual Y, Multi-Z
// Chip Select and Digital Micro-stepping
//

#if EITHER(DUAL_X_CARRIAGE, X_DUAL_STEPPER_DRIVERS)
  #if PIN_EXISTS(X2_CS) && AXIS_HAS_SPI(X2)
    #define _X2_CS X2_CS_PIN,
  #else
    #define _X2_CS
  #endif
  #if PIN_EXISTS(X2_MS1)
    #define _X2_MS1 X2_MS1_PIN,
  #else
    #define _X2_MS1
  #endif
  #if PIN_EXISTS(X2_MS2)
    #define _X2_MS2 X2_MS2_PIN,
  #else
    #define _X2_MS2
  #endif
  #if PIN_EXISTS(X2_MS3)
    #define _X2_MS3 X2_MS3_PIN,
  #else
    #define _X2_MS3
  #endif
  #define _X2_PINS X2_STEP_PIN, X2_DIR_PIN, X2_ENABLE_PIN, _X2_CS _X2_MS1 _X2_MS2 _X2_MS3
#else
  #define _X2_PINS
#endif

#if ENABLED(Y_DUAL_STEPPER_DRIVERS)
  #if PIN_EXISTS(Y2_CS)
    #define _Y2_CS Y2_CS_PIN,
  #else
    #define _Y2_CS
  #endif
  #if PIN_EXISTS(Y2_MS1)
    #define _Y2_MS1 Y2_MS1_PIN,
  #else
    #define _Y2_MS1
  #endif
  #if PIN_EXISTS(Y2_MS2)
    #define _Y2_MS2 Y2_MS2_PIN,
  #else
    #define _Y2_MS2
  #endif
  #if PIN_EXISTS(Y2_MS3)
    #define _Y2_MS3 Y2_MS3_PIN,
  #else
    #define _Y2_MS3
  #endif
  #define _Y2_PINS Y2_STEP_PIN, Y2_DIR_PIN, Y2_ENABLE_PIN, _Y2_CS _Y2_MS1 _Y2_MS2 _Y2_MS3
#else
  #define _Y2_PINS
#endif

#if NUM_Z_STEPPER_DRIVERS >= 2
  #if PIN_EXISTS(Z2_CS) && AXIS_HAS_SPI(Z2)
    #define _Z2_CS Z2_CS_PIN,
  #else
    #define _Z2_CS
  #endif
  #if PIN_EXISTS(Z2_MS1)
    #define _Z2_MS1 Z2_MS1_PIN,
  #else
    #define _Z2_MS1
  #endif
  #if PIN_EXISTS(Z2_MS2)
    #define _Z2_MS2 Z2_MS2_PIN,
  #else
    #define _Z2_MS2
  #endif
  #if PIN_EXISTS(Z2_MS3)
    #define _Z2_MS3 Z2_MS3_PIN,
  #else
    #define _Z2_MS3
  #endif
  #define _Z2_PINS Z2_STEP_PIN, Z2_DIR_PIN, Z2_ENABLE_PIN, _Z2_CS _Z2_MS1 _Z2_MS2 _Z2_MS3
#else
  #define _Z2_PINS
#endif

#if NUM_Z_STEPPER_DRIVERS >= 3
  #if PIN_EXISTS(Z3_CS) && AXIS_HAS_SPI(Z3)
    #define _Z3_CS Z3_CS_PIN,
  #else
    #define _Z3_CS
  #endif
  #if PIN_EXISTS(Z3_MS1)
    #define _Z3_MS1 Z3_MS1_PIN,
  #else
    #define _Z3_MS1
  #endif
  #if PIN_EXISTS(Z3_MS2)
    #define _Z3_MS2 Z3_MS2_PIN,
  #else
    #define _Z3_MS2
  #endif
  #if PIN_EXISTS(Z3_MS3)
    #define _Z3_MS3 Z3_MS3_PIN,
  #else
    #define _Z3_MS3
  #endif
  #define _Z3_PINS Z3_STEP_PIN, Z3_DIR_PIN, Z3_ENABLE_PIN, _Z3_CS _Z3_MS1 _Z3_MS2 _Z3_MS3
#else
  #define _Z3_PINS
#endif

#if NUM_Z_STEPPER_DRIVERS >= 4
  #if PIN_EXISTS(Z4_CS) && AXIS_HAS_SPI(Z4)
    #define _Z4_CS Z4_CS_PIN,
  #else
    #define _Z4_CS
  #endif
  #if PIN_EXISTS(Z4_MS1)
    #define _Z4_MS1 Z4_MS1_PIN,
  #else
    #define _Z4_MS1
  #endif
  #if PIN_EXISTS(Z4_MS2)
    #define _Z4_MS2 Z4_MS2_PIN,
  #else
    #define _Z4_MS2
  #endif
  #if PIN_EXISTS(Z4_MS3)
    #define _Z4_MS3 Z4_MS3_PIN,
  #else
    #define _Z4_MS3
  #endif
  #define _Z4_PINS Z4_STEP_PIN, Z4_DIR_PIN, Z4_ENABLE_PIN, _Z4_CS _Z4_MS1 _Z4_MS2 _Z4_MS3
#else
  #define _Z4_PINS
#endif

//
// Generate the final Sensitive Pins array,
// keeping the array as small as possible.
//

#if PIN_EXISTS(PS_ON)
  #define _PS_ON PS_ON_PIN,
#else
  #define _PS_ON
#endif

#if HAS_BED_PROBE && PIN_EXISTS(Z_MIN_PROBE)
  #define _Z_PROBE Z_MIN_PROBE_PIN,
#else
  #define _Z_PROBE
#endif

#if PIN_EXISTS(FAN)
  #define _FAN0 FAN_PIN,
#else
  #define _FAN0
#endif
#if PIN_EXISTS(FAN1)
  #define _FAN1 FAN1_PIN,
#else
  #define _FAN1
#endif
#if PIN_EXISTS(FAN2)
  #define _FAN2 FAN2_PIN,
#else
  #define _FAN2
#endif
#if PIN_EXISTS(FAN3)
  #define _FAN3 FAN3_PIN,
#else
  #define _FAN3
#endif
#if PIN_EXISTS(FAN4)
  #define _FAN4 FAN4_PIN,
#else
  #define _FAN4
#endif
#if PIN_EXISTS(FAN5)
  #define _FAN5 FAN5_PIN,
#else
  #define _FAN5
#endif
#if PIN_EXISTS(FAN6)
  #define _FAN6 FAN6_PIN,
#else
  #define _FAN6
#endif
#if PIN_EXISTS(FAN7)
  #define _FAN7 FAN7_PIN,
#else
  #define _FAN7
#endif
#if PIN_EXISTS(CONTROLLER_FAN)
  #define _FANC CONTROLLER_FAN_PIN,
#else
  #define _FANC
#endif

#if TEMP_SENSOR_BED && PINS_EXIST(TEMP_BED, HEATER_BED)
  #define _BED_PINS HEATER_BED_PIN, analogInputToDigitalPin(TEMP_BED_PIN),
#else
  #define _BED_PINS
#endif

#if TEMP_SENSOR_CHAMBER && PIN_EXISTS(TEMP_CHAMBER)
  #define _CHAMBER_TEMP analogInputToDigitalPin(TEMP_CHAMBER_PIN),
#else
  #define _CHAMBER_TEMP
#endif
#if TEMP_SENSOR_CHAMBER && PINS_EXIST(TEMP_CHAMBER, HEATER_CHAMBER)
  #define _CHAMBER_HEATER HEATER_CHAMBER_PIN,
#else
  #define _CHAMBER_HEATER
#endif
#if TEMP_SENSOR_CHAMBER && PINS_EXIST(TEMP_CHAMBER, CHAMBER_AUTO_FAN)
  #define _CHAMBER_FAN CHAMBER_AUTO_FAN_PIN,
#else
  #define _CHAMBER_FAN
#endif

#ifndef HAL_SENSITIVE_PINS
  #define HAL_SENSITIVE_PINS
#endif

#define SENSITIVE_PINS { \
  _X_PINS _Y_PINS _Z_PINS _X2_PINS _Y2_PINS _Z2_PINS _Z3_PINS _Z4_PINS _Z_PROBE \
  _A_PINS \
  _PS_ON _FAN0 _FAN1 _FAN2 _FAN3 _FAN4 _FAN5 _FAN6 _FAN7 _FANC \
  _BED_PINS _CHAMBER_TEMP _CHAMBER_HEATER _CHAMBER_FAN HAL_SENSITIVE_PINS \
}
