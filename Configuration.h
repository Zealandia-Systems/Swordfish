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

// #define DEBUG_GCODE_PARSER

/**
 * Configuration.h
 *
 * Basic settings such as:
 *
 * - Type of electronics
 * - Type of temperature sensor
 * - Printer geometry
 * - Endstop configuration
 * - LCD controller
 * - Extra features
 *
 * Advanced settings can be found in Configuration_adv.h
 */
#define CONFIGURATION_H_VERSION 020008

//===========================================================================
//============================= Getting Started =============================
//===========================================================================

/**
 * Here are some standard links for getting your machine calibrated:
 *
 * https://reprap.org/wiki/Calibration
 * https://youtu.be/wAL9d7FgInk
 * http://calculator.josefprusa.cz
 * https://reprap.org/wiki/Triffid_Hunter%27s_Calibration_Guide
 * https://www.thingiverse.com/thing:5573
 * https://sites.google.com/site/repraplogphase/calibration-of-your-reprap
 * https://www.thingiverse.com/thing:298812
 */

// @section info

// Author info of this build printed to the host during boot and M115
#define STRING_CONFIG_H_AUTHOR  "(none, default config)" // Who made the changes.
// #define CUSTOM_VERSION_FILE Version.h // Path from the root directory (no quotes)

/**
 * *** VENDORS PLEASE READ ***
 *
 * Marlin allows you to add a custom boot image for Graphical LCDs.
 * With this option Marlin will first show your custom screen followed
 * by the standard Marlin logo with version number and web URL.
 *
 * We encourage you to take advantage of this new feature and we also
 * respectfully request that you retain the unmodified Marlin boot screen.
 */

// Show the Marlin bootscreen on startup. ** ENABLE FOR PRODUCTION **
// #define SHOW_BOOTSCREEN

// Show the bitmap in Marlin/_Bootscreen.h on startup.
// #define SHOW_CUSTOM_BOOTSCREEN

// Show the bitmap in Marlin/_Statusscreen.h on the status screen.
// #define CUSTOM_STATUS_SCREEN_IMAGE

// @section machine

/**
 * Select the serial port on the board to use for communication with the host.
 * This allows the connection of wireless adapters (for instance) to non-default port pins.
 * Serial port -1 is the USB emulated serial port, if available.
 * Note: The first serial port (-1 or 0) will always be used by the Arduino bootloader.
 *
 * :[-1, 0, 1, 2, 3, 4, 5, 6, 7]
 */
#define SERIAL_PORT             -1

/**
 * Select a secondary serial port on the board to use for communication with the host.
 * Currently Ethernet (-2) is only supported on Teensy 4.1 boards.
 * :[-2, -1, 0, 1, 2, 3, 4, 5, 6, 7]
 */

// #define SERIAL_PORT_2 -3

/**
 * This setting determines the communication speed of the printer.
 *
 * 250000 works in most cases, but you might try a lower speed if
 * you commonly experience drop-outs during host printing.
 * You may try up to 1000000 to speed up SD file transfer.
 *
 * :[2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]
 */
#define BAUDRATE                1000000

// Enable the Bluetooth serial interface on AT90USB devices
// #define BLUETOOTH

// Choose the name from boards.h that matches your setup
#ifndef MOTHERBOARD
#	define MOTHERBOARD BOARD_AGCM4_BOARDINATOR
#endif

// Name displayed in the LCD "Ready" message and Info menu
#define CUSTOM_MACHINE_NAME ""

// Printer's unique ID, used by some programs to differentiate between machines.
// Choose your own or use a service like https://www.uuidgenerator.net/version4
// #define MACHINE_UUID "00000000-0000-0000-0000-000000000000"

#define HAS_A_AXIS          true

// @section extruder

// This defines the number of extruders
// :[0, 1, 2, 3, 4, 5, 6, 7, 8]
#define EXTRUDERS           0

#define HAS_TOOL_CHANGE     1

/**
 * Power Supply Control
 *
 * Enable and connect the power supply to the PS_ON_PIN.
 * Specify whether the power supply is active HIGH or active LOW.
 */
#define PSU_CONTROL
#define PSU_NAME "Power Supply"

#if ENABLED(PSU_CONTROL)
#	define PSU_ACTIVE_STATE  HIGH // Set 'LOW' for ATX, 'HIGH' for X-Box

// #define PSU_DEFAULT_OFF         // Keep power off until enabled directly with M80
#	define PSU_POWERUP_DELAY 250 // (ms) Delay for the PSU to warm up to full power

// #define PSU_POWERUP_GCODE  "M355 S1"  // G-code to run after power-on (e.g., case light on)
// #define PSU_POWEROFF_GCODE "M355 S0"  // G-code to run before power-off (e.g., case light off)

// #define AUTO_POWER_CONTROL      // Enable automatic control of the PS_ON pin
#	if ENABLED(AUTO_POWER_CONTROL)
// #define AUTO_POWER_FANS         // Turn on PSU if fans need power
// #define AUTO_POWER_E_FANS
// #define AUTO_POWER_CONTROLLERFAN
// #define AUTO_POWER_CHAMBER_FAN
// #define AUTO_POWER_E_TEMP        50 // (°C) Turn on PSU if any extruder is over this temperature
// #define AUTO_POWER_CHAMBER_TEMP  30 // (°C) Turn on PSU if the chamber is over this temperature
// #define POWER_TIMEOUT              30 // (s) Turn off power if the machine is idle for this duration
// #define POWER_OFF_DELAY          60 // (s) Delay of poweroff after M81 command. Useful to let fans run for extra time.
#	endif
#endif

//===========================================================================
//============================= Thermal Settings ============================
//===========================================================================
// @section temperature

