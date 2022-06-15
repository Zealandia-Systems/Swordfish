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
 * stepper/indirection.h
 *
 * Stepper motor driver indirection to allow some stepper functions to
 * be done via SPI/I2c instead of direct pin manipulation.
 *
 * Copyright (c) 2015 Dominik Wenger
 */

#include "../../inc/MarlinConfig.h"

#if HAS_L64XX
#	include "L64xx.h"
#endif

#if HAS_DRIVER(TMC26X)
#	include "TMC26X.h"
#endif

#if HAS_TRINAMIC_CONFIG
#	include "trinamic.h"
#endif

void restore_stepper_drivers(); // Called by PSU_ON
void reset_stepper_drivers(); // Called by settings.load / settings.reset

// X Stepper
#ifndef X_ENABLE_INIT
#	define X_ENABLE_INIT()       SET_OUTPUT(X_ENABLE_PIN)
#	define X_ENABLE_WRITE(STATE) WRITE(X_ENABLE_PIN, STATE)
#	define X_ENABLE_READ()       bool(READ(X_ENABLE_PIN))
#endif
#ifndef X_DIR_INIT
#	define X_DIR_INIT()       SET_OUTPUT(X_DIR_PIN)
#	define X_DIR_WRITE(STATE) WRITE(X_DIR_PIN, STATE)
#	define X_DIR_READ()       bool(READ(X_DIR_PIN))
#endif
#define X_STEP_INIT() SET_OUTPUT(X_STEP_PIN)
#ifndef X_STEP_WRITE
#	define X_STEP_WRITE(STATE) WRITE(X_STEP_PIN, STATE)
#endif
#define X_STEP_READ() bool(READ(X_STEP_PIN))

// Y Stepper
#ifndef Y_ENABLE_INIT
#	define Y_ENABLE_INIT()       SET_OUTPUT(Y_ENABLE_PIN)
#	define Y_ENABLE_WRITE(STATE) WRITE(Y_ENABLE_PIN, STATE)
#	define Y_ENABLE_READ()       bool(READ(Y_ENABLE_PIN))
#endif
#ifndef Y_DIR_INIT
#	define Y_DIR_INIT()       SET_OUTPUT(Y_DIR_PIN)
#	define Y_DIR_WRITE(STATE) WRITE(Y_DIR_PIN, STATE)
#	define Y_DIR_READ()       bool(READ(Y_DIR_PIN))
#endif
#define Y_STEP_INIT() SET_OUTPUT(Y_STEP_PIN)
#ifndef Y_STEP_WRITE
#	define Y_STEP_WRITE(STATE) WRITE(Y_STEP_PIN, STATE)
#endif
#define Y_STEP_READ() bool(READ(Y_STEP_PIN))

// Z Stepper
#ifndef Z_ENABLE_INIT
#	define Z_ENABLE_INIT()       SET_OUTPUT(Z_ENABLE_PIN)
#	define Z_ENABLE_WRITE(STATE) WRITE(Z_ENABLE_PIN, STATE)
#	define Z_ENABLE_READ()       bool(READ(Z_ENABLE_PIN))
#endif
#ifndef Z_DIR_INIT
#	define Z_DIR_INIT()       SET_OUTPUT(Z_DIR_PIN)
#	define Z_DIR_WRITE(STATE) WRITE(Z_DIR_PIN, STATE)
#	define Z_DIR_READ()       bool(READ(Z_DIR_PIN))
#endif
#define Z_STEP_INIT() SET_OUTPUT(Z_STEP_PIN)
#ifndef Z_STEP_WRITE
#	define Z_STEP_WRITE(STATE) WRITE(Z_STEP_PIN, STATE)
#endif
#define Z_STEP_READ() bool(READ(Z_STEP_PIN))
#ifndef Z_BRAKE_INIT
#	define Z_BRAKE_INIT()       SET_OUTPUT(Z_BRAKE_PIN)
#	define Z_BRAKE_WRITE(STATE) WRITE(Z_BRAKE_PIN, STATE != Z_BRAKE_PIN_INVERTED)
#	define Z_BRAKE_READ()       bool(READ(Z_BRAKE_PIN) != Z_BRAKE_PIN_INVERTED)
#endif

