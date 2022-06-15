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
 * temperature.cpp - temperature control
 */

// Useful when debugging thermocouples
//#define IGNORE_THERMOCOUPLE_ERRORS

#include "temperature.h"
#include "endstops.h"

#include "../MarlinCore.h"
#include "planner.h"
#include "../HAL/shared/Delay.h"

#include "printcounter.h"

Temperature thermalManager;

/**
 * Initialize the temperature manager
 * The manager is implemented by periodic calls to manage_heater()
 */
void Temperature::init() {
	HAL_adc_init();

	HAL_timer_start(TEMP_TIMER_NUM, TEMP_TIMER_FREQUENCY);
	ENABLE_TEMPERATURE_INTERRUPT();
}

//
// Temperature Error Handlers
//

inline void loud_kill(PGM_P const lcd_msg, const heater_id_t heater_id) {
	marlin_state = MF_KILLED;
	kill(lcd_msg);
}

/**
 * Manage heating activities for extruder hot-ends and a heated bed
 *  - Acquire updated temperature readings
 *    - Also resets the watchdog timer
 *  - Invoke thermal runaway protection
 *  - Manage extruder auto-fan
 *  - Apply filament width to the extrusion rate (may move)
 *  - Update the heated bed PID output value
 */
void Temperature::manage_heater() {

#if EARLY_WATCHDOG
	// If thermal manager is still not running, make sure to at least reset the watchdog!
	if (!inited)
		return watchdog_refresh();
#endif

#if ENABLED(EMERGENCY_PARSER)
	if (emergency_parser.killed_by_M112)
		kill(M112_KILL_STR, nullptr, true);

	if (emergency_parser.quickstop_by_M410) {
		emergency_parser.quickstop_by_M410 = false; // quickstop_stepper may call idle so clear this now!
		quickstop_stepper();
	}
#endif

	watchdog_refresh();
}

/**
 * Timer 0 is shared with millies so don't change the prescaler.
 *
 * On AVR this ISR uses the compare method so it runs at the base
 * frequency (16 MHz / 64 / 256 = 976.5625 Hz), but at the TCNT0 set
 * in OCR0B above (128 or halfway between OVFs).
 *
 *  - Manage PWM to all the heaters and fan
 *  - Prepare or Measure one of the raw ADC sensor values
 *  - Check new temperature values for MIN/MAX errors (kill on error)
 *  - Step the babysteps value for each axis towards 0
 *  - For PINS_DEBUGGING, monitor and report endstop pins
 *  - For ENDSTOP_INTERRUPTS_FEATURE check endstops if flagged
 *  - Call planner.tick to count down its "ignore" time
 */
HAL_TEMP_TIMER_ISR() {
	HAL_timer_isr_prologue(TEMP_TIMER_NUM);

	Temperature::tick();

	HAL_timer_isr_epilogue(TEMP_TIMER_NUM);
}

/**
 * Handle various ~1KHz tasks associated with temperature
 *  - Heater PWM (~1KHz with scaler)
 *  - LCD Button polling (~500Hz)
 *  - Start / Read one ADC sensor
 *  - Advance Babysteps
 *  - Endstop polling
 *  - Planner clean buffer
 */
void Temperature::tick() {
	// Poll endstops state, if required
	endstops.poll();

	// Periodically call the planner timer
	planner.tick();
}