/**
 * --NORMAL IS 4.7kohm PULLUP!-- 1kohm pullup can be used on hotend sensor, using correct resistor and table
 *
 * Temperature sensors available:
 *
 *    -5 : PT100 / PT1000 with MAX31865 (only for sensors 0-1)
 *    -3 : thermocouple with MAX31855 (only for sensors 0-1)
 *    -2 : thermocouple with MAX6675 (only for sensors 0-1)
 *    -4 : thermocouple with AD8495
 *    -1 : thermocouple with AD595
 *     0 : not used
 *     1 : 100k thermistor - best choice for EPCOS 100k (4.7k pullup)
 *   331 : (3.3V scaled thermistor 1 table for MEGA)
 *   332 : (3.3V scaled thermistor 1 table for DUE)
 *     2 : 200k thermistor - ATC Semitec 204GT-2 (4.7k pullup)
 *   202 : 200k thermistor - Copymaster 3D
 *     3 : Mendel-parts thermistor (4.7k pullup)
 *     4 : 10k thermistor !! do not use it for a hotend. It gives bad resolution at high temp. !!
 *     5 : 100K thermistor - ATC Semitec 104GT-2/104NT-4-R025H42G (Used in ParCan, J-Head, and E3D) (4.7k pullup)
 *   501 : 100K Zonestar (Tronxy X3A) Thermistor
 *   502 : 100K Zonestar Thermistor used by hot bed in Zonestar Průša P802M
 *   512 : 100k RPW-Ultra hotend thermistor (4.7k pullup)
 *     6 : 100k EPCOS - Not as accurate as table 1 (created using a fluke thermocouple) (4.7k pullup)
 *     7 : 100k Honeywell thermistor 135-104LAG-J01 (4.7k pullup)
 *    71 : 100k Honeywell thermistor 135-104LAF-J01 (4.7k pullup)
 *     8 : 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
 *     9 : 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
 *    10 : 100k RS thermistor 198-961 (4.7k pullup)
 *    11 : 100k beta 3950 1% thermistor (Used in Keenovo AC silicone mats and most Wanhao i3 machines) (4.7k pullup)
 *    12 : 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup) (calibrated for Makibox hot bed)
 *    13 : 100k Hisens 3950  1% up to 300°C for hotend "Simple ONE " & "Hotend "All In ONE"
 *    15 : 100k thermistor calibration for JGAurora A5 hotend
 *    18 : ATC Semitec 204GT-2 (4.7k pullup) Dagoma.Fr - MKS_Base_DKU001327
 *    20 : Pt100 with circuit in the Ultimainboard V2.x with mainboard ADC reference voltage = INA826 amplifier-board supply voltage.
 *         NOTES: (1) Must use an ADC input with no pullup. (2) Some INA826 amplifiers are unreliable at 3.3V so consider using sensor 147, 110, or 21.
 *    21 : Pt100 with circuit in the Ultimainboard V2.x with 3.3v ADC reference voltage (STM32, LPC176x....) and 5V INA826 amplifier board supply.
 *         NOTE: ADC pins are not 5V tolerant. Not recommended because it's possible to damage the CPU by going over 500°C.
 *    22 : 100k (hotend) with 4.7k pullup to 3.3V and 220R to analog input (as in GTM32 Pro vB)
 *    23 : 100k (bed) with 4.7k pullup to 3.3v and 220R to analog input (as in GTM32 Pro vB)
 *    30 : Kis3d Silicone heating mat 200W/300W with 6mm precision cast plate (EN AW 5083) NTC100K / B3950 (4.7k pullup)
 *   201 : Pt100 with circuit in Overlord, similar to Ultimainboard V2.x
 *    60 : 100k Maker's Tool Works Kapton Bed Thermistor beta=3950
 *    61 : 100k Formbot / Vivedino 3950 350C thermistor 4.7k pullup
 *    66 : 4.7M High Temperature thermistor from Dyze Design
 *    67 : 450C thermistor from SliceEngineering
 *    70 : the 100K thermistor found in the bq Hephestos 2
 *    75 : 100k Generic Silicon Heat Pad with NTC 100K MGB18-104F39050L32 thermistor
 *    99 : 100k thermistor with a 10K pull-up resistor (found on some Wanhao i3 machines)
 *
 *       1k ohm pullup tables - This is atypical, and requires changing out the 4.7k pullup for 1k.
 *                              (but gives greater accuracy and more stable PID)
 *    51 : 100k thermistor - EPCOS (1k pullup)
 *    52 : 200k thermistor - ATC Semitec 204GT-2 (1k pullup)
 *    55 : 100k thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (1k pullup)
 *
 *  1047 : Pt1000 with 4k7 pullup (E3D)
 *  1010 : Pt1000 with 1k pullup (non standard)
 *   147 : Pt100 with 4k7 pullup
 *   110 : Pt100 with 1k pullup (non standard)
 *
 *  1000 : Custom - Specify parameters in Configuration_adv.h
 *
 *         Use these for Testing or Development purposes. NEVER for production machine.
 *   998 : Dummy Table that ALWAYS reads 25°C or the temperature defined below.
 *   999 : Dummy Table that ALWAYS reads 100°C or the temperature defined below.
 */
#define TEMP_SENSOR_0                  999
#define TEMP_SENSOR_1                  0
#define TEMP_SENSOR_2                  0
#define TEMP_SENSOR_3                  0
#define TEMP_SENSOR_4                  0
#define TEMP_SENSOR_5                  0
#define TEMP_SENSOR_6                  0
#define TEMP_SENSOR_7                  0
#define TEMP_SENSOR_BED                0
#define TEMP_SENSOR_PROBE              0
#define TEMP_SENSOR_CHAMBER            0

// Dummy thermistor constant temperature readings, for use with 998 and 999
#define DUMMY_THERMISTOR_998_VALUE     25
#define DUMMY_THERMISTOR_999_VALUE     100

// Resistor values when using MAX31865 sensors (-5) on TEMP_SENSOR_0 / 1
// #define MAX31865_SENSOR_OHMS_0      100   // (Ω) Typically 100 or 1000 (PT100 or PT1000)
// #define MAX31865_CALIBRATION_OHMS_0 430   // (Ω) Typically 430 for AdaFruit PT100; 4300 for AdaFruit PT1000
// #define MAX31865_SENSOR_OHMS_1      100
// #define MAX31865_CALIBRATION_OHMS_1 430

// Use temp sensor 1 as a redundant sensor with sensor 0. If the readings
// from the two sensors differ too much the print will be aborted.
// #define TEMP_SENSOR_1_AS_REDUNDANT
#define MAX_REDUNDANT_TEMP_SENSOR_DIFF 10

#define TEMP_RESIDENCY_TIME            10 // (seconds) Time to wait for hotend to "settle" in M109
#define TEMP_WINDOW                    1 // (°C) Temperature proximity for the "temperature reached" timer
#define TEMP_HYSTERESIS                3 // (°C) Temperature proximity considered "close enough" to the target