// X2 Stepper
#if HAS_X2_ENABLE
#	ifndef X2_ENABLE_INIT
#		define X2_ENABLE_INIT()       SET_OUTPUT(X2_ENABLE_PIN)
#		define X2_ENABLE_WRITE(STATE) WRITE(X2_ENABLE_PIN, STATE)
#		define X2_ENABLE_READ()       bool(READ(X2_ENABLE_PIN))
#	endif
#	ifndef X2_DIR_INIT
#		define X2_DIR_INIT()       SET_OUTPUT(X2_DIR_PIN)
#		define X2_DIR_WRITE(STATE) WRITE(X2_DIR_PIN, STATE)
#		define X2_DIR_READ()       bool(READ(X2_DIR_PIN))
#	endif
#	define X2_STEP_INIT() SET_OUTPUT(X2_STEP_PIN)
#	ifndef X2_STEP_WRITE
#		define X2_STEP_WRITE(STATE) WRITE(X2_STEP_PIN, STATE)
#	endif
#	define X2_STEP_READ() bool(READ(X2_STEP_PIN))
#endif

// Y2 Stepper
#if HAS_Y2_ENABLE
#	ifndef Y2_ENABLE_INIT
#		define Y2_ENABLE_INIT()       SET_OUTPUT(Y2_ENABLE_PIN)
#		define Y2_ENABLE_WRITE(STATE) WRITE(Y2_ENABLE_PIN, STATE)
#		define Y2_ENABLE_READ()       bool(READ(Y2_ENABLE_PIN))
#	endif
#	ifndef Y2_DIR_INIT
#		define Y2_DIR_INIT()       SET_OUTPUT(Y2_DIR_PIN)
#		define Y2_DIR_WRITE(STATE) WRITE(Y2_DIR_PIN, STATE)
#		define Y2_DIR_READ()       bool(READ(Y2_DIR_PIN))
#	endif
#	define Y2_STEP_INIT() SET_OUTPUT(Y2_STEP_PIN)
#	ifndef Y2_STEP_WRITE
#		define Y2_STEP_WRITE(STATE) WRITE(Y2_STEP_PIN, STATE)
#	endif
#	define Y2_STEP_READ() bool(READ(Y2_STEP_PIN))
#else
#	define Y2_DIR_WRITE(STATE) NOOP
#endif

// Z2 Stepper
#if HAS_Z2_ENABLE
#	ifndef Z2_ENABLE_INIT
#		define Z2_ENABLE_INIT()       SET_OUTPUT(Z2_ENABLE_PIN)
#		define Z2_ENABLE_WRITE(STATE) WRITE(Z2_ENABLE_PIN, STATE)
#		define Z2_ENABLE_READ()       bool(READ(Z2_ENABLE_PIN))
#	endif
#	ifndef Z2_DIR_INIT
#		define Z2_DIR_INIT()       SET_OUTPUT(Z2_DIR_PIN)
#		define Z2_DIR_WRITE(STATE) WRITE(Z2_DIR_PIN, STATE)
#		define Z2_DIR_READ()       bool(READ(Z2_DIR_PIN))
#	endif
#	define Z2_STEP_INIT() SET_OUTPUT(Z2_STEP_PIN)
#	ifndef Z2_STEP_WRITE
#		define Z2_STEP_WRITE(STATE) WRITE(Z2_STEP_PIN, STATE)
#	endif
#	define Z2_STEP_READ() bool(READ(Z2_STEP_PIN))
#else
#	define Z2_DIR_WRITE(STATE) NOOP
#endif

// Z3 Stepper
#if HAS_Z3_ENABLE
#	ifndef Z3_ENABLE_INIT
#		define Z3_ENABLE_INIT()       SET_OUTPUT(Z3_ENABLE_PIN)
#		define Z3_ENABLE_WRITE(STATE) WRITE(Z3_ENABLE_PIN, STATE)
#		define Z3_ENABLE_READ()       bool(READ(Z3_ENABLE_PIN))
#	endif
#	ifndef Z3_DIR_INIT
#		define Z3_DIR_INIT()       SET_OUTPUT(Z3_DIR_PIN)
#		define Z3_DIR_WRITE(STATE) WRITE(Z3_DIR_PIN, STATE)
#		define Z3_DIR_READ()       bool(READ(Z3_DIR_PIN))
#	endif
#	define Z3_STEP_INIT() SET_OUTPUT(Z3_STEP_PIN)
#	ifndef Z3_STEP_WRITE
#		define Z3_STEP_WRITE(STATE) WRITE(Z3_STEP_PIN, STATE)
#	endif
#	define Z3_STEP_READ() bool(READ(Z3_STEP_PIN))
#else
#	define Z3_DIR_WRITE(STATE) NOOP
#endif

