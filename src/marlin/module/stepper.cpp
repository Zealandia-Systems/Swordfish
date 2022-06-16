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
 * stepper.cpp - A singleton object to execute motion plans using stepper motors
 * Marlin Firmware
 *
 * Derived from Grbl
 * Copyright (c) 2009-2011 Simen Svale Skogsrud
 *
 * Grbl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grbl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grbl.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * Timer calculations informed by the 'RepRap cartesian firmware' by Zack Smith
 * and Philipp Tiefenbacher.
 */

/**
 *         __________________________
 *        /|                        |\     _________________         ^
 *       / |                        | \   /|               |\        |
 *      /  |                        |  \ / |               | \       s
 *     /   |                        |   |  |               |  \      p
 *    /    |                        |   |  |               |   \     e
 *   +-----+------------------------+---+--+---------------+----+    e
 *   |               BLOCK 1            |      BLOCK 2          |    d
 *
 *                           time ----->
 *
 *  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates
 *  first block->accelerate_until step_events_completed, then keeps going at constant speed until
 *  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
 *  The slope of acceleration is calculated using v = u + at where t is the accumulated timer values of the steps so far.
 */

/**
 * Marlin uses the Bresenham algorithm. For a detailed explanation of theory and
 * method see https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
 */

/**
 * Jerk controlled movements planner added Apr 2018 by Eduardo José Tagle.
 * Equations based on Synthethos TinyG2 sources, but the fixed-point
 * implementation is new, as we are running the ISR with a variable period.
 * Also implemented the Bézier velocity curve evaluation in ARM assembler,
 * to avoid impacting ISR speed.
 */

#include "stepper.h"
#include <swordfish/debug.h>

Stepper stepper; // Singleton

#define BABYSTEPPING_EXTRA_DIR_WAIT

#ifdef __AVR__
#	include "speed_lookuptable.h"
#endif

#include "endstops.h"
#include "planner.h"
#include "motion.h"

#include "../gcode/queue.h"
#include "../sd/cardreader.h"
#include "../MarlinCore.h"
#include "../HAL/shared/Delay.h"

#if ENABLED(POWER_LOSS_RECOVERY)
#	include "../feature/powerloss.h"
#endif

// public:

#if EITHER(HAS_EXTRA_ENDSTOPS, Z_STEPPER_AUTO_ALIGN)
bool Stepper::separate_multi_axis = false;
#endif

// private:

block_t* Stepper::current_block; // (= nullptr) A pointer to the block currently being traced

uint8_t Stepper::last_direction_bits, // = 0
		Stepper::axis_did_move; // = 0

bool Stepper::abort_current_block;

#if DISABLED(MIXING_EXTRUDER) && HAS_MULTI_EXTRUDER
uint8_t Stepper::last_moved_extruder = 0xFF;
#endif

#if ENABLED(X_DUAL_ENDSTOPS)
bool Stepper::locked_X_motor = false, Stepper::locked_X2_motor = false;
#endif
#if ENABLED(Y_DUAL_ENDSTOPS)
bool Stepper::locked_Y_motor = false, Stepper::locked_Y2_motor = false;
#endif

#if EITHER(Z_MULTI_ENDSTOPS, Z_STEPPER_AUTO_ALIGN)
bool Stepper::locked_Z_motor = false, Stepper::locked_Z2_motor = false
#	if NUM_Z_STEPPER_DRIVERS >= 3
		,
		 Stepper::locked_Z3_motor = false
#		if NUM_Z_STEPPER_DRIVERS >= 4
		,
		 Stepper::locked_Z4_motor = false
#		endif
#	endif
		;
#endif

uint32_t Stepper::acceleration_time, Stepper::deceleration_time;
uint8_t Stepper::steps_per_isr;

IF_DISABLED(ADAPTIVE_STEP_SMOOTHING, constexpr)
uint8_t Stepper::oversampling_factor;

xyza_long_t Stepper::delta_error { 0, 0, 0 };

xyza_ulong_t Stepper::advance_dividend { 0, 0, 0 };
uint32_t Stepper::advance_divisor = 0,
				 Stepper::step_events_completed = 0, // The number of step events executed in the current block
		Stepper::accelerate_until, // The count at which to stop accelerating
		Stepper::decelerate_after, // The count at which to start decelerating
		Stepper::step_event_count; // The total event count for the current block

#if EITHER(HAS_MULTI_EXTRUDER, MIXING_EXTRUDER)
uint8_t Stepper::stepper_extruder;
#else
constexpr uint8_t Stepper::stepper_extruder;
#endif

#if ENABLED(S_CURVE_ACCELERATION)
int32_t __attribute__((used)) Stepper::bezier_A __asm__("bezier_A"); // A coefficient in Bézier speed curve with alias for assembler
int32_t __attribute__((used)) Stepper::bezier_B __asm__("bezier_B"); // B coefficient in Bézier speed curve with alias for assembler
int32_t __attribute__((used)) Stepper::bezier_C __asm__("bezier_C"); // C coefficient in Bézier speed curve with alias for assembler
uint32_t __attribute__((used)) Stepper::bezier_F __asm__("bezier_F"); // F coefficient in Bézier speed curve with alias for assembler
uint32_t __attribute__((used)) Stepper::bezier_AV __asm__("bezier_AV"); // AV coefficient in Bézier speed curve with alias for assembler
bool Stepper::bezier_2nd_half; // =false If Bézier curve has been initialized or not
#endif

int32_t Stepper::ticks_nominal = -1;
#if DISABLED(S_CURVE_ACCELERATION)
uint32_t Stepper::acc_step_rate; // needed for deceleration start point
#endif

xyz_long_t Stepper::endstops_trigsteps;
xyza_long_t Stepper::count_position { 0, 0, 0, 0 };
xyza_int8_t Stepper::count_direction { 0, 0, 0, 0 };

#if ENABLED(LASER_POWER_INLINE_TRAPEZOID)
Stepper::stepper_laser_t Stepper::laser_trap = {
	.enabled = false,
	.cur_power = 0,
	.cruise_set = false,
#	if DISABLED(LASER_POWER_INLINE_TRAPEZOID_CONT)
	.last_step_count = 0,
	.acc_step_count = 0
#	else
	.till_update = 0
#	endif
};
#endif