#define TEMP_BED_RESIDENCY_TIME        10 // (seconds) Time to wait for bed to "settle" in M190
#define TEMP_BED_WINDOW                1 // (°C) Temperature proximity for the "temperature reached" timer
#define TEMP_BED_HYSTERESIS            3 // (°C) Temperature proximity considered "close enough" to the target

// Below this temperature the heater will be switched off
// because it probably indicates a broken thermistor wire.
#define HEATER_0_MINTEMP               5
#define HEATER_1_MINTEMP               5
#define HEATER_2_MINTEMP               5
#define HEATER_3_MINTEMP               5
#define HEATER_4_MINTEMP               5
#define HEATER_5_MINTEMP               5
#define HEATER_6_MINTEMP               5
#define HEATER_7_MINTEMP               5
#define BED_MINTEMP                    5

// Above this temperature the heater will be switched off.
// This can protect components from overheating, but NOT from shorts and failures.
// (Use MINTEMP for thermistor short/failure protection.)
#define HEATER_0_MAXTEMP               275
#define HEATER_1_MAXTEMP               275
#define HEATER_2_MAXTEMP               275
#define HEATER_3_MAXTEMP               275
#define HEATER_4_MAXTEMP               275
#define HEATER_5_MAXTEMP               275
#define HEATER_6_MAXTEMP               275
#define HEATER_7_MAXTEMP               275
#define BED_MAXTEMP                    150

//===========================================================================
//====================== PID > Bed Temperature Control ======================
//===========================================================================

/**
 * PID Bed Heating
 *
 * If this option is enabled set PID constants below.
 * If this option is disabled, bang-bang will be used and BED_LIMIT_SWITCHING will enable hysteresis.
 *
 * The PID frequency will be the same as the extruder PWM.
 * If PID_dT is the default, and correct for the hardware/configuration, that means 7.689Hz,
 * which is fine for driving a square wave into a resistive load and does not significantly
 * impact FET heating. This also works fine on a Fotek SSR-10DA Solid State Relay into a 250W
 * heater. If your configuration is significantly different than this and you don't understand
 * the issues involved, don't use bed PID until someone else verifies that your hardware works.
 */
// #define PIDTEMPBED

// #define BED_LIMIT_SWITCHING

/**
 * Max Bed Power
 * Applies to all forms of bed control (PID, bang-bang, and bang-bang with hysteresis).
 * When set to any value below 255, enables a form of PWM to the bed that acts like a divider
 * so don't use it unless you are OK with PWM on your bed. (See the comment on enabling PIDTEMPBED)
 */
#define MAX_BED_POWER                  255 // limits duty cycle to bed; 255=full current

#if ENABLED(PIDTEMPBED)
// #define MIN_BED_POWER 0
// #define PID_BED_DEBUG // Sends debug data to the serial port.

// 120V 250W silicone heater into 4mm borosilicate (MendelMax 1.5+)
// from FOPDT model - kp=.39 Tp=405 Tdead=66, Tc set to 79.2, aggressive factor of .15 (vs .1, 1, 10)
#	define DEFAULT_bedKp 10.00
#	define DEFAULT_bedKi .023
#	define DEFAULT_bedKd 305.4

// FIND YOUR OWN: "M303 E-1 C8 S90" to run autotune on the bed at 90 degreesC for 8 cycles.
#endif // PIDTEMPBED

#if EITHER(PIDTEMP, PIDTEMPBED)
// #define PID_DEBUG             // Sends debug data to the serial port. Use 'M303 D' to toggle activation.
// #define PID_OPENLOOP          // Puts PID in open loop. M104/M140 sets the output power from 0 to PID_MAX
// #define SLOW_PWM_HEATERS      // PWM with very low frequency (roughly 0.125Hz=8s) and minimum state time of approximately 1s useful for heaters driven by a relay
#	define PID_FUNCTIONAL_RANGE 10 // If the temperature difference between the target temperature and the actual temperature
		                              // is more than PID_FUNCTIONAL_RANGE then the PID will be shut off and the heater will be set to min/max.
#endif

//===========================================================================
//============================== Endstop Settings ===========================
//===========================================================================

// @section homing

// Specify here all the endstop connectors that are connected to any endstop or probe.
// Almost all printers will be using one per axis. Probes will use one or more of the
// extra connectors. Leave undefined any used for non-endstop and non-probe purposes.
#define USE_XMIN_PLUG
#define USE_YMIN_PLUG
// #define USE_ZMIN_PLUG
// #define USE_XMAX_PLUG
#define USE_YMAX_PLUG
#define USE_ZMAX_PLUG

// Enable pullup for all endstops to prevent a floating state
// #define ENDSTOPPULLUPS
#if DISABLED(ENDSTOPPULLUPS)
// Disable ENDSTOPPULLUPS to set pullups individually
// #define ENDSTOPPULLUP_XMAX
// #define ENDSTOPPULLUP_YMAX
// #define ENDSTOPPULLUP_ZMAX
// #define ENDSTOPPULLUP_XMIN
// #define ENDSTOPPULLUP_YMIN
// #define ENDSTOPPULLUP_ZMIN
// #define ENDSTOPPULLUP_ZMIN_PROBE
// #define ENDSTOPPULLUP_TOOL_PROBE
// #define ENDSTOPPULLUP_WORK_PROBE
#endif

// Enable pulldown for all endstops to prevent a floating state
#define ENDSTOPPULLDOWNS
#if DISABLED(ENDSTOPPULLDOWNS)
// Disable ENDSTOPPULLDOWNS to set pulldowns individually
// #define ENDSTOPPULLDOWN_XMAX
// #define ENDSTOPPULLDOWN_YMAX
// #define ENDSTOPPULLDOWN_ZMAX
// #define ENDSTOPPULLDOWN_XMIN
// #define ENDSTOPPULLDOWN_YMIN
// #define ENDSTOPPULLDOWN_ZMIN
// #define ENDSTOPPULLDOWN_ZMIN_PROBE
// #define ENDSTOPPULLDOWN_WORK_PROBE
#endif