// Z4 Stepper
#if HAS_Z4_ENABLE
#	ifndef Z4_ENABLE_INIT
#		define Z4_ENABLE_INIT()       SET_OUTPUT(Z4_ENABLE_PIN)
#		define Z4_ENABLE_WRITE(STATE) WRITE(Z4_ENABLE_PIN, STATE)
#		define Z4_ENABLE_READ()       bool(READ(Z4_ENABLE_PIN))
#	endif
#	ifndef Z4_DIR_INIT
#		define Z4_DIR_INIT()       SET_OUTPUT(Z4_DIR_PIN)
#		define Z4_DIR_WRITE(STATE) WRITE(Z4_DIR_PIN, STATE)
#		define Z4_DIR_READ()       bool(READ(Z4_DIR_PIN))
#	endif
#	define Z4_STEP_INIT() SET_OUTPUT(Z4_STEP_PIN)
#	ifndef Z4_STEP_WRITE
#		define Z4_STEP_WRITE(STATE) WRITE(Z4_STEP_PIN, STATE)
#	endif
#	define Z4_STEP_READ() bool(READ(Z4_STEP_PIN))
#else
#	define Z4_DIR_WRITE(STATE) NOOP
#endif

// A Stepper
#ifndef A_ENABLE_INIT
#	define A_ENABLE_INIT()       SET_OUTPUT(A_ENABLE_PIN)
#	define A_ENABLE_WRITE(STATE) WRITE(A_ENABLE_PIN, STATE)
#	define A_ENABLE_READ()       bool(READ(A_ENABLE_PIN))
#endif
#ifndef A_DIR_INIT
#	define A_DIR_INIT()       SET_OUTPUT(A_DIR_PIN)
#	define A_DIR_WRITE(STATE) WRITE(A_DIR_PIN, STATE)
#	define A_DIR_READ()       bool(READ(A_DIR_PIN))
#endif
#define A_STEP_INIT() SET_OUTPUT(A_STEP_PIN)
#ifndef A_STEP_WRITE
#	define A_STEP_WRITE(STATE) WRITE(A_STEP_PIN, STATE)
#endif
#define A_STEP_READ() bool(READ(A_STEP_PIN))

// A2 Stepper
#ifndef A2_ENABLE_INIT
#	define A2_ENABLE_INIT()       SET_OUTPUT(A2_ENABLE_PIN)
#	define A2_ENABLE_WRITE(STATE) WRITE(A2_ENABLE_PIN, STATE)
#	define A2_ENABLE_READ()       bool(READ(A2_ENABLE_PIN))
#endif
#ifndef A2_DIR_INIT
#	define A2_DIR_INIT()       SET_OUTPUT(A2_DIR_PIN)
#	define A2_DIR_WRITE(STATE) WRITE(A2_DIR_PIN, STATE)
#	define A2_DIR_READ()       bool(READ(A2_DIR_PIN))
#endif
#define A2_STEP_INIT() SET_OUTPUT(A2_STEP_PIN)
#ifndef A2_STEP_WRITE
#	define A2_STEP_WRITE(STATE) WRITE(A2_STEP_PIN, STATE)
#endif
#define A2_STEP_READ() bool(READ(A2_STEP_PIN))

//
// Individual stepper enable / disable macros
//

#ifndef ENABLE_STEPPER_X
#	if HAS_X_ENABLE
#		define ENABLE_STEPPER_X() X_ENABLE_WRITE(X_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_X() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_X
#	if HAS_X_ENABLE
#		define DISABLE_STEPPER_X() X_ENABLE_WRITE(!X_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_X() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_X2
#	if HAS_X2_ENABLE
#		define ENABLE_STEPPER_X2() X2_ENABLE_WRITE(X_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_X2() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_X2
#	if HAS_X2_ENABLE
#		define DISABLE_STEPPER_X2() X2_ENABLE_WRITE(!X_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_X2() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Y
#	if HAS_Y_ENABLE
#		define ENABLE_STEPPER_Y() Y_ENABLE_WRITE(Y_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Y() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Y
#	if HAS_Y_ENABLE
#		define DISABLE_STEPPER_Y() Y_ENABLE_WRITE(!Y_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Y() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Y2
#	if HAS_Y2_ENABLE
#		define ENABLE_STEPPER_Y2() Y2_ENABLE_WRITE(Y_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Y2() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Y2
#	if HAS_Y2_ENABLE
#		define DISABLE_STEPPER_Y2() Y2_ENABLE_WRITE(!Y_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Y2() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Z
#	if HAS_Z_ENABLE
#		define ENABLE_STEPPER_Z() Z_ENABLE_WRITE(Z_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Z() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Z
#	if HAS_Z_ENABLE
#		define DISABLE_STEPPER_Z() Z_ENABLE_WRITE(!Z_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Z() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Z2
#	if HAS_Z2_ENABLE
#		define ENABLE_STEPPER_Z2() Z2_ENABLE_WRITE(Z_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Z2() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Z2
#	if HAS_Z2_ENABLE
#		define DISABLE_STEPPER_Z2() Z2_ENABLE_WRITE(!Z_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Z2() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Z3
#	if HAS_Z3_ENABLE
#		define ENABLE_STEPPER_Z3() Z3_ENABLE_WRITE(Z_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Z3() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Z3
#	if HAS_Z3_ENABLE
#		define DISABLE_STEPPER_Z3() Z3_ENABLE_WRITE(!Z_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Z3() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_Z4
#	if HAS_Z4_ENABLE
#		define ENABLE_STEPPER_Z4() Z4_ENABLE_WRITE(Z_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_Z4() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_Z4
#	if HAS_Z4_ENABLE
#		define DISABLE_STEPPER_Z4() Z4_ENABLE_WRITE(!Z_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_Z4() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_A
#	if HAS_A_ENABLE
#		define ENABLE_STEPPER_A() A_ENABLE_WRITE(A_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_A() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_A
#	if HAS_A_ENABLE
#		define DISABLE_STEPPER_A() A_ENABLE_WRITE(!A_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_A() NOOP
#	endif
#endif