#define DUAL_ENDSTOP_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (A##_HOME_DIR < 0) { \
			if (!(TEST(endstops.state(), A##_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
		} else { \
			if (!(TEST(endstops.state(), A##_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
		} \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
	}

#define DUAL_SEPARATE_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (!locked_##A##_motor) \
			A##_STEP_WRITE(V); \
		if (!locked_##A##2_motor) \
			A##2_STEP_WRITE(V); \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
	}

#define TRIPLE_ENDSTOP_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (A##_HOME_DIR < 0) { \
			if (!(TEST(endstops.state(), A##_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##3_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##3_motor) \
				A##3_STEP_WRITE(V); \
		} else { \
			if (!(TEST(endstops.state(), A##_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##3_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##3_motor) \
				A##3_STEP_WRITE(V); \
		} \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
		A##3_STEP_WRITE(V); \
	}

#define TRIPLE_SEPARATE_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (!locked_##A##_motor) \
			A##_STEP_WRITE(V); \
		if (!locked_##A##2_motor) \
			A##2_STEP_WRITE(V); \
		if (!locked_##A##3_motor) \
			A##3_STEP_WRITE(V); \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
		A##3_STEP_WRITE(V); \
	}

#define QUAD_ENDSTOP_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (A##_HOME_DIR < 0) { \
			if (!(TEST(endstops.state(), A##_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##3_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##3_motor) \
				A##3_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##4_MIN) && count_direction[_AXIS(A)] < 0) && !locked_##A##4_motor) \
				A##4_STEP_WRITE(V); \
		} else { \
			if (!(TEST(endstops.state(), A##_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##_motor) \
				A##_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##2_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##2_motor) \
				A##2_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##3_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##3_motor) \
				A##3_STEP_WRITE(V); \
			if (!(TEST(endstops.state(), A##4_MAX) && count_direction[_AXIS(A)] > 0) && !locked_##A##4_motor) \
				A##4_STEP_WRITE(V); \
		} \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
		A##3_STEP_WRITE(V); \
		A##4_STEP_WRITE(V); \
	}

#define QUAD_SEPARATE_APPLY_STEP(A, V) \
	if (separate_multi_axis) { \
		if (!locked_##A##_motor) \
			A##_STEP_WRITE(V); \
		if (!locked_##A##2_motor) \
			A##2_STEP_WRITE(V); \
		if (!locked_##A##3_motor) \
			A##3_STEP_WRITE(V); \
		if (!locked_##A##4_motor) \
			A##4_STEP_WRITE(V); \
	} else { \
		A##_STEP_WRITE(V); \
		A##2_STEP_WRITE(V); \
		A##3_STEP_WRITE(V); \
		A##4_STEP_WRITE(V); \
	}

#if ENABLED(X_DUAL_STEPPER_DRIVERS)
#	define X_APPLY_DIR(v, Q) \
		do { \
			X_DIR_WRITE(v); \
			X2_DIR_WRITE((v) ^ ENABLED(INVERT_X2_VS_X_DIR)); \
		} while (0)
#	if ENABLED(X_DUAL_ENDSTOPS)
#		define X_APPLY_STEP(v, Q) DUAL_ENDSTOP_APPLY_STEP(X, v)
#	else
#		define X_APPLY_STEP(v, Q) \
			do { \
				X_STEP_WRITE(v); \
				X2_STEP_WRITE(v); \
			} while (0)
#	endif
#elif ENABLED(DUAL_X_CARRIAGE)
#	define X_APPLY_DIR(v, ALWAYS) \
		do { \
			if (extruder_duplication_enabled || ALWAYS) { \
				X_DIR_WRITE(v); \
				X2_DIR_WRITE((v) ^ idex_mirrored_mode); \
			} else if (last_moved_extruder) \
				X2_DIR_WRITE(v); \
			else \
				X_DIR_WRITE(v); \
		} while (0)
#	define X_APPLY_STEP(v, ALWAYS) \
		do { \
			if (extruder_duplication_enabled || ALWAYS) { \
				X_STEP_WRITE(v); \
				X2_STEP_WRITE(v); \
			} else if (last_moved_extruder) \
				X2_STEP_WRITE(v); \
			else \
				X_STEP_WRITE(v); \
		} while (0)
#else
#	define X_APPLY_DIR(v, Q)  X_DIR_WRITE(v)
#	define X_APPLY_STEP(v, Q) X_STEP_WRITE(v)
#endif

#if ENABLED(Y_DUAL_STEPPER_DRIVERS)
#	define Y_APPLY_DIR(v, Q) \
		do { \
			Y_DIR_WRITE(v); \
			Y2_DIR_WRITE((v) ^ ENABLED(INVERT_Y2_VS_Y_DIR)); \
		} while (0)
#	if ENABLED(Y_DUAL_ENDSTOPS)
#		define Y_APPLY_STEP(v, Q) DUAL_ENDSTOP_APPLY_STEP(Y, v)
#	else
#		define Y_APPLY_STEP(v, Q) \
			do { \
				Y_STEP_WRITE(v); \
				Y2_STEP_WRITE(v); \
			} while (0)
#	endif
#else
#	define Y_APPLY_DIR(v, Q)  Y_DIR_WRITE(v)
#	define Y_APPLY_STEP(v, Q) Y_STEP_WRITE(v)
#endif

#if NUM_Z_STEPPER_DRIVERS == 4
#	define Z_APPLY_DIR(v, Q) \
		do { \
			Z_DIR_WRITE(v); \
			Z2_DIR_WRITE((v) ^ ENABLED(INVERT_Z2_VS_Z_DIR)); \
			Z3_DIR_WRITE((v) ^ ENABLED(INVERT_Z3_VS_Z_DIR)); \
			Z4_DIR_WRITE((v) ^ ENABLED(INVERT_Z4_VS_Z_DIR)); \
		} while (0)
#	if ENABLED(Z_MULTI_ENDSTOPS)
#		define Z_APPLY_STEP(v, Q) QUAD_ENDSTOP_APPLY_STEP(Z, v)
#	elif ENABLED(Z_STEPPER_AUTO_ALIGN)
#		define Z_APPLY_STEP(v, Q) QUAD_SEPARATE_APPLY_STEP(Z, v)
#	else
#		define Z_APPLY_STEP(v, Q) \
			do { \
				Z_STEP_WRITE(v); \
				Z2_STEP_WRITE(v); \
				Z3_STEP_WRITE(v); \
				Z4_STEP_WRITE(v); \
			} while (0)
#	endif
#elif NUM_Z_STEPPER_DRIVERS == 3
#	define Z_APPLY_DIR(v, Q) \
		do { \
			Z_DIR_WRITE(v); \
			Z2_DIR_WRITE((v) ^ ENABLED(INVERT_Z2_VS_Z_DIR)); \
			Z3_DIR_WRITE((v) ^ ENABLED(INVERT_Z3_VS_Z_DIR)); \
		} while (0)
#	if ENABLED(Z_MULTI_ENDSTOPS)
#		define Z_APPLY_STEP(v, Q) TRIPLE_ENDSTOP_APPLY_STEP(Z, v)
#	elif ENABLED(Z_STEPPER_AUTO_ALIGN)
#		define Z_APPLY_STEP(v, Q) TRIPLE_SEPARATE_APPLY_STEP(Z, v)
#	else
#		define Z_APPLY_STEP(v, Q) \
			do { \
				Z_STEP_WRITE(v); \
				Z2_STEP_WRITE(v); \
				Z3_STEP_WRITE(v); \
			} while (0)
#	endif
#elif NUM_Z_STEPPER_DRIVERS == 2
#	define Z_APPLY_DIR(v, Q) \
		do { \
			Z_DIR_WRITE(v); \
			Z2_DIR_WRITE((v) ^ ENABLED(INVERT_Z2_VS_Z_DIR)); \
		} while (0)
#	if ENABLED(Z_MULTI_ENDSTOPS)
#		define Z_APPLY_STEP(v, Q) DUAL_ENDSTOP_APPLY_STEP(Z, v)
#	elif ENABLED(Z_STEPPER_AUTO_ALIGN)
#		define Z_APPLY_STEP(v, Q) DUAL_SEPARATE_APPLY_STEP(Z, v)
#	else
#		define Z_APPLY_STEP(v, Q) \
			do { \
				Z_STEP_WRITE(v); \
				Z2_STEP_WRITE(v); \
			} while (0)
#	endif
#else
#	define Z_APPLY_DIR(v, Q)  Z_DIR_WRITE(v)
#	define Z_APPLY_STEP(v, Q) Z_STEP_WRITE(v)
#endif

#if ENABLED(A_DUAL_STEPPER_DRIVERS)
#	define A_APPLY_DIR(v, Q) \
		do { \
			A_DIR_WRITE(v); \
			A2_DIR_WRITE((v) ^ ENABLED(INVERT_A2_VS_A_DIR)); \
		} while (0)
#	if ENABLED(A_DUAL_ENDSTOPS)
#		define A_APPLY_STEP(v, Q) DUAL_ENDSTOP_APPLY_STEP(A, v)
#	else
#		define A_APPLY_STEP(v, Q) \
			do { \
				A_STEP_WRITE(v); \
				A2_STEP_WRITE(v); \
			} while (0)
#	endif
#else
#	define A_APPLY_DIR(v, Q)  A_DIR_WRITE(v)
#	define A_APPLY_STEP(v, Q) A_STEP_WRITE(v)
#endif

#define CYCLES_TO_NS(CYC)           (1000UL * (CYC) / ((F_CPU) / 1000000))
#define NS_PER_PULSE_TIMER_TICK     (1000000000UL / (STEPPER_TIMER_RATE))

// Round up when converting from ns to timer ticks
#define NS_TO_PULSE_TIMER_TICKS(NS) (((NS) + (NS_PER_PULSE_TIMER_TICK) / 2) / (NS_PER_PULSE_TIMER_TICK))

#define TIMER_SETUP_NS              (CYCLES_TO_NS(TIMER_READ_ADD_AND_STORE_CYCLES))

#define PULSE_HIGH_TICK_COUNT       hal_timer_t(NS_TO_PULSE_TIMER_TICKS(_MIN_PULSE_HIGH_NS - _MIN(_MIN_PULSE_HIGH_NS, TIMER_SETUP_NS)))
#define PULSE_LOW_TICK_COUNT        hal_timer_t(NS_TO_PULSE_TIMER_TICKS(_MIN_PULSE_LOW_NS - _MIN(_MIN_PULSE_LOW_NS, TIMER_SETUP_NS)))

#define USING_TIMED_PULSE()         hal_timer_t start_pulse_count = 0
#define START_TIMED_PULSE(DIR)      (start_pulse_count = HAL_timer_get_count(PULSE_TIMER_NUM))
#define AWAIT_TIMED_PULSE(DIR) \
	while (PULSE_##DIR##_TICK_COUNT > HAL_timer_get_count(PULSE_TIMER_NUM) - start_pulse_count) { \
	}
#define START_HIGH_PULSE() START_TIMED_PULSE(HIGH)
#define AWAIT_HIGH_PULSE() AWAIT_TIMED_PULSE(HIGH)
#define START_LOW_PULSE()  START_TIMED_PULSE(LOW)
#define AWAIT_LOW_PULSE()  AWAIT_TIMED_PULSE(LOW)

#if MINIMUM_STEPPER_PRE_DIR_DELAY > 0
#	define DIR_WAIT_BEFORE() DELAY_NS(MINIMUM_STEPPER_PRE_DIR_DELAY)
#else
#	define DIR_WAIT_BEFORE()
#endif

#if MINIMUM_STEPPER_POST_DIR_DELAY > 0
#	define DIR_WAIT_AFTER() DELAY_NS(MINIMUM_STEPPER_POST_DIR_DELAY)
#else
#	define DIR_WAIT_AFTER()
#endif

/**
 * Set the stepper direction of each axis
 */
void Stepper::set_directions() {

	DIR_WAIT_BEFORE();

#define SET_STEP_DIR(A) \
	if (motor_direction(_AXIS(A))) { \
		A##_APPLY_DIR(INVERT_##A##_DIR, false); \
		count_direction[_AXIS(A)] = -1; \
	} else { \
		A##_APPLY_DIR(!INVERT_##A##_DIR, false); \
		count_direction[_AXIS(A)] = 1; \
	}

#if HAS_X_DIR
	SET_STEP_DIR(X);
#endif
#if HAS_Y_DIR
	SET_STEP_DIR(Y);
#endif
#if HAS_Z_DIR
	SET_STEP_DIR(Z);
#endif
#if HAS_A_DIR
	SET_STEP_DIR(A);
#endif

	DIR_WAIT_AFTER();
}

#if ENABLED(S_CURVE_ACCELERATION)
/**
 *  This uses a quintic (fifth-degree) Bézier polynomial for the velocity curve, giving
 *  a "linear pop" velocity curve; with pop being the sixth derivative of position:
 *  velocity - 1st, acceleration - 2nd, jerk - 3rd, snap - 4th, crackle - 5th, pop - 6th
 *
 *  The Bézier curve takes the form:
 *
 *  V(t) = P_0 * B_0(t) + P_1 * B_1(t) + P_2 * B_2(t) + P_3 * B_3(t) + P_4 * B_4(t) + P_5 * B_5(t)
 *
 *  Where 0 <= t <= 1, and V(t) is the velocity. P_0 through P_5 are the control points, and B_0(t)
 *  through B_5(t) are the Bernstein basis as follows:
 *
 *        B_0(t) =   (1-t)^5        =   -t^5 +  5t^4 - 10t^3 + 10t^2 -  5t   +   1
 *        B_1(t) =  5(1-t)^4 * t    =   5t^5 - 20t^4 + 30t^3 - 20t^2 +  5t
 *        B_2(t) = 10(1-t)^3 * t^2  = -10t^5 + 30t^4 - 30t^3 + 10t^2
 *        B_3(t) = 10(1-t)^2 * t^3  =  10t^5 - 20t^4 + 10t^3
 *        B_4(t) =  5(1-t)   * t^4  =  -5t^5 +  5t^4
 *        B_5(t) =             t^5  =    t^5
 *                                      ^       ^       ^       ^       ^       ^
 *                                      |       |       |       |       |       |
 *                                      A       B       C       D       E       F
 *
 *  Unfortunately, we cannot use forward-differencing to calculate each position through
 *  the curve, as Marlin uses variable timer periods. So, we require a formula of the form:
 *
 *        V_f(t) = A*t^5 + B*t^4 + C*t^3 + D*t^2 + E*t + F
 *
 *  Looking at the above B_0(t) through B_5(t) expanded forms, if we take the coefficients of t^5
 *  through t of the Bézier form of V(t), we can determine that:
 *
 *        A =    -P_0 +  5*P_1 - 10*P_2 + 10*P_3 -  5*P_4 +  P_5
 *        B =   5*P_0 - 20*P_1 + 30*P_2 - 20*P_3 +  5*P_4
 *        C = -10*P_0 + 30*P_1 - 30*P_2 + 10*P_3
 *        D =  10*P_0 - 20*P_1 + 10*P_2
 *        E = - 5*P_0 +  5*P_1
 *        F =     P_0
 *
 *  Now, since we will (currently) *always* want the initial acceleration and jerk values to be 0,
 *  We set P_i = P_0 = P_1 = P_2 (initial velocity), and P_t = P_3 = P_4 = P_5 (target velocity),
 *  which, after simplification, resolves to:
 *
 *        A = - 6*P_i +  6*P_t =  6*(P_t - P_i)
 *        B =  15*P_i - 15*P_t = 15*(P_i - P_t)
 *        C = -10*P_i + 10*P_t = 10*(P_t - P_i)
 *        D = 0
 *        E = 0
 *        F = P_i
 *
 *  As the t is evaluated in non uniform steps here, there is no other way rather than evaluating
 *  the Bézier curve at each point:
 *
 *        V_f(t) = A*t^5 + B*t^4 + C*t^3 + F          [0 <= t <= 1]
 *
 * Floating point arithmetic execution time cost is prohibitive, so we will transform the math to
 * use fixed point values to be able to evaluate it in realtime. Assuming a maximum of 250000 steps
 * per second (driver pulses should at least be 2µS hi/2µS lo), and allocating 2 bits to avoid
 * overflows on the evaluation of the Bézier curve, means we can use
 *
 *   t: unsigned Q0.32 (0 <= t < 1) |range 0 to 0xFFFFFFFF unsigned
 *   A:   signed Q24.7 ,            |range = +/- 250000 * 6 * 128 = +/- 192000000 = 0x0B71B000 | 28 bits + sign
 *   B:   signed Q24.7 ,            |range = +/- 250000 *15 * 128 = +/- 480000000 = 0x1C9C3800 | 29 bits + sign
 *   C:   signed Q24.7 ,            |range = +/- 250000 *10 * 128 = +/- 320000000 = 0x1312D000 | 29 bits + sign
 *   F:   signed Q24.7 ,            |range = +/- 250000     * 128 =      32000000 = 0x01E84800 | 25 bits + sign
 *
 * The trapezoid generator state contains the following information, that we will use to create and evaluate
 * the Bézier curve:
 *
 *  blk->step_event_count [TS] = The total count of steps for this movement. (=distance)
 *  blk->initial_rate     [VI] = The initial steps per second (=velocity)
 *  blk->final_rate       [VF] = The ending steps per second  (=velocity)
 *  and the count of events completed (step_events_completed) [CS] (=distance until now)
 *
 *  Note the abbreviations we use in the following formulae are between []s
 *
 *  For Any 32bit CPU:
 *
 *    At the start of each trapezoid, calculate the coefficients A,B,C,F and Advance [AV], as follows:
 *
 *      A =  6*128*(VF - VI) =  768*(VF - VI)
 *      B = 15*128*(VI - VF) = 1920*(VI - VF)
 *      C = 10*128*(VF - VI) = 1280*(VF - VI)
 *      F =    128*VI        =  128*VI
 *     AV = (1<<32)/TS      ~= 0xFFFFFFFF / TS (To use ARM UDIV, that is 32 bits) (this is computed at the planner, to offload expensive calculations from the ISR)
 *
 *    And for each point, evaluate the curve with the following sequence:
 *
 *      void lsrs(uint32_t& d, uint32_t s, int cnt) {
 *        d = s >> cnt;
 *      }
 *      void lsls(uint32_t& d, uint32_t s, int cnt) {
 *        d = s << cnt;
 *      }
 *      void lsrs(int32_t& d, uint32_t s, int cnt) {
 *        d = uint32_t(s) >> cnt;
 *      }
 *      void lsls(int32_t& d, uint32_t s, int cnt) {
 *        d = uint32_t(s) << cnt;
 *      }
 *      void umull(uint32_t& rlo, uint32_t& rhi, uint32_t op1, uint32_t op2) {
 *        uint64_t res = uint64_t(op1) * op2;
 *        rlo = uint32_t(res & 0xFFFFFFFF);
 *        rhi = uint32_t((res >> 32) & 0xFFFFFFFF);
 *      }
 *      void smlal(int32_t& rlo, int32_t& rhi, int32_t op1, int32_t op2) {
 *        int64_t mul = int64_t(op1) * op2;
 *        int64_t s = int64_t(uint32_t(rlo) | ((uint64_t(uint32_t(rhi)) << 32U)));
 *        mul += s;
 *        rlo = int32_t(mul & 0xFFFFFFFF);
 *        rhi = int32_t((mul >> 32) & 0xFFFFFFFF);
 *      }
 *      int32_t _eval_bezier_curve_arm(uint32_t curr_step) {
 *        uint32_t flo = 0;
 *        uint32_t fhi = bezier_AV * curr_step;
 *        uint32_t t = fhi;
 *        int32_t alo = bezier_F;
 *        int32_t ahi = 0;
 *        int32_t A = bezier_A;
 *        int32_t B = bezier_B;
 *        int32_t C = bezier_C;
 *
 *        lsrs(ahi, alo, 1);          // a  = F << 31
 *        lsls(alo, alo, 31);         //
 *        umull(flo, fhi, fhi, t);    // f *= t
 *        umull(flo, fhi, fhi, t);    // f>>=32; f*=t
 *        lsrs(flo, fhi, 1);          //
 *        smlal(alo, ahi, flo, C);    // a+=(f>>33)*C
 *        umull(flo, fhi, fhi, t);    // f>>=32; f*=t
 *        lsrs(flo, fhi, 1);          //
 *        smlal(alo, ahi, flo, B);    // a+=(f>>33)*B
 *        umull(flo, fhi, fhi, t);    // f>>=32; f*=t
 *        lsrs(flo, fhi, 1);          // f>>=33;
 *        smlal(alo, ahi, flo, A);    // a+=(f>>33)*A;
 *        lsrs(alo, ahi, 6);          // a>>=38
 *
 *        return alo;
 *      }
 *
 *  This is rewritten in ARM assembly for optimal performance (43 cycles to execute).
 *
 *  For AVR, the precision of coefficients is scaled so the Bézier curve can be evaluated in real-time:
 *  Let's reduce precision as much as possible. After some experimentation we found that:
 *
 *    Assume t and AV with 24 bits is enough
 *       A =  6*(VF - VI)
 *       B = 15*(VI - VF)
 *       C = 10*(VF - VI)
 *       F =     VI
 *      AV = (1<<24)/TS   (this is computed at the planner, to offload expensive calculations from the ISR)
 *
 *    Instead of storing sign for each coefficient, we will store its absolute value,
 *    and flag the sign of the A coefficient, so we can save to store the sign bit.
 *    It always holds that sign(A) = - sign(B) = sign(C)
 *
 *     So, the resulting range of the coefficients are:
 *
 *       t: unsigned (0 <= t < 1) |range 0 to 0xFFFFFF unsigned
 *       A:   signed Q24 , range = 250000 * 6 = 1500000 = 0x16E360 | 21 bits
 *       B:   signed Q24 , range = 250000 *15 = 3750000 = 0x393870 | 22 bits
 *       C:   signed Q24 , range = 250000 *10 = 2500000 = 0x1312D0 | 21 bits
 *       F:   signed Q24 , range = 250000     =  250000 = 0x0ED090 | 20 bits
 *
 *    And for each curve, estimate its coefficients with:
 *
 *      void _calc_bezier_curve_coeffs(int32_t v0, int32_t v1, uint32_t av) {
 *       // Calculate the Bézier coefficients
 *       if (v1 < v0) {
 *         A_negative = true;
 *         bezier_A = 6 * (v0 - v1);
 *         bezier_B = 15 * (v0 - v1);
 *         bezier_C = 10 * (v0 - v1);
 *       }
 *       else {
 *         A_negative = false;
 *         bezier_A = 6 * (v1 - v0);
 *         bezier_B = 15 * (v1 - v0);
 *         bezier_C = 10 * (v1 - v0);
 *       }
 *       bezier_F = v0;
 *      }
 *
 *    And for each point, evaluate the curve with the following sequence:
 *
 *      // unsigned multiplication of 24 bits x 24bits, return upper 16 bits
 *      void umul24x24to16hi(uint16_t& r, uint24_t op1, uint24_t op2) {
 *        r = (uint64_t(op1) * op2) >> 8;
 *      }
 *      // unsigned multiplication of 16 bits x 16bits, return upper 16 bits
 *      void umul16x16to16hi(uint16_t& r, uint16_t op1, uint16_t op2) {
 *        r = (uint32_t(op1) * op2) >> 16;
 *      }
 *      // unsigned multiplication of 16 bits x 24bits, return upper 24 bits
 *      void umul16x24to24hi(uint24_t& r, uint16_t op1, uint24_t op2) {
 *        r = uint24_t((uint64_t(op1) * op2) >> 16);
 *      }
 *
 *      int32_t _eval_bezier_curve(uint32_t curr_step) {
 *        // To save computing, the first step is always the initial speed
 *        if (!curr_step)
 *          return bezier_F;
 *
 *        uint16_t t;
 *        umul24x24to16hi(t, bezier_AV, curr_step);   // t: Range 0 - 1^16 = 16 bits
 *        uint16_t f = t;
 *        umul16x16to16hi(f, f, t);                   // Range 16 bits (unsigned)
 *        umul16x16to16hi(f, f, t);                   // Range 16 bits : f = t^3  (unsigned)
 *        uint24_t acc = bezier_F;                    // Range 20 bits (unsigned)
 *        if (A_negative) {
 *          uint24_t v;
 *          umul16x24to24hi(v, f, bezier_C);          // Range 21bits
 *          acc -= v;
 *          umul16x16to16hi(f, f, t);                 // Range 16 bits : f = t^4  (unsigned)
 *          umul16x24to24hi(v, f, bezier_B);          // Range 22bits
 *          acc += v;
 *          umul16x16to16hi(f, f, t);                 // Range 16 bits : f = t^5  (unsigned)
 *          umul16x24to24hi(v, f, bezier_A);          // Range 21bits + 15 = 36bits (plus sign)
 *          acc -= v;
 *        }
 *        else {
 *          uint24_t v;
 *          umul16x24to24hi(v, f, bezier_C);          // Range 21bits
 *          acc += v;
 *          umul16x16to16hi(f, f, t);                 // Range 16 bits : f = t^4  (unsigned)
 *          umul16x24to24hi(v, f, bezier_B);          // Range 22bits
 *          acc -= v;
 *          umul16x16to16hi(f, f, t);                 // Range 16 bits : f = t^5  (unsigned)
 *          umul16x24to24hi(v, f, bezier_A);          // Range 21bits + 15 = 36bits (plus sign)
 *          acc += v;
 *        }
 *        return acc;
 *      }
 *    These functions are translated to assembler for optimal performance.
 *    Coefficient calculation takes 70 cycles. Bezier point evaluation takes 150 cycles.
 */

// For all the other 32bit CPUs
FORCE_INLINE void Stepper::_calc_bezier_curve_coeffs(const int32_t v0, const int32_t v1, const uint32_t av) {
	// Calculate the Bézier coefficients
	bezier_A = 768 * (v1 - v0);
	bezier_B = 1920 * (v0 - v1);
	bezier_C = 1280 * (v1 - v0);
	bezier_F = 128 * v0;
	bezier_AV = av;
}

FORCE_INLINE int32_t Stepper::_eval_bezier_curve(const uint32_t curr_step) {
#	if defined(__arm__) || defined(__thumb__)

	// For ARM Cortex M3/M4 CPUs, we have the optimized assembler version, that takes 43 cycles to execute
	uint32_t flo = 0;
	uint32_t fhi = bezier_AV * curr_step;
	uint32_t t = fhi;
	int32_t alo = bezier_F;
	int32_t ahi = 0;
	int32_t A = bezier_A;
	int32_t B = bezier_B;
	int32_t C = bezier_C;

	__asm__ __volatile__(
			".syntax unified"
			"\n\t" // is to prevent CM0,CM1 non-unified syntax
			"lsrs  %[ahi],%[alo],#1\n\t" // a  = F << 31      1 cycles
			"lsls  %[alo],%[alo],#31\n\t" //                   1 cycles
			"umull %[flo],%[fhi],%[fhi],%[t]\n\t" // f *= t            5 cycles [fhi:flo=64bits]
			"umull %[flo],%[fhi],%[fhi],%[t]\n\t" // f>>=32; f*=t      5 cycles [fhi:flo=64bits]
			"lsrs  %[flo],%[fhi],#1\n\t" //                   1 cycles [31bits]
			"smlal %[alo],%[ahi],%[flo],%[C]\n\t" // a+=(f>>33)*C;     5 cycles
			"umull %[flo],%[fhi],%[fhi],%[t]\n\t" // f>>=32; f*=t      5 cycles [fhi:flo=64bits]
			"lsrs  %[flo],%[fhi],#1\n\t" //                   1 cycles [31bits]
			"smlal %[alo],%[ahi],%[flo],%[B]\n\t" // a+=(f>>33)*B;     5 cycles
			"umull %[flo],%[fhi],%[fhi],%[t]\n\t" // f>>=32; f*=t      5 cycles [fhi:flo=64bits]
			"lsrs  %[flo],%[fhi],#1\n\t" // f>>=33;           1 cycles [31bits]
			"smlal %[alo],%[ahi],%[flo],%[A]\n\t" // a+=(f>>33)*A;     5 cycles
			"lsrs  %[alo],%[ahi],#6\n\t" // a>>=38            1 cycles
			: [alo] "+r"(alo),
				[flo] "+r"(flo),
				[fhi] "+r"(fhi),
				[ahi] "+r"(ahi),
				[A] "+r"(A), // <== Note: Even if A, B, C, and t registers are INPUT ONLY
				[B] "+r"(B), //  GCC does bad optimizations on the code if we list them as
				[C] "+r"(C), //  such, breaking this function. So, to avoid that problem,
				[t] "+r"(t) //  we list all registers as input-outputs.
			:
			: "cc");

	return alo;

#	else

	// For non ARM targets, we provide a fallback implementation. Really doubt it
	// will be useful, unless the processor is fast and 32bit

	uint32_t t = bezier_AV * curr_step; // t: Range 0 - 1^32 = 32 bits
	uint64_t f = t;
	f *= t; // Range 32*2 = 64 bits (unsigned)
	f >>= 32; // Range 32 bits  (unsigned)
	f *= t; // Range 32*2 = 64 bits  (unsigned)
	f >>= 32; // Range 32 bits : f = t^3  (unsigned)
	int64_t acc = (int64_t) bezier_F << 31; // Range 63 bits (signed)
	acc += ((uint32_t) f >> 1) * (int64_t) bezier_C; // Range 29bits + 31 = 60bits (plus sign)
	f *= t; // Range 32*2 = 64 bits
	f >>= 32; // Range 32 bits : f = t^3  (unsigned)
	acc += ((uint32_t) f >> 1) * (int64_t) bezier_B; // Range 29bits + 31 = 60bits (plus sign)
	f *= t; // Range 32*2 = 64 bits
	f >>= 32; // Range 32 bits : f = t^3  (unsigned)
	acc += ((uint32_t) f >> 1) * (int64_t) bezier_A; // Range 28bits + 31 = 59bits (plus sign)
	acc >>= (31 + 7); // Range 24bits (plus sign)
	return (int32_t) acc;

#	endif
}
#endif // S_CURVE_ACCELERATION

/**
 * Stepper Driver Interrupt
 *
 * Directly pulses the stepper motors at high frequency.
 */

HAL_STEP_TIMER_ISR() {
	HAL_timer_isr_prologue(STEP_TIMER_NUM);

	Stepper::isr();

	HAL_timer_isr_epilogue(STEP_TIMER_NUM);
}

#ifdef CPU_32_BIT
#	define STEP_MULTIPLY(A, B) MultiU32X24toH32(A, B)
#else
#	define STEP_MULTIPLY(A, B) MultiU24X32toH16(A, B)
#endif

void Stepper::isr() {

	static uint32_t nextMainISR = 0; // Interval until the next main Stepper Pulse phase (0 = Now)

	// Program timer compare for the maximum period, so it does NOT
	// flag an interrupt while this ISR is running - So changes from small
	// periods to big periods are respected and the timer does not reset to 0
	HAL_timer_set_compare(STEP_TIMER_NUM, hal_timer_t(HAL_TIMER_TYPE_MAX));

	// Count of ticks for the next ISR
	hal_timer_t next_isr_ticks = 0;

	// Limit the amount of iterations
	uint8_t max_loops = 10;

	// We need this variable here to be able to use it in the following loop
	hal_timer_t min_ticks;
	do {
		// Enable ISRs to reduce USART processing latency
		ENABLE_ISRS();

		if (!nextMainISR)
			pulse_phase_isr(); // 0 = Do coordinated axes Stepper pulses

		// ^== Time critical. NOTHING besides pulse generation should be above here!!!

		if (!nextMainISR)
			nextMainISR = block_phase_isr(); // Manage acc/deceleration, get next block

		// Get the interval to the next ISR call
		const uint32_t interval = _MIN(
				nextMainISR, // Time until the next Pulse / Block phase
				uint32_t(HAL_TIMER_TYPE_MAX) // Come back in a very long time
		);

		//
		// Compute remaining time for each ISR phase
		//     NEVER : The phase is idle
		//      Zero : The phase will occur on the next ISR call
		//  Non-zero : The phase will occur on a future ISR call
		//

		nextMainISR -= interval;

		/**
		 * This needs to avoid a race-condition caused by interleaving
		 * of interrupts required by both the LA and Stepper algorithms.
		 *
		 * Assume the following tick times for stepper pulses:
		 *   Stepper ISR (S):  1 1000 2000 3000 4000
		 *   Linear Adv. (E): 10 1010 2010 3010 4010
		 *
		 * The current algorithm tries to interleave them, giving:
		 *  1:S 10:E 1000:S 1010:E 2000:S 2010:E 3000:S 3010:E 4000:S 4010:E
		 *
		 * Ideal timing would yield these delta periods:
		 *  1:S  9:E  990:S   10:E  990:S   10:E  990:S   10:E  990:S   10:E
		 *
		 * But, since each event must fire an ISR with a minimum duration, the
		 * minimum delta might be 900, so deltas under 900 get rounded up:
		 *  900:S d900:E d990:S d900:E d990:S d900:E d990:S d900:E d990:S d900:E
		 *
		 * It works, but divides the speed of all motors by half, leading to a sudden
		 * reduction to 1/2 speed! Such jumps in speed lead to lost steps (not even
		 * accounting for double/quad stepping, which makes it even worse).
		 */

		// Compute the tick count for the next ISR
		next_isr_ticks += interval;

		/**
		 * The following section must be done with global interrupts disabled.
		 * We want nothing to interrupt it, as that could mess the calculations
		 * we do for the next value to program in the period register of the
		 * stepper timer and lead to skipped ISRs (if the value we happen to program
		 * is less than the current count due to something preempting between the
		 * read and the write of the new period value).
		 */
		DISABLE_ISRS();

		/**
		 * Get the current tick value + margin
		 * Assuming at least 6µs between calls to this ISR...
		 * On AVR the ISR epilogue+prologue is estimated at 100 instructions - Give 8µs as margin
		 * On ARM the ISR epilogue+prologue is estimated at 20 instructions - Give 1µs as margin
		 */
		min_ticks = HAL_timer_get_count(STEP_TIMER_NUM) + hal_timer_t(1 * (STEPPER_TIMER_TICKS_PER_US));

		/**
		 * NB: If for some reason the stepper monopolizes the MPU, eventually the
		 * timer will wrap around (and so will 'next_isr_ticks'). So, limit the
		 * loop to 10 iterations. Beyond that, there's no way to ensure correct pulse
		 * timing, since the MCU isn't fast enough.
		 */
		if (!--max_loops)
			next_isr_ticks = min_ticks;

		// Advance pulses if not enough time to wait for the next ISR
	} while (next_isr_ticks < min_ticks);

	// Now 'next_isr_ticks' contains the period to the next Stepper ISR - And we are
	// sure that the time has not arrived yet - Warrantied by the scheduler

	// Set the next ISR to fire at the proper time
	HAL_timer_set_compare(STEP_TIMER_NUM, hal_timer_t(next_isr_ticks));

	// Don't forget to finally reenable interrupts
	ENABLE_ISRS();
}

#if MINIMUM_STEPPER_PULSE || MAXIMUM_STEPPER_RATE
#	define ISR_PULSE_CONTROL 1
#endif
#if ISR_PULSE_CONTROL && DISABLED(I2S_STEPPER_STREAM)
#	define ISR_MULTI_STEPS 1
#endif

/**
 * This phase of the ISR should ONLY create the pulses for the steppers.
 * This prevents jitter caused by the interval between the start of the
 * interrupt and the start of the pulses. DON'T add any logic ahead of the
 * call to this method that might cause variation in the timing. The aim
 * is to keep pulse timing as regular as possible.
 */
void Stepper::pulse_phase_isr() {
	// Count of pending loops and events for this iteration
	const uint32_t pending_events = step_event_count - step_events_completed;
	uint8_t events_to_do = _MIN(pending_events, steps_per_isr);

	// Just update the value we will get at the end of the loop
	step_events_completed += events_to_do;

// Take multiple steps per interrupt (For high speed moves)
#if ISR_MULTI_STEPS
	bool firstStep = true;
	USING_TIMED_PULSE();
#endif
	xyza_bool_t step_needed { 0, 0, 0, 0 };

	do {
		// If we must abort the current block, do so!
		if (abort_current_block) {
			abort_current_block = false;
			if (current_block)
				discard_current_block();
		}

		// If there is no current block, do nothing
		if (!current_block)
			return;

#define _APPLY_STEP(AXIS, INV, ALWAYS) AXIS##_APPLY_STEP(INV, ALWAYS)
#define _INVERT_STEP_PIN(AXIS)         INVERT_##AXIS##_STEP_PIN

// Determine if a pulse is needed using Bresenham
#define PULSE_PREP(AXIS) \
	do { \
		delta_error[_AXIS(AXIS)] += advance_dividend[_AXIS(AXIS)]; \
		step_needed[_AXIS(AXIS)] = (delta_error[_AXIS(AXIS)] >= 0); \
		if (step_needed[_AXIS(AXIS)]) { \
			count_position[_AXIS(AXIS)] += count_direction[_AXIS(AXIS)]; \
			delta_error[_AXIS(AXIS)] -= advance_divisor; \
		} \
	} while (0)

// Start an active pulse if needed
#define PULSE_START(AXIS) \
	do { \
		if (step_needed[_AXIS(AXIS)]) { \
			_APPLY_STEP(AXIS, !_INVERT_STEP_PIN(AXIS), 0); \
		} \
	} while (0)

// Stop an active pulse if needed
#define PULSE_STOP(AXIS) \
	do { \
		if (step_needed[_AXIS(AXIS)]) { \
			_APPLY_STEP(AXIS, _INVERT_STEP_PIN(AXIS), 0); \
		} \
	} while (0)

		// Direct Stepping page?
		const bool is_page = IS_PAGE(current_block);

		if (!is_page) {
// Determine if pulses are needed
#if HAS_X_STEP
			PULSE_PREP(X);
#endif
#if HAS_Y_STEP
			PULSE_PREP(Y);
#endif
#if HAS_Z_STEP
			PULSE_PREP(Z);
#endif
#if HAS_A_STEP
			PULSE_PREP(A);
#endif
		}

#if ISR_MULTI_STEPS
		if (firstStep)
			firstStep = false;
		else
			AWAIT_LOW_PULSE();
#endif

// Pulse start
#if HAS_X_STEP
		PULSE_START(X);
#endif
#if HAS_Y_STEP
		PULSE_START(Y);
#endif
#if HAS_Z_STEP
		PULSE_START(Z);
#endif
#if HAS_A_STEP
		PULSE_START(A);
#endif

#if ENABLED(I2S_STEPPER_STREAM)
		i2s_push_sample();
#endif

// TODO: need to deal with MINIMUM_STEPPER_PULSE over i2s
#if ISR_MULTI_STEPS
		START_HIGH_PULSE();
		AWAIT_HIGH_PULSE();
#endif

// Pulse stop
#if HAS_X_STEP
		PULSE_STOP(X);
#endif
#if HAS_Y_STEP
		PULSE_STOP(Y);
#endif
#if HAS_Z_STEP
		PULSE_STOP(Z);
#endif
#if HAS_A_STEP
		PULSE_STOP(A);
#endif

#if ISR_MULTI_STEPS
		if (events_to_do)
			START_LOW_PULSE();
#endif

	} while (--events_to_do);
}

// This is the last half of the stepper interrupt: This one processes and
// properly schedules blocks from the planner. This is executed after creating
// the step pulses, so it is not time critical, as pulses are already done.

uint32_t Stepper::block_phase_isr() {
	// If no queued movements, just wait 1ms for the next block
	uint32_t interval = (STEPPER_TIMER_RATE) / 1000UL;

	// If there is a current block
	if (current_block) {

		// If current block is finished, reset pointer and finalize state
		if (step_events_completed >= step_event_count) {
			TERN_(HAS_FILAMENT_RUNOUT_DISTANCE, runout.block_completed(current_block));
			discard_current_block();
		} else {
			// Step events not completed yet...

			// Are we in acceleration phase ?
			if (step_events_completed <= accelerate_until) { // Calculate new timer value

#if ENABLED(S_CURVE_ACCELERATION)
				// Get the next speed to use (Jerk limited!)
				uint32_t acc_step_rate = acceleration_time < current_block->acceleration_time
				                             ? _eval_bezier_curve(acceleration_time)
				                             : current_block->cruise_rate;
#else
				acc_step_rate = STEP_MULTIPLY(acceleration_time, current_block->acceleration_rate) + current_block->initial_rate;
				NOMORE(acc_step_rate, current_block->nominal_rate);
#endif

				// acc_step_rate is in steps/second

				// step_rate to timer interval and steps per stepper isr
				interval = calc_timer_interval(acc_step_rate, &steps_per_isr);
				acceleration_time += interval;

// Update laser - Accelerating
#if ENABLED(LASER_POWER_INLINE_TRAPEZOID)
				if (laser_trap.enabled) {
#	if DISABLED(LASER_POWER_INLINE_TRAPEZOID_CONT)
					if (current_block->laser.entry_per) {
						laser_trap.acc_step_count -= step_events_completed - laser_trap.last_step_count;
						laser_trap.last_step_count = step_events_completed;

						// Should be faster than a divide, since this should trip just once
						if (laser_trap.acc_step_count < 0) {
							while (laser_trap.acc_step_count < 0) {
								laser_trap.acc_step_count += current_block->laser.entry_per;
								if (laser_trap.cur_power < current_block->laser.power)
									laser_trap.cur_power++;
							}
							cutter.set_ocr_power(laser_trap.cur_power);
						}
					}
#	else
					if (laser_trap.till_update)
						laser_trap.till_update--;
					else {
						laser_trap.till_update = LASER_POWER_INLINE_TRAPEZOID_CONT_PER;
						laser_trap.cur_power = (current_block->laser.power * acc_step_rate) / current_block->nominal_rate;
						cutter.set_ocr_power(laser_trap.cur_power); // Cycle efficiency is irrelevant it the last line was many cycles
					}
#	endif
				}
#endif
			}
			// Are we in Deceleration phase ?
			else if (step_events_completed > decelerate_after) {
				uint32_t step_rate;

#if ENABLED(S_CURVE_ACCELERATION)
				// If this is the 1st time we process the 2nd half of the trapezoid...
				if (!bezier_2nd_half) {
					// Initialize the Bézier speed curve
					_calc_bezier_curve_coeffs(current_block->cruise_rate, current_block->final_rate, current_block->deceleration_time_inverse);
					bezier_2nd_half = true;
					// The first point starts at cruise rate. Just save evaluation of the Bézier curve
					step_rate = current_block->cruise_rate;
				} else {
					// Calculate the next speed to use
					step_rate = deceleration_time < current_block->deceleration_time
					                ? _eval_bezier_curve(deceleration_time)
					                : current_block->final_rate;
				}
#else

				// Using the old trapezoidal control
				step_rate = STEP_MULTIPLY(deceleration_time, current_block->acceleration_rate);
				if (step_rate < acc_step_rate) { // Still decelerating?
					step_rate = acc_step_rate - step_rate;
					NOLESS(step_rate, current_block->final_rate);
				} else
					step_rate = current_block->final_rate;
#endif

				// step_rate is in steps/second

				// step_rate to timer interval and steps per stepper isr
				interval = calc_timer_interval(step_rate, &steps_per_isr);
				deceleration_time += interval;

// Update laser - Decelerating
#if ENABLED(LASER_POWER_INLINE_TRAPEZOID)
				if (laser_trap.enabled) {
#	if DISABLED(LASER_POWER_INLINE_TRAPEZOID_CONT)
					if (current_block->laser.exit_per) {
						laser_trap.acc_step_count -= step_events_completed - laser_trap.last_step_count;
						laser_trap.last_step_count = step_events_completed;

						// Should be faster than a divide, since this should trip just once
						if (laser_trap.acc_step_count < 0) {
							while (laser_trap.acc_step_count < 0) {
								laser_trap.acc_step_count += current_block->laser.exit_per;
								if (laser_trap.cur_power > current_block->laser.power_exit)
									laser_trap.cur_power--;
							}
							cutter.set_ocr_power(laser_trap.cur_power);
						}
					}
#	else
					if (laser_trap.till_update)
						laser_trap.till_update--;
					else {
						laser_trap.till_update = LASER_POWER_INLINE_TRAPEZOID_CONT_PER;
						laser_trap.cur_power = (current_block->laser.power * step_rate) / current_block->nominal_rate;
						cutter.set_ocr_power(laser_trap.cur_power); // Cycle efficiency isn't relevant when the last line was many cycles
					}
#	endif
				}
#endif
			}
			// Must be in cruise phase otherwise
			else {

				// Calculate the ticks_nominal for this nominal speed, if not done yet
				if (ticks_nominal < 0) {
					// step_rate to timer interval and loops for the nominal speed
					ticks_nominal = calc_timer_interval(current_block->nominal_rate, &steps_per_isr);
				}

				// The timer interval is just the nominal value for the nominal speed
				interval = ticks_nominal;

// Update laser - Cruising
#if ENABLED(LASER_POWER_INLINE_TRAPEZOID)
				if (laser_trap.enabled) {
					if (!laser_trap.cruise_set) {
						laser_trap.cur_power = current_block->laser.power;
						cutter.set_ocr_power(laser_trap.cur_power);
						laser_trap.cruise_set = true;
					}
#	if ENABLED(LASER_POWER_INLINE_TRAPEZOID_CONT)
					laser_trap.till_update = LASER_POWER_INLINE_TRAPEZOID_CONT_PER;
#	else
					laser_trap.last_step_count = step_events_completed;
#	endif
				}
#endif
			}
		}
	}

	// If there is no current block at this point, attempt to pop one from the buffer
	// and prepare its movement
	if (!current_block) {

		// Anything in the buffer?
		if ((current_block = planner.get_current_block())) {

			// Sync block? Sync the stepper counts and return
			while (TEST(current_block->flag, BLOCK_BIT_SYNC_POSITION)) {
				_set_position(current_block->position);
				discard_current_block();

				// Try to get a new block
				if (!(current_block = planner.get_current_block()))
					return interval; // No more queued movements!
			}

// For non-inline cutter, grossly apply power
#if ENABLED(LASER_FEATURE) && DISABLED(LASER_POWER_INLINE)
			cutter.apply_power(current_block->cutter_power);
#endif

			TERN_(POWER_LOSS_RECOVERY, recovery.info.sdpos = current_block->sdpos);

			// Flag all moving axes for proper endstop handling

#define X_MOVE_TEST !!current_block->steps.x
#define Y_MOVE_TEST !!current_block->steps.y
#define Z_MOVE_TEST !!current_block->steps.z

			// Based on the oversampling factor, do the calculations
			step_event_count = current_block->step_event_count;

			// Initialize Bresenham delta errors to 1/2
			delta_error = -int32_t(step_event_count);

			// Calculate Bresenham dividends and divisors
			advance_dividend = current_block->steps << 1;
			advance_divisor = step_event_count << 1;

			// No step events completed so far
			step_events_completed = 0;

			// Compute the acceleration and deceleration points
			accelerate_until = current_block->accelerate_until;
			decelerate_after = current_block->decelerate_after;

			if (ENABLED(HAS_L64XX) // Always set direction for L64xx (Also enables the chips)
			    || ENABLED(DUAL_X_CARRIAGE) // TODO: Find out why this fixes "jittery" small circles
			    || current_block->direction_bits != last_direction_bits || TERN(MIXING_EXTRUDER, false, stepper_extruder != last_moved_extruder)) {
				TERN_(HAS_MULTI_EXTRUDER, last_moved_extruder = stepper_extruder);
				TERN_(HAS_L64XX, L64XX_OK_to_power_up = true);
				set_directions(current_block->direction_bits);
			}

#if ENABLED(LASER_POWER_INLINE)
			const power_status_t stat = current_block->laser.status;
#	if ENABLED(LASER_POWER_INLINE_TRAPEZOID)
			laser_trap.enabled = stat.isPlanned && stat.isEnabled;
			laser_trap.cur_power = current_block->laser.power_entry; // RESET STATE
			laser_trap.cruise_set = false;
#		if DISABLED(LASER_POWER_INLINE_TRAPEZOID_CONT)
			laser_trap.last_step_count = 0;
			laser_trap.acc_step_count = current_block->laser.entry_per / 2;
#		else
			laser_trap.till_update = 0;
#		endif
			// Always have PWM in this case
			if (stat.isPlanned) { // Planner controls the laser
				cutter.set_ocr_power(
						stat.isEnabled ? laser_trap.cur_power : 0 // ON with power or OFF
				);
			}
#	else
			if (stat.isPlanned) { // Planner controls the laser
#		if ENABLED(SPINDLE_LASER_PWM)
				cutter.set_ocr_power(
						stat.isEnabled ? current_block->laser.power : 0 // ON with power or OFF
				);
#		else
				cutter.enabled(stat.isEnabled);
#		endif
			}
#	endif
#endif // LASER_POWER_INLINE

			// At this point, we must ensure the movement about to execute isn't
			// trying to force the head against a limit switch. If using interrupt-
			// driven change detection, and already against a limit then no call to
			// the endstop_triggered method will be done and the movement will be
			// done against the endstop. So, check the limits here: If the movement
			// is against the limits, the block will be marked as to be killed, and
			// on the next call to this ISR, will be discarded.
			endstops.update();

			// Mark the time_nominal as not calculated yet
			ticks_nominal = -1;

#if ENABLED(S_CURVE_ACCELERATION)
			// Initialize the Bézier speed curve
			_calc_bezier_curve_coeffs(current_block->initial_rate, current_block->cruise_rate, current_block->acceleration_time_inverse);
			// We haven't started the 2nd half of the trapezoid
			bezier_2nd_half = false;
#else
			// Set as deceleration point the initial rate of the block
			acc_step_rate = current_block->initial_rate;
#endif

			// Calculate the initial timer interval
			interval = calc_timer_interval(current_block->initial_rate, &steps_per_isr);
		}
#if ENABLED(LASER_POWER_INLINE_CONTINUOUS)
		else { // No new block found; so apply inline laser parameters
			// This should mean ending file with 'M5 I' will stop the laser; thus the inline flag isn't needed
			const power_status_t stat = planner.laser_inline.status;
			if (stat.isPlanned) { // Planner controls the laser
#	if ENABLED(SPINDLE_LASER_PWM)
				cutter.set_ocr_power(
						stat.isEnabled ? planner.laser_inline.power : 0 // ON with power or OFF
				);
#	else
				cutter.enabled(stat.isEnabled);
#	endif
			}
		}
#endif
	}

	// Return the interval to wait
	return interval;
}

// Check if the given block is busy or not - Must not be called from ISR contexts
// The current_block could change in the middle of the read by an Stepper ISR, so
// we must explicitly prevent that!
bool Stepper::is_block_busy(const block_t* const block) {
	block_t* vnew = current_block;
	// Return if the block is busy or not
	return block == vnew;
}

void Stepper::init() {
	// Init Dir Pins
	TERN_(HAS_X_DIR, X_DIR_INIT());
	TERN_(HAS_X2_DIR, X2_DIR_INIT());
#if HAS_Y_DIR
	Y_DIR_INIT();
#	if BOTH(Y_DUAL_STEPPER_DRIVERS, HAS_Y2_DIR)
	Y2_DIR_INIT();
#	endif
#endif
#if HAS_Z_DIR
	Z_DIR_INIT();
#	if NUM_Z_STEPPER_DRIVERS >= 2 && HAS_Z2_DIR
	Z2_DIR_INIT();
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 3 && HAS_Z3_DIR
	Z3_DIR_INIT();
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 4 && HAS_Z4_DIR
	Z4_DIR_INIT();
#	endif
#endif
#if HAS_A_DIR
	A_DIR_INIT();
#endif
#if HAS_A2_DIR
	A2_DIR_INIT();
#endif

// Init Enable Pins - steppers default to disabled.
#if HAS_X_ENABLE
	X_ENABLE_INIT();
	if (!X_ENABLE_ON)
		X_ENABLE_WRITE(HIGH);
#	if EITHER(DUAL_X_CARRIAGE, X_DUAL_STEPPER_DRIVERS) && HAS_X2_ENABLE
	X2_ENABLE_INIT();
	if (!X_ENABLE_ON)
		X2_ENABLE_WRITE(HIGH);
#	endif
#endif
#if HAS_Y_ENABLE
	Y_ENABLE_INIT();
	if (!Y_ENABLE_ON)
		Y_ENABLE_WRITE(HIGH);
#	if BOTH(Y_DUAL_STEPPER_DRIVERS, HAS_Y2_ENABLE)
	Y2_ENABLE_INIT();
	if (!Y_ENABLE_ON)
		Y2_ENABLE_WRITE(HIGH);
#	endif
#endif
#if HAS_Z_ENABLE
	Z_ENABLE_INIT();
	if (!Z_ENABLE_ON)
		Z_ENABLE_WRITE(HIGH);
#	if NUM_Z_STEPPER_DRIVERS >= 2 && HAS_Z2_ENABLE
	Z2_ENABLE_INIT();
	if (!Z_ENABLE_ON)
		Z2_ENABLE_WRITE(HIGH);
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 3 && HAS_Z3_ENABLE
	Z3_ENABLE_INIT();
	if (!Z_ENABLE_ON)
		Z3_ENABLE_WRITE(HIGH);
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 4 && HAS_Z4_ENABLE
	Z4_ENABLE_INIT();
	if (!Z_ENABLE_ON)
		Z4_ENABLE_WRITE(HIGH);
#	endif
#endif
#if HAS_Z_BRAKE
	Z_BRAKE_INIT();
	BRAKE_Z();
#endif
#if HAS_A_ENABLE
	A_ENABLE_INIT();
	if (!A_ENABLE_ON)
		A_ENABLE_WRITE(HIGH);
#endif
#if HAS_A2_ENABLE
	A2_ENABLE_INIT();
	if (!A_ENABLE_ON)
		A2_ENABLE_WRITE(HIGH);
#endif

#define _STEP_INIT(AXIS)           AXIS##_STEP_INIT()
#define _WRITE_STEP(AXIS, HIGHLOW) AXIS##_STEP_WRITE(HIGHLOW)
#define _DISABLE_AXIS(AXIS)        DISABLE_AXIS_##AXIS()

#define AXIS_INIT(AXIS, PIN) \
	_STEP_INIT(AXIS); \
	_WRITE_STEP(AXIS, _INVERT_STEP_PIN(PIN)); \
	_DISABLE_AXIS(AXIS)

// Init Step Pins
#if HAS_X_STEP
#	if EITHER(X_DUAL_STEPPER_DRIVERS, DUAL_X_CARRIAGE)
	X2_STEP_INIT();
	X2_STEP_WRITE(INVERT_X_STEP_PIN);
#	endif
	AXIS_INIT(X, X);
#endif

#if HAS_Y_STEP
#	if ENABLED(Y_DUAL_STEPPER_DRIVERS)
	Y2_STEP_INIT();
	Y2_STEP_WRITE(INVERT_Y_STEP_PIN);
#	endif
	AXIS_INIT(Y, Y);
#endif

#if HAS_Z_STEP
#	if NUM_Z_STEPPER_DRIVERS >= 2
	Z2_STEP_INIT();
	Z2_STEP_WRITE(INVERT_Z_STEP_PIN);
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 3
	Z3_STEP_INIT();
	Z3_STEP_WRITE(INVERT_Z_STEP_PIN);
#	endif
#	if NUM_Z_STEPPER_DRIVERS >= 4
	Z4_STEP_INIT();
	Z4_STEP_WRITE(INVERT_Z_STEP_PIN);
#	endif
	AXIS_INIT(Z, Z);
#endif

#if HAS_A_STEP
#	if ENABLED(A_DUAL_STEPPER_DRIVERS)
	A2_STEP_INIT();
	A2_STEP_WRITE(INVERT_A_STEP_PIN);
#	endif
	AXIS_INIT(A, A);
#endif

#if DISABLED(I2S_STEPPER_STREAM)
	HAL_timer_start(STEP_TIMER_NUM, 122); // Init Stepper ISR to 122 Hz for quick starting
	wake_up();
	sei();
#endif

	// Init direction bits for first moves
	set_directions((INVERT_X_DIR ? _BV(X_AXIS) : 0) | (INVERT_Y_DIR ? _BV(Y_AXIS) : 0) | (INVERT_Z_DIR ? _BV(Z_AXIS) : 0));
}

/**
 * Set the stepper positions directly in steps
 *
 * The input is based on the typical per-axis XYZ steps.
 * For CORE machines XYZ needs to be translated to ABC.
 *
 * This allows get_axis_position_mm to correctly
 * derive the current XYZ position later on.
 */
void Stepper::_set_position(const int32_t& x, const int32_t& y, const int32_t& z, const int32_t& a) {
	count_position.set(x, y, z, a);
}

/**
 * Get a stepper's position in steps.
 */
int32_t Stepper::position(const AxisEnum axis) {
	return count_position[axis];
}

// Set the current position in steps
void Stepper::set_position(const int32_t& x, const int32_t& y, const int32_t& z, const int32_t& a) {
	planner.synchronize();
	const bool was_enabled = suspend();
	_set_position(x, y, z, a);
	if (was_enabled)
		wake_up();
}

void Stepper::set_axis_position(const AxisEnum a, const int32_t& v) {
	planner.synchronize();

	count_position[a] = v;
}

// Signal endstops were triggered - This function can be called from
// an ISR context  (Temperature, Stepper or limits ISR), so we must
// be very careful here. If the interrupt being preempted was the
// Stepper ISR (this CAN happen with the endstop limits ISR) then
// when the stepper ISR resumes, we must be very sure that the movement
// is properly canceled
void Stepper::endstop_triggered(const AxisEnum axis) {

	const bool was_enabled = suspend();
	endstops_trigsteps[axis] = count_position[axis];

	// Discard the rest of the move if there is a current block
	quick_stop();

	if (was_enabled)
		wake_up();
}

int32_t Stepper::triggered_position(const AxisEnum axis) {
	return endstops_trigsteps[axis];
}

void Stepper::report_a_position(const xyz_long_t& pos) {
	SERIAL_ECHOLNPAIR_P(SP_Z_LBL, pos.z);
}

void Stepper::report_positions() {
	report_a_position(count_position);
}