// Mechanical endstop with COM to ground and NC to Signal uses "false" here (most common setup).
#define X_MIN_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define Y_MIN_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define Z_MIN_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define X_MAX_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define Y_MAX_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define Z_MAX_ENDSTOP_INVERTING       true // Set to true to invert the logic of the endstop.
#define Z_MIN_PROBE_ENDSTOP_INVERTING true // Set to true to invert the logic of the probe.
#define TOOL_PROBE_ENDSTOP_INVERTING  true
#define WORK_PROBE_ENDSTOP_INVERTING  true
#define ESTOP_ENDSTOP_INVERTING       true

/**
 * Stepper Drivers
 *
 * These settings allow Marlin to tune stepper driver timing and enable advanced options for
 * stepper drivers that support them. You may also override timing options in Configuration_adv.h.
 *
 * A4988 is assumed for unspecified drivers.
 *
 * Use TMC2208/TMC2208_STANDALONE for TMC2225 drivers and TMC2209/TMC2209_STANDALONE for TMC2226 drivers.
 *
 * Options: A4988, A5984, DRV8825, LV8729, L6470, L6474, POWERSTEP01,
 *          TB6560, TB6600, TMC2100,
 *          TMC2130, TMC2130_STANDALONE, TMC2160, TMC2160_STANDALONE,
 *          TMC2208, TMC2208_STANDALONE, TMC2209, TMC2209_STANDALONE,
 *          TMC26X,  TMC26X_STANDALONE,  TMC2660, TMC2660_STANDALONE,
 *          TMC5130, TMC5130_STANDALONE, TMC5160, TMC5160_STANDALONE
 * :['A4988', 'A5984', 'DRV8825', 'LV8729', 'L6470', 'L6474', 'POWERSTEP01', 'TB6560', 'TB6600', 'TMC2100', 'TMC2130', 'TMC2130_STANDALONE', 'TMC2160', 'TMC2160_STANDALONE', 'TMC2208', 'TMC2208_STANDALONE', 'TMC2209', 'TMC2209_STANDALONE', 'TMC26X', 'TMC26X_STANDALONE', 'TMC2660', 'TMC2660_STANDALONE', 'TMC5130', 'TMC5130_STANDALONE', 'TMC5160', 'TMC5160_STANDALONE']
 */
#define X_DRIVER_TYPE                 DM556T
#define Y_DRIVER_TYPE                 DM556T
#define Z_DRIVER_TYPE                 DM556T
// #define X2_DRIVER_TYPE A4988
#define Y2_DRIVER_TYPE                DM556T
// #define Z2_DRIVER_TYPE A4988
// #define Z3_DRIVER_TYPE A4988
// #define Z4_DRIVER_TYPE A4988
#define E0_DRIVER_TYPE                DM556T
// #define E1_DRIVER_TYPE A4988
// #define E2_DRIVER_TYPE A4988
// #define E3_DRIVER_TYPE A4988
// #define E4_DRIVER_TYPE A4988
// #define E5_DRIVER_TYPE A4988
// #define E6_DRIVER_TYPE A4988
// #define E7_DRIVER_TYPE A4988

// Enable this feature if all enabled endstop pins are interrupt-capable.
// This will remove the need to poll the interrupt pins, saving many CPU cycles.
#define ENDSTOP_INTERRUPTS_FEATURE

/**
 * Endstop Noise Threshold
 *
 * Enable if your probe or endstops falsely trigger due to noise.
 *
 * - Higher values may affect repeatability or accuracy of some bed probes.
 * - To fix noise install a 100nF ceramic capacitor in parallel with the switch.
 * - This feature is not required for common micro-switches mounted on PCBs
 *   based on the Makerbot design, which already have the 100nF capacitor.
 *
 * :[2,3,4,5,6,7]
 */
// #define ENDSTOP_NOISE_THRESHOLD 2

// Check for stuck or disconnected endstops during homing moves.
// #define DETECT_BROKEN_ENDSTOP

//=============================================================================
//============================== Movement Settings ============================
//=============================================================================
// @section motion

/**
 * Default Settings
 *
 * These settings can be reset by M502
 *
 * Note that if EEPROM is enabled, saved values will override these.
 */

/**
 * Default Axis Steps Per Unit (steps/mm)
 * Override with M92
 *                                      X, Y, Z, E0 [, E1 [, E2...]]
 */
#define DEFAULT_AXIS_STEPS_PER_UNIT \
	{ \
		400, 400, 400, 400 \
	}

/**
 * Default Max Feed Rate (mm/s)
 * Override with M203
 *                                      X, Y, Z, E0 [, E1 [, E2...]]
 */
#define DEFAULT_MAX_FEEDRATE \
	{ \
		208.33333333333333, 208.33333333333333, 133.33333333333333, 333.33333333333333 \
	}

// #define LIMITED_MAX_FR_EDITING        // Limit edit via M203 or LCD to DEFAULT_MAX_FEEDRATE * 2
#if ENABLED(LIMITED_MAX_FR_EDITING)
#	define MAX_FEEDRATE_EDIT_VALUES \
		{ \
			20000, 20000, 20000, 20000 \
		} // ...or, set your own edit limits
#endif

/**
 * Default Max Acceleration (change/s) change = mm/s
 * (Maximum start speed for accelerated moves)
 * Override with M201
 *                                      X, Y, Z, E0 [, E1 [, E2...]]
 */
#define DEFAULT_MAX_ACCELERATION \
	{ \
		1000, 1000, 1000, 1000 \
	}

// #define LIMITED_MAX_ACCEL_EDITING     // Limit edit via M201 or LCD to DEFAULT_MAX_ACCELERATION * 2
#if ENABLED(LIMITED_MAX_ACCEL_EDITING)
#	define MAX_ACCEL_EDIT_VALUES \
		{ \
			600, 600, 600, 200 \
		} // ...or, set your own edit limits
#endif

/**
 * Default Acceleration (change/s) change = mm/s
 * Override with M204
 *
 *   M204 P    Acceleration
 *   M204 R    Retract Acceleration
 *   M204 T    Travel Acceleration
 */
#define DEFAULT_ACCELERATION         100 // X, Y, Z and E acceleration for printing moves
#define DEFAULT_RETRACT_ACCELERATION 100 // E acceleration for retracts
#define DEFAULT_TRAVEL_ACCELERATION  100 // X, Y, Z acceleration for travel (non printing) moves

/**
 * Default Jerk limits (mm/s)
 * Override with M205 X Y Z E
 *
 * "Jerk" specifies the minimum speed change that requires acceleration.
 * When changing speed and direction, if the difference is less than the
 * value set here, it may happen instantaneously.
 */