#ifndef ENABLE_STEPPER_A2
#	if HAS_A2_ENABLE
#		define ENABLE_STEPPER_A2() A2_ENABLE_WRITE(A2_ENABLE_ON)
#	else
#		define ENABLE_STEPPER_A2() NOOP
#	endif
#endif
#ifndef DISABLE_STEPPER_A2
#	if HAS_A2_ENABLE
#		define DISABLE_STEPPER_A2() A2_ENABLE_WRITE(!A2_ENABLE_ON)
#	else
#		define DISABLE_STEPPER_A2() NOOP
#	endif
#endif

//
// Axis steppers enable / disable macros
//

extern xyza_bool_t axis_enabled;

#define SHOULD_ENABLE(N)    !axis_enabled.N
#define SHOULD_DISABLE(N)   axis_enabled.N
#define AFTER_CHANGE(N, TF) axis_enabled.N = TF

#ifdef HAS_Z_BRAKE
#	define BRAKE_Z()   Z_BRAKE_WRITE(LOW);
#	define UNBRAKE_Z() Z_BRAKE_WRITE(HIGH);
#else
#	define BRAKE_Z()   NOOP
#	define UNBRAKE_Z() NOOP
#endif

#define ENABLE_AXIS_X() \
	if (SHOULD_ENABLE(x)) { \
		ENABLE_STEPPER_X(); \
		ENABLE_STEPPER_X2(); \
		AFTER_CHANGE(x, true); \
	}
#define DISABLE_AXIS_X() \
	if (SHOULD_DISABLE(x)) { \
		DISABLE_STEPPER_X(); \
		DISABLE_STEPPER_X2(); \
		AFTER_CHANGE(x, false); \
		set_axis_untrusted(X_AXIS); \
	}
#define ENABLE_AXIS_Y() \
	if (SHOULD_ENABLE(y)) { \
		ENABLE_STEPPER_Y(); \
		ENABLE_STEPPER_Y2(); \
		AFTER_CHANGE(y, true); \
	}
#define DISABLE_AXIS_Y() \
	if (SHOULD_DISABLE(y)) { \
		DISABLE_STEPPER_Y(); \
		DISABLE_STEPPER_Y2(); \
		AFTER_CHANGE(y, false); \
		set_axis_untrusted(Y_AXIS); \
	}
#define ENABLE_AXIS_Z() \
	if (SHOULD_ENABLE(z)) { \
		ENABLE_STEPPER_Z(); \
		safe_delay(100); \
		UNBRAKE_Z(); \
		AFTER_CHANGE(z, true); \
	}
#define DISABLE_AXIS_Z() \
	if (SHOULD_DISABLE(z)) { \
		BRAKE_Z(); \
		DISABLE_STEPPER_Z(); \
		AFTER_CHANGE(z, false); \
		set_axis_untrusted(Z_AXIS); \
		Z_RESET(); \
	}
#define ENABLE_AXIS_A() \
	if (SHOULD_ENABLE(a)) { \
		ENABLE_STEPPER_A(); \
		ENABLE_STEPPER_A2(); \
		AFTER_CHANGE(a, true); \
	}
#define DISABLE_AXIS_A() \
	if (SHOULD_DISABLE(a)) { \
		DISABLE_STEPPER_A(); \
		DISABLE_STEPPER_A2(); \
		AFTER_CHANGE(a, false); \
		set_axis_untrusted(A_AXIS); \
	}

#ifdef Z_AFTER_DEACTIVATE
#	define Z_RESET() \
		do { \
			current_position.z = Z_AFTER_DEACTIVATE; \
			sync_plan_position(); \
		} while (0)
#else
#	define Z_RESET()
#endif