// #define CLASSIC_JERK
#if ENABLED(CLASSIC_JERK)
#	define DEFAULT_XJERK 10.0
#	define DEFAULT_YJERK 10.0
#	define DEFAULT_ZJERK 0.3
#	define DEFAULT_AJERK 10.0

// #define TRAVEL_EXTRA_XYJERK 0.0     // Additional jerk allowance for all travel moves

// #define LIMITED_JERK_EDITING        // Limit edit via M205 or LCD to DEFAULT_aJERK * 2
#	if ENABLED(LIMITED_JERK_EDITING)
#		define MAX_JERK_EDIT_VALUES \
			{ \
				20, 20, 0.6, 10 \
			} // ...or, set your own edit limits
#	endif
#endif

#define DEFAULT_EJERK 5.0 // May be used by Linear Advance

/**
 * Junction Deviation Factor
 *
 * See:
 *   https://reprap.org/forum/read.php?1,739819
 *   https://blog.kyneticcnc.com/2018/10/computing-junction-deviation-for-marlin.html
 */
#if DISABLED(CLASSIC_JERK)
#	define JUNCTION_DEVIATION_MM    0.013 // (mm) Distance from real junction edge
#	define JD_HANDLE_SMALL_SEGMENTS // Use curvature estimation instead of just the junction angle
	                                 // for small segments (< 1mm) with large junction angles (> 135°).
#endif

/**
 * S-Curve Acceleration
 *
 * This option eliminates vibration during printing by fitting a Bézier
 * curve to move acceleration, producing much smoother direction changes.
 *
 * See https://github.com/synthetos/TinyG/wiki/Jerk-Controlled-Motion-Explained
 */
#define S_CURVE_ACCELERATION

/**
 * Nozzle-to-Probe offsets { X, Y, Z }
 *
 * X and Y offset
 *   Use a caliper or ruler to measure the distance from the tip of
 *   the Nozzle to the center-point of the Probe in the X and Y axes.
 *
 * Z offset
 * - For the Z offset use your best known value and adjust at runtime.
 * - Common probes trigger below the nozzle and have negative values for Z offset.
 * - Probes triggering above the nozzle height are uncommon but do exist. When using
 *   probes such as this, carefully set Z_CLEARANCE_DEPLOY_PROBE and Z_CLEARANCE_BETWEEN_PROBES
 *   to avoid collisions during probing.
 *
 * Tune and Adjust
 * -  Probe Offsets can be tuned at runtime with 'M851', LCD menus, babystepping, etc.
 * -  PROBE_OFFSET_WIZARD (configuration_adv.h) can be used for setting the Z offset.
 *
 * Assuming the typical work area orientation:
 *  - Probe to RIGHT of the Nozzle has a Positive X offset
 *  - Probe to LEFT  of the Nozzle has a Negative X offset
 *  - Probe in BACK  of the Nozzle has a Positive Y offset
 *  - Probe in FRONT of the Nozzle has a Negative Y offset
 *
 * Some examples:
 *   #define NOZZLE_TO_PROBE_OFFSET { 10, 10, -1 }   // Example "1"
 *   #define NOZZLE_TO_PROBE_OFFSET {-10,  5, -1 }   // Example "2"
 *   #define NOZZLE_TO_PROBE_OFFSET {  5, -5, -1 }   // Example "3"
 *   #define NOZZLE_TO_PROBE_OFFSET {-15,-10, -1 }   // Example "4"
 *
 *     +-- BACK ---+
 *     |    [+]    |
 *   L |        1  | R <-- Example "1" (right+,  back+)
 *   E |  2        | I <-- Example "2" ( left-,  back+)
 *   F |[-]  N  [+]| G <-- Nozzle
 *   T |       3   | H <-- Example "3" (right+, front-)
 *     | 4         | T <-- Example "4" ( left-, front-)
 *     |    [-]    |
 *     O-- FRONT --+
 */
#define NOZZLE_TO_PROBE_OFFSET \
	{ \
		10, 10, 0 \
	}

// Most probes should stay away from the edges of the bed, but
// with NOZZLE_AS_PROBE this can be negative for a wider probing area.
#define PROBING_MARGIN     10

// X and Y axis travel speed (mm/min) between probes
#define XY_PROBE_SPEED     (133 * 60)

// Feedrate (mm/min) for the first approach when double-probing (MULTIPLE_PROBING == 2)
#define Z_PROBE_SPEED_FAST (4 * 60)

// Feedrate (mm/min) for the "accurate" probe of each point
#define Z_PROBE_SPEED_SLOW (Z_PROBE_SPEED_FAST / 2)

/**
 * Probe Activation Switch
 * A switch indicating proper deployment, or an optical
 * switch triggered when the carriage is near the bed.
 */
// #define PROBE_ACTIVATION_SWITCH
#if ENABLED(PROBE_ACTIVATION_SWITCH)
#	define PROBE_ACTIVATION_SWITCH_STATE LOW // State indicating probe is active
//#define PROBE_ACTIVATION_SWITCH_PIN PC6 // Override default pin
#endif

/**
 * Multiple Probing
 *
 * You may get improved results by probing 2 or more times.
 * With EXTRA_PROBING the more atypical reading(s) will be disregarded.
 *
 * A total of 2 does fast/slow probes with a weighted average.
 * A total of 3 or more adds more slow probes, taking the average.
 */
#define MULTIPLE_PROBING         2
#define EXTRA_PROBING            1

/**
 * Z probes require clearance when deploying, stowing, and moving between
 * probe points to avoid hitting the bed and other hardware.
 * Servo-mounted probes require extra space for the arm to rotate.
 * Inductive probes need space to keep from triggering early.
 *
 * Use these settings to specify the distance (mm) to raise the probe (or
 * lower the bed). The values set here apply over and above any (negative)
 * probe Z Offset set with NOZZLE_TO_PROBE_OFFSET, M851, or the LCD.
 * Only integer values >= 1 are valid here.
 *
 * Example: `M851 Z-5` with a CLEARANCE of 4  =>  9mm from bed to nozzle.
 *     But: `M851 Z+1` with a CLEARANCE of 2  =>  2mm from bed to nozzle.
 */
#define Z_CLEARANCE_DEPLOY_PROBE 10 // Z Clearance for Deploy/Stow
// #define Z_CLEARANCE_BETWEEN_PROBES  5 // Z Clearance between probe points
#define Z_CLEARANCE_MULTI_PROBE  5 // Z Clearance between multiple probes
// #define Z_AFTER_PROBING           5 // Z position after probing is done

#define Z_PROBE_LOW_POINT        -2 // Farthest distance below the trigger-point to go before stopping

// For M851 give a range for adjusting the Z probe offset
#define Z_PROBE_OFFSET_RANGE_MIN -20
#define Z_PROBE_OFFSET_RANGE_MAX 20

// Enable the M48 repeatability test to test probe accuracy
// #define Z_MIN_PROBE_REPEATABILITY_TEST

// Before deploy/stow pause for user confirmation
// #define PAUSE_BEFORE_DEPLOY_STOW
#if ENABLED(PAUSE_BEFORE_DEPLOY_STOW)
// #define PAUSE_PROBE_DEPLOY_WHEN_TRIGGERED // For Manual Deploy Allenkey Probe
#endif

/**
 * Enable one or more of the following if probing seems unreliable.
 * Heaters and/or fans can be disabled during probing to minimize electrical
 * noise. A delay can also be added to allow noise and vibration to settle.
 * These options are most useful for the BLTouch probe, but may also improve
 * readings with inductive probes and piezo sensors.
 */
// #define PROBING_HEATERS_OFF       // Turn heaters off when probing
#if ENABLED(PROBING_HEATERS_OFF)
// #define WAIT_FOR_BED_HEATER     // Wait for bed to heat back up between probes (to improve accuracy)
#endif
// #define PROBING_FANS_OFF          // Turn fans off when probing
// #define PROBING_STEPPERS_OFF      // Turn steppers off (unless needed to hold position) when probing
// #define DELAY_BEFORE_PROBING 200  // (ms) To prevent vibrations from triggering piezo sensors

// Require minimum nozzle and/or bed temperature for probing
// #define PREHEAT_BEFORE_PROBING
#if ENABLED(PREHEAT_BEFORE_PROBING)
#	define PROBING_NOZZLE_TEMP 120 // (°C) Only applies to E0 at this time
#	define PROBING_BED_TEMP    50
#endif

// For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1
// :{ 0:'Low', 1:'High' }
#define X_ENABLE_ON  1
#define Y_ENABLE_ON  1
#define Z_ENABLE_ON  1
#define A_ENABLE_ON  1 // For all extruders

// Disable axis steppers immediately when they're not being stepped.
// WARNING: When motors turn off there is a chance of losing position accuracy!
#define DISABLE_X    false
#define DISABLE_Y    false
#define DISABLE_Z    false
#define DISABLE_A    false
// Turn off the display blinking that warns about possible accuracy reduction
// #define DISABLE_REDUCED_ACCURACY_WARNING

// @section extruder

// @section machine

// Invert the stepper direction. Change (or reverse the motor connector) if an axis goes the wrong way.
#define INVERT_X_DIR false
#define INVERT_Y_DIR true
#define INVERT_Z_DIR true
#define INVERT_A_DIR true

// @section homing

#define NO_MOTION_BEFORE_HOMING   // Inhibit movement until all axes have been homed. Also enable HOME_AFTER_DEACTIVATE for extra safety.
// #define HOME_AFTER_DEACTIVATE   // Require rehoming after steppers are deactivated. Also enable NO_MOTION_BEFORE_HOMING for extra safety.
// #define UNKNOWN_Z_NO_RAISE      // Don't raise Z (lower the bed) if Z is "unknown." For beds that fall when Z is powered off.

// #define Z_HOMING_HEIGHT  4      // (mm) Minimal Z height before homing (G28) for Z clearance above the bed, clamps, ...
//  Be sure to have this much clearance over your Z_MAX_POS to prevent grinding.

// #define Z_AFTER_HOMING  0      // (mm) Height to move to after homing Z

// Direction of endstops when homing; 1=MAX, -1=MIN
// :[-1,1]
#define X_HOME_DIR   -1
#define Y_HOME_DIR   1
#define Z_HOME_DIR   1

// @section machine

// The size of the print bed
#if MACHINE_TYPE == 1
#	define X_BED_SIZE 735
#	define Y_BED_SIZE 730
#elif MACHINE_TYPE == 2
#	define X_BED_SIZE 1355
#	define Y_BED_SIZE 730
#elif MACHINE_TYPE == 3
#	define X_BED_SIZE 1355
#	define Y_BED_SIZE 1330
#elif MACHINE_TYPE == 4
#	define X_BED_SIZE 1355
#	define Y_BED_SIZE 2530
#else
#	define X_BED_SIZE 1355
#	define Y_BED_SIZE 1330
#endif

// Travel limits (mm) after homing, corresponding to endstop positions.
#define X_MIN_POS 0
#define Y_MIN_POS 0
#define Z_MIN_POS -155
#define X_MAX_POS X_BED_SIZE
#define Y_MAX_POS Y_BED_SIZE
#define Z_MAX_POS 0

/**
 * Software Endstops
 *
 * - Prevent moves outside the set machine bounds.
 * - Individual axes can be disabled, if desired.
 * - X and Y only apply to Cartesian robots.
 * - Use 'M211' to set software endstops on/off or report current state
 */

// Min software endstops constrain movement within minimum coordinate bounds
#define MIN_SOFTWARE_ENDSTOPS
#if ENABLED(MIN_SOFTWARE_ENDSTOPS)
#	define MIN_SOFTWARE_ENDSTOP_X
#	define MIN_SOFTWARE_ENDSTOP_Y
#	define MIN_SOFTWARE_ENDSTOP_Z
#endif

// Max software endstops constrain movement within maximum coordinate bounds
#define MAX_SOFTWARE_ENDSTOPS
#if ENABLED(MAX_SOFTWARE_ENDSTOPS)
#	define MAX_SOFTWARE_ENDSTOP_X
#	define MAX_SOFTWARE_ENDSTOP_Y
#	define MAX_SOFTWARE_ENDSTOP_Z
#endif

#if EITHER(MIN_SOFTWARE_ENDSTOPS, MAX_SOFTWARE_ENDSTOPS)
#	define SOFT_ENDSTOPS_MENU_ITEM // Enable/Disable software endstops from the LCD
#endif

/**
 * Enable detailed logging of G28, G29, M48, etc.
 * Turn on with the command 'M111 S32'.
 * NOTE: Requires a lot of PROGMEM!
 */
#define DEBUG_LEVELING_FEATURE

// @section homing

// The center of the bed is at (X=0, Y=0)
// #define BED_CENTER_AT_0_0

// Manually set the home position. Leave these undefined for automatic settings.
// For DELTA this is the top-center of the Cartesian print volume.
// #define MANUAL_X_HOME_POS 0
// #define MANUAL_Y_HOME_POS 0
// #define MANUAL_Z_HOME_POS 0

// Use "Z Safe Homing" to avoid homing with a Z probe outside the bed area.
//
// With this feature enabled:
//
// - Allow Z homing only after X and Y homing AND stepper drivers still enabled.
// - If stepper drivers time out, it will need X and Y homing again before Z homing.
// - Move the Z probe (or nozzle) to a defined XY point before Z Homing.
// - Prevent Z homing when the Z probe is outside bed area.
//
// #define Z_SAFE_HOMING

#if ENABLED(Z_SAFE_HOMING)
#	define Z_SAFE_HOMING_X_POINT 0 // X point for Z homing
#	define Z_SAFE_HOMING_Y_POINT Y_BED_SIZE // Y point for Z homing
#endif

// Homing speeds (mm/min)
// #define HOMING_FEEDRATE_MM_M { (50*60), (50*60), (50*60) }
#define HOMING_FEEDRATE_MM_M \
	{ \
		(4000), (4000), (2000) \
	}

// Validate that endstops are triggered on homing moves
// #define VALIDATE_HOMING_ENDSTOPS

// @section calibrate

/**
 * Bed Skew Compensation
 *
 * This feature corrects for misalignment in the XYZ axes.
 *
 * Take the following steps to get the bed skew in the XY plane:
 *  1. Print a test square (e.g., https://www.thingiverse.com/thing:2563185)
 *  2. For XY_DIAG_AC measure the diagonal A to C
 *  3. For XY_DIAG_BD measure the diagonal B to D
 *  4. For XY_SIDE_AD measure the edge A to D
 *
 * Marlin automatically computes skew factors from these measurements.
 * Skew factors may also be computed and set manually:
 *
 *  - Compute AB     : SQRT(2*AC*AC+2*BD*BD-4*AD*AD)/2
 *  - XY_SKEW_FACTOR : TAN(PI/2-ACOS((AC*AC-AB*AB-AD*AD)/(2*AB*AD)))
 *
 * If desired, follow the same procedure for XZ and YZ.
 * Use these diagrams for reference:
 *
 *    Y                     Z                     Z
 *    ^     B-------C       ^     B-------C       ^     B-------C
 *    |    /       /        |    /       /        |    /       /
 *    |   /       /         |   /       /         |   /       /
 *    |  A-------D          |  A-------D          |  A-------D
 *    +-------------->X     +-------------->X     +-------------->Y
 *     XY_SKEW_FACTOR        XZ_SKEW_FACTOR        YZ_SKEW_FACTOR
 */
// #define SKEW_CORRECTION

#if ENABLED(SKEW_CORRECTION)
// Input all length measurements here:
#	define XY_DIAG_AC     282.8427124746
#	define XY_DIAG_BD     282.8427124746
#	define XY_SIDE_AD     200

// Or, set the default skew factors directly here
// to override the above measurements:
#	define XY_SKEW_FACTOR 0.0

// #define SKEW_CORRECTION_FOR_Z
#	if ENABLED(SKEW_CORRECTION_FOR_Z)
#		define XZ_DIAG_AC     282.8427124746
#		define XZ_DIAG_BD     282.8427124746
#		define YZ_DIAG_AC     282.8427124746
#		define YZ_DIAG_BD     282.8427124746
#		define YZ_SIDE_AD     200
#		define XZ_SKEW_FACTOR 0.0
#		define YZ_SKEW_FACTOR 0.0
#	endif

// Enable this option for M852 to set skew at runtime
// #define SKEW_CORRECTION_GCODE
#endif

//=============================================================================
//============================= Additional Features ===========================
//=============================================================================

// @section extras

/**
 * EEPROM
 *
 * Persistent storage to preserve configurable settings across reboots.
 *
 *   M500 - Store settings to EEPROM.
 *   M501 - Read settings from EEPROM. (i.e., Throw away unsaved changes)
 *   M502 - Revert settings to "factory" defaults. (Follow with M500 to init the EEPROM.)
 */
#define EEPROM_SETTINGS    // Persistent storage with M500 and M501
// #define DISABLE_M503        // Saves ~2700 bytes of PROGMEM. Disable for release!
#define EEPROM_CHITCHAT    // Give feedback on EEPROM commands. Disable to save PROGMEM.
#define EEPROM_BOOT_SILENT // Keep M503 quiet and only give errors during first load
#if ENABLED(EEPROM_SETTINGS)
#	define EEPROM_AUTO_INIT // Init EEPROM automatically on any errors.
#endif

//
// Host Keepalive
//
// When enabled Marlin will send a busy status message to the host
// every couple of seconds when it can't accept commands.
//
#define HOST_KEEPALIVE_FEATURE     // Disable this if your host doesn't like keepalive messages
#define DEFAULT_KEEPALIVE_INTERVAL 0.5 // Number of seconds between "busy" messages. Set with M113.
#define BUSY_WHILE_HEATING         // Some hosts require "busy" messages even during heating

//
// G20/G21 Inch mode support
//
#define INCH_MODE_SUPPORT

//
// M149 Set temperature units support
//
// #define TEMPERATURE_UNITS_SUPPORT

// @section temperature

// Preheat Constants
#define PREHEAT_1_LABEL       "PLA"
#define PREHEAT_1_TEMP_HOTEND 180
#define PREHEAT_1_TEMP_BED    70
#define PREHEAT_1_FAN_SPEED   0 // Value from 0 to 255

#define PREHEAT_2_LABEL       "ABS"
#define PREHEAT_2_TEMP_HOTEND 240
#define PREHEAT_2_TEMP_BED    110
#define PREHEAT_2_FAN_SPEED   0 // Value from 0 to 255

/**
 * Print Job Timer
 *
 * Automatically start and stop the print job timer on M104/M109/M190.
 *
 *   M104 (hotend, no wait) - high temp = none,        low temp = stop timer
 *   M109 (hotend, wait)    - high temp = start timer, low temp = stop timer
 *   M190 (bed, wait)       - high temp = start timer, low temp = none
 *
 * The timer can also be controlled with the following commands:
 *
 *   M75 - Start the print job timer
 *   M76 - Pause the print job timer
 *   M77 - Stop the print job timer
 */
#define PRINTJOB_TIMER_AUTOSTART

/**
 * Print Counter
 *
 * Track statistical data such as:
 *
 *  - Total print jobs
 *  - Total successful print jobs
 *  - Total failed print jobs
 *  - Total time printing
 *
 * View the current statistics with M78.
 */
//#define PRINTCOUNTER

/**
 * SD CARD
 *
 * SD Card support is disabled by default. If your controller has an SD slot,
 * you must uncomment the following option or it won't work.
 */
#define SDSUPPORT

/**
 * SD CARD: ENABLE CRC
 *
 * Use CRC checks and retries on the SD communication.
 */
//#define SD_CHECK_AND_RETRY

//=============================================================================
//=============================== Extra Features ==============================
//=============================================================================

// @section extras

// Set number of user-controlled fans. Disable to use all board-defined fans.
// :[1,2,3,4,5,6,7,8]
//#define NUM_M106_FANS 1

// Increase the FAN PWM frequency. Removes the PWM noise but increases heating in the FET/Arduino
//#define FAST_PWM_FAN

// Use software PWM to drive the fan, as for the heaters. This uses a very low frequency
// which is not as annoying as with the hardware PWM. On the other hand, if this frequency
// is too low, you should also increment SOFT_PWM_SCALE.
//#define FAN_SOFT_PWM

// Incrementing this by 1 will double the software PWM frequency,
// affecting heaters, and the fan if FAN_SOFT_PWM is enabled.
// However, control resolution will be halved for each increment;
// at zero value, there are 128 effective control positions.
// :[0,1,2,3,4,5,6,7]
#define SOFT_PWM_SCALE 0

/**
 * RGB LED / LED Strip Control
 *
 * Enable support for an RGB LED connected to 5V digital pins, or
 * an RGB Strip connected to MOSFETs controlled by digital pins.
 *
 * Adds the M150 command to set the LED (or LED strip) color.
 * If pins are PWM capable (e.g., 4, 5, 6, 11) then a range of
 * luminance values can be set from 0 to 255.
 * For NeoPixel LED an overall brightness parameter is also available.
 *
 * *** CAUTION ***
 *  LED Strips require a MOSFET Chip between PWM lines and LEDs,
 *  as the Arduino cannot handle the current the LEDs will require.
 *  Failure to follow this precaution can destroy your Arduino!
 *  NOTE: A separate 5V power supply is required! The NeoPixel LED needs
 *  more current than the Arduino 5V linear regulator can produce.
 * *** CAUTION ***
 *
 * LED Type. Enable only one of the following two options.
 */
// #define RGB_LED
// #define RGBW_LED

#if EITHER(RGB_LED, RGBW_LED)
// #define RGB_LED_R_PIN 34
// #define RGB_LED_G_PIN 43
// #define RGB_LED_B_PIN 35
// #define RGB_LED_W_PIN -1
#endif

// Support for Adafruit NeoPixel LED driver
#define NEOPIXEL_LED
#if ENABLED(NEOPIXEL_LED)
#	define NEOPIXEL_TYPE          NEO_GRBW // NEO_GRBW / NEO_GRB - four/three channel driver type (defined in Adafruit_NeoPixel.h)
#	define NEOPIXEL_PIN           88 // LED driving pin
// #define NEOPIXEL2_TYPE NEOPIXEL_TYPE
// #define NEOPIXEL2_PIN    5
#	define NEOPIXEL_PIXELS        1 // Number of LEDs in the strip. (Longest strip when NEOPIXEL2_SEPARATE is disabled.)
#	define NEOPIXEL_IS_SEQUENTIAL // Sequential display for temperature change - LED by LED. Disable to change all LEDs at once.
#	define NEOPIXEL_BRIGHTNESS    127 // Initial brightness (0-255)
#	define NEOPIXEL_STARTUP_TEST  // Cycle through colors at startup

// Support for second Adafruit NeoPixel LED driver controlled with M150 S1 ...
// #define NEOPIXEL2_SEPARATE
#	if ENABLED(NEOPIXEL2_SEPARATE)
#		define NEOPIXEL2_PIXELS       15 // Number of LEDs in the second strip
#		define NEOPIXEL2_BRIGHTNESS   127 // Initial brightness (0-255)
#		define NEOPIXEL2_STARTUP_TEST // Cycle through colors at startup
#	else
// #define NEOPIXEL2_INSERIES      // Default behavior is NeoPixel 2 in parallel
#	endif

// Use a single NeoPixel LED for static (background) lighting
// #define NEOPIXEL_BKGD_LED_INDEX  0               // Index of the LED to use
// #define NEOPIXEL_BKGD_COLOR { 255, 255, 255, 0 } // R, G, B, W
#endif

/**
 * Printer Event LEDs
 *
 * During printing, the LEDs will reflect the printer status:
 *
 *  - Gradually change from blue to violet as the heated bed gets to target temp
 *  - Gradually change from violet to red as the hotend gets to temperature
 *  - Change to white to illuminate work surface
 *  - Change to green once print has finished
 *  - Turn off after the print has finished and the user has pushed a button
 */
#if ANY(BLINKM, RGB_LED, RGBW_LED, PCA9632, PCA9533, NEOPIXEL_LED)
#	define PRINTER_EVENT_LEDS
#endif

/**
 * Number of servos
 *
 * For some servo-related options NUM_SERVOS will be set automatically.
 * Set this manually if there are extra servos needing manual control.
 * Set to 0 to turn off servo support.
 */
// #define NUM_SERVOS 3 // Servo index starts with 0 for M280 command

// (ms) Delay  before the next move will start, to give the servo time to reach its target angle.
// 300ms is a good value but you can try less delay.
// If the servo can't reach the requested position, increase it.
#define SERVO_DELAY \
	{ \
		300 \
	}

// Only power servos during movement, otherwise leave off to prevent jitter
// #define DEACTIVATE_SERVOS_AFTER_MOVE

// Edit servo angles with M281 and save to EEPROM with M500
// #define EDITABLE_SERVO_ANGLES
