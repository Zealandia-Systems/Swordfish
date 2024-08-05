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
 * gcode.cpp - Temporary container for all gcode handlers
 *             Most will migrate to classes, by feature.
 */

#include <swordfish/math.h>

using namespace swordfish::math;

#include "gcode.h"
GcodeSuite gcode;

#if ENABLED(WIFI_CUSTOM_COMMAND)
extern bool wifi_custom_command(char* const command_ptr);
#endif

#include "parser.h"
#include "queue.h"
#include "../module/motion.h"
#include "../module/stepper.h"
#include "../feature/spindle_laser.h"

#if ENABLED(PRINTCOUNTER)
#	include "../module/printcounter.h"
#endif

#if ENABLED(HOST_PROMPT_SUPPORT)
#	include "../feature/host_actions.h"
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
#	include "../sd/cardreader.h"
#	include "../feature/powerloss.h"
#endif

#if ENABLED(CANCEL_OBJECTS)
#	include "../feature/cancel_object.h"
#endif

#if ENABLED(LASER_MOVE_POWER)

#endif

#if ENABLED(PASSWORD_FEATURE)
#	include "../feature/password/password.h"
#endif

#include "../MarlinCore.h" // for idle()

#include "../module/estop.h"

#include <swordfish/Controller.h>
#include <swordfish/core/Console.h>
#include <swordfish/modules/motion/LimitException.h>
#include <swordfish/modules/estop/EStopException.h>

using namespace swordfish;
using namespace swordfish::core;
using namespace swordfish::estop;
using namespace swordfish::io;
using namespace swordfish::motion;
using namespace swordfish::status;
using namespace swordfish::tools;

// Inactivity shutdown
millis_t GcodeSuite::previous_move_ms = 0,
				 GcodeSuite::max_inactive_time = 0,
				 GcodeSuite::stepper_inactive_time = SEC_TO_MS(DEFAULT_STEPPER_DEACTIVE_TIME);

// Relative motion mode for each logical axis
static constexpr xyze_bool_t ar_init = AXIS_RELATIVE_MODES;
uint8_t GcodeSuite::axis_relative = ((ar_init.x ? _BV(REL_X) : 0) | (ar_init.y ? _BV(REL_Y) : 0) | (ar_init.z ? _BV(REL_Z) : 0) | (ar_init.e ? _BV(REL_E) : 0));
bool GcodeSuite::aborted_ = false;

#if EITHER(HAS_AUTO_REPORTING, HOST_KEEPALIVE_FEATURE)
bool GcodeSuite::autoreport_paused; // = false
#endif

#if ENABLED(HOST_KEEPALIVE_FEATURE)
//MarlinBusyState GcodeSuite::busy_state = NOT_BUSY;
float GcodeSuite::host_keepalive_interval = DEFAULT_KEEPALIVE_INTERVAL;
#endif

#if ENABLED(CNC_WORKSPACE_PLANES)
GcodeSuite::WorkspacePlane GcodeSuite::workspace_plane = PLANE_XY;
#endif

#if ENABLED(CNC_COORDINATE_SYSTEMS)
// int8_t GcodeSuite::active_coordinate_system = 0; // G54
// xyz_pos_t GcodeSuite::coordinate_system[MAX_COORDINATE_SYSTEMS];
#endif

#if HAS_TOOL_OFFSET
tool_record_t GcodeSuite::tool_table[MAX_TOOL_OFFSETS];
#endif

/**
 * Get the target extruder from the T parameter or the active_extruder
 * Return -1 if the T parameter is out of range
 */
int8_t GcodeSuite::get_target_extruder_from_command() {
	if (parser.seenval('T')) {
		const int8_t e = parser.value_byte();
		if (e < EXTRUDERS)
			return e;
		SERIAL_ECHO_START();
		SERIAL_CHAR('M');
		SERIAL_ECHO(parser.codenum);
		SERIAL_ECHOLNPAIR(" " STR_INVALID_EXTRUDER " ", int(e));
		return -1;
	}
	return active_extruder;
}

/**
 * Get the target e stepper from the T parameter
 * Return -1 if the T parameter is out of range or unspecified
 */
int8_t GcodeSuite::get_target_e_stepper_from_command() {
	const int8_t e = parser.intval('T', -1);
	if (WITHIN(e, 0, E_STEPPERS - 1))
		return e;

	SERIAL_ECHO_START();
	SERIAL_CHAR('M');
	SERIAL_ECHO(parser.codenum);
	if (e == -1)
		SERIAL_ECHOLNPGM(" " STR_E_STEPPER_NOT_SPECIFIED);
	else
		SERIAL_ECHOLNPAIR(" " STR_INVALID_E_STEPPER " ", int(e));
	return -1;
}

/**
 * Set XYZE destination and feedrate from the current GCode command
 *
 *  - Set destination from included axis codes
 *  - Set to current for missing axis codes
 *  - Set the feedrate, if included
 */
void GcodeSuite::get_destination_from_command(bool rapid_move) {
	auto& motionModule = MotionModule::getInstance();

	Vector6u8 seen = { false, false, false, false, false, false };

#if ENABLED(CANCEL_OBJECTS)
	const bool& skip_move = cancelable.skipping;
#else
	constexpr bool skip_move = false;
#endif

	u8 linear_moves = 0;
	u8 radial_moves = 0;

	debug()("rapid_move: ", rapid_move);

	// validate move
	for (auto axis : all_axes) {
		if (parser.seen(axis.to_char())) {
			if (axis.is_linear()) {
				linear_moves ++;
			} else {
				radial_moves ++;
			}
		}
	}

	debug()("linear_moves: ", linear_moves, ", radial_moves: ", radial_moves);

	if (!rapid_move && linear_moves > 0 && radial_moves > 0 && parser.feedrate_type != FeedRateType::InverseTime) {
		throw CommandException("Linear and radial moves can only be combined in G93 mode");
	}

	if (parser.feedrate_type == FeedRateType::InverseTime && !parser.seen('F')) {
		throw CommandException("Expected F parameter while in G93 mode.");
	}

	// Get new XYZ position, whether absolute or relative
	for (auto axis : all_axes) {

		if ((seen[axis] = parser.seenval(axis.to_char()))) {
			const float v = parser.value_axis_units(axis);

			debug()("axis: ", (usize)axis, " (", axis.to_char(), "), ", v);

			if (skip_move)
				destination[axis] = current_position[axis];
			else
				destination[axis] = axis_is_relative(axis) ? current_position[axis] + v : motionModule.toNative(axis, v);
		} else
			destination[axis] = current_position[axis];
	}

	if (parser.linearval('F') > 0)
		feedrate_mm_s = parser.value_feedrate();

// Get ABCDHI mixing factors
#if BOTH(MIXING_EXTRUDER, DIRECT_MIXING_IN_G1)
	M165();
#endif

#if ENABLED(LASER_MOVE_POWER)
	// Set the laser power in the planner to configure this move
	if (parser.seen('S')) {
		const float spwr = parser.value_float();
		cutter.inline_power(TERN(SPINDLE_LASER_PWM, cutter.power_to_range(cutter_power_t(round(spwr))), spwr > 0 ? 255 : 0));
	} else if (ENABLED(LASER_MOVE_G0_OFF) && parser.codenum == 0) // G0
		cutter.set_inline_enabled(false);
#endif
}

/**
 * Dwell waits immediately. It does not synchronize. Use M400 instead of G4
 */
void GcodeSuite::dwell(millis_t time) {
	time += millis();
	while (PENDING(millis(), time))
		idle();
}

/**
 * When G29_RETRY_AND_RECOVER is enabled, call G29() in
 * a loop with recovery and retry handling.
 */
#if BOTH(HAS_LEVELING, G29_RETRY_AND_RECOVER)

#	ifndef G29_MAX_RETRIES
#		define G29_MAX_RETRIES 0
#	endif

void GcodeSuite::G29_with_retry() {
	uint8_t retries = G29_MAX_RETRIES;
	while (G29()) { // G29 should return true for failed probes ONLY
		if (retries--)
			event_probe_recover();
		else {
			event_probe_failure();
			return;
		}
	}

	TERN_(HOST_PROMPT_SUPPORT, host_action_prompt_end());

#	ifdef G29_SUCCESS_COMMANDS
	process_subcommands_now_P(PSTR(G29_SUCCESS_COMMANDS));
#	endif
}

#endif // HAS_LEVELING && G29_RETRY_AND_RECOVER

//
// Placeholders for non-migrated codes
//
#if ENABLED(M100_FREE_MEMORY_WATCHER)
extern void M100_dump_routine(PGM_P const title, const char* const start, const char* const end);
#endif

/**
 * Process the parsed command and dispatch it to its handler
 */
void GcodeSuite::process_parsed_command(const bool no_ok /*=false*/) {
/**
 * Block all Gcodes except M511 Unlock Printer, if printer is locked
 * Will still block Gcodes if M511 is disabled, in which case the printer should be unlocked via LCD Menu
 */
#if ENABLED(PASSWORD_FEATURE)
	if (password.is_locked && !parser.is_command('M', 511)) {
		SERIAL_ECHO_MSG(STR_PRINTER_LOCKED);
		if (!no_ok)
			queue.ok_to_send();
		return;
	}
#endif

#if ENABLED(EMERGENCY_PARSER)
	if (emergency_parser.is_enabled()) {
		if (parser.command_letter == 'M') {
			switch (parser.codenum) {
				case 114: {
					if (!no_ok) {
						queue.ok_to_send();
					}

					return;
				}
			}
		}
	}
#endif

#if HAS_ESTOP
	GcodeSuite::aborted_ = false;

	if (!EStopModule::getInstance().checkOrClear()) {
		SERIAL_ECHO_MSG(STR_ESTOP_ENGAGED);

		if (!no_ok) {
			queue.ok_to_send();
		}

		return;
	}
#endif

	//KEEPALIVE_STATE(IN_HANDLER);

	//auto _ = keepalive_state(IN_HANDLER);

	auto& out = Console::out();

	auto writeResult = [&](std::function<void(Writer & out)> write) {
		if (!no_ok) {
			out << "ok";

			if (parser.id_string.size() > 0) {
				out << '#' << parser.id_string;
			}

			if (write) {
				out << ':';

				write(out);
			}

			out << '\n';
		}
	};

	try {
		// Handle a known G, M, or T
		switch (parser.command_letter) {
			case 'G':
				switch (parser.codenum) {

					case 0:
						G0_G1(true);
						break;

					case 1: // G0: Fast Move, G1: Linear Move
						G0_G1(false);
						break;

#if ENABLED(ARC_SUPPORT) && DISABLED(SCARA)
					case 2:
					case 3:
						G2_G3(parser.codenum == 2);
						break; // G2: CW ARC, G3: CCW ARC
#endif

					case 4:
						G4();
						break; // G4: Dwell

#if ENABLED(BEZIER_CURVE_SUPPORT)
					case 5:
						G5();
						break; // G5: Cubic B_spline
#endif

#if ENABLED(DIRECT_STEPPING)
					case 6:
						G6();
						break; // G6: Direct Stepper Move
#endif

#if ENABLED(FWRETRACT)
					case 10:
						G10();
						break; // G10: Retract / Swap Retract
					case 11:
						G11();
						break; // G11: Recover / Swap Recover
#endif

#if ENABLED(NOZZLE_CLEAN_FEATURE)
					case 12:
						G12();
						break; // G12: Nozzle Clean
#endif

#if ENABLED(CNC_WORKSPACE_PLANES)
					case 17:
						G17();
						break; // G17: Select Plane XY
					case 18:
						G18();
						break; // G18: Select Plane ZX
					case 19:
						G19();
						break; // G19: Select Plane YZ
#endif

#if ENABLED(INCH_MODE_SUPPORT)
					case 20:
						G20();
						break; // G20: Inch Mode
					case 21:
						G21();
						break; // G21: MM Mode
#else
					case 21:
						NOOP;
						break; // No error on unknown G21
#endif

#if ENABLED(G26_MESH_VALIDATION)
					case 26:
						G26();
						break; // G26: Mesh Validation Pattern generation
#endif

#if ENABLED(NOZZLE_PARK_FEATURE)
					case 27:
						G27();
						break; // G27: Nozzle Park
#endif

					case 28:
						G28(parser.subcode);
						break; // G28: Home one or more axes

#if HAS_LEVELING
					case 29: // G29: Bed leveling calibration
						TERN(G29_RETRY_AND_RECOVER, G29_with_retry, G29)
						();
						break;
#endif

#if HAS_BED_PROBE
					case 30:
						G30();
						break; // G30: Single Z probe
#	if ENABLED(Z_PROBE_SLED)
					case 31:
						G31();
						break; // G31: dock the sled
					case 32:
						G32();
						break; // G32: undock the sled
#	endif
#endif

#if ENABLED(DELTA_AUTO_CALIBRATION)
					case 33:
						G33();
						break; // G33: Delta Auto-Calibration
#endif

#if ANY(Z_MULTI_ENDSTOPS, Z_STEPPER_AUTO_ALIGN, MECHANICAL_GANTRY_CALIBRATION)
					case 34:
						G34();
						break; // G34: Z Stepper automatic alignment using probe
#endif

#if ENABLED(ASSISTED_TRAMMING)
					case 35:
						G35();
						break; // G35: Read four bed corners to help adjust bed screws
#endif

#if HAS_TOOL_PROBE
					case 37:
						G37();
						break;
#endif

#if ENABLED(G38_PROBE_TARGET)
					case 38: // G38.2, G38.3: Probe towards target
						if (WITHIN(parser.subcode, 2, TERN(G38_PROBE_AWAY, 5, 3)))
							G38(parser.subcode); // G38.4, G38.5: Probe away from target
						break;
#endif

#if ENABLED(CNC_COORDINATE_SYSTEMS)
					case 53:
						G53();
						break; // G53: (prefix) Apply native workspace
					case 54:
						G54();
						break; // G54: Switch to Workspace 1
					case 55:
						G55();
						break; // G55: Switch to Workspace 2
					case 56:
						G56();
						break; // G56: Switch to Workspace 3
					case 57:
						G57();
						break; // G57: Switch to Workspace 4
					case 58:
						G58();
						break; // G58: Switch to Workspace 5
					case 59:
						G59();
						break; // G59.0 - G59.3: Switch to Workspace 6-9
					case 10:
						G10();
						break; // G10: Retract / Swap Retract
					case 11:
						G11();
						break;
#endif

#if HAS_TOOL_OFFSET
					case 43:
						G43();
						break;
					case 49:
						G49();
						break;
#endif

#if SAVED_POSITIONS
					case 60:
						G60();
						break; // G60:  save current position
					case 61:
						G61();
						break; // G61:  Apply/restore saved coordinates.
#endif

#if ENABLED(PROBE_TEMP_COMPENSATION)
					case 76:
						G76();
						break; // G76: Calibrate first layer compensation values
#endif

#if ENABLED(GCODE_MOTION_MODES)
					case 80:
						G80();
						break; // G80: Reset the current motion mode
#endif

					case 90: { // G90: Absolute Mode
						uint8_t mode = relative_mode();

						set_relative_mode(false);

						if (parser.chain()) { // Command to chain?
							process_parsed_command(true);

							restore_relative_mode(mode);
						}

						break;
					}

					case 91: { // G91: Relative Mode
						uint8_t mode = relative_mode();

						set_relative_mode(true);

						if (parser.chain()) {
							process_parsed_command(true);

							restore_relative_mode(mode);
						}

						break;
					}

					case 92: {
						G92();
						break; // G92: Set current axis position(s)
					}

					case 93: {
						G93(); // set feed rate mode to inverse time

						break;
					}

					case 94: {
						G94(); // set feed rate mode to mm/s

						break;
					}

#if HAS_MESH
					case 42:
						G42();
						break; // G42: Coordinated move to a mesh point
#endif

#if ENABLED(CALIBRATION_GCODE)
					case 425:
						G425();
						break; // G425: Perform calibration with calibration cube
#endif

#if ENABLED(DEBUG_GCODE_PARSER)
					case 800:
						parser.debug();
						break; // G800: GCode Parser Test for G
#endif

					default:
						parser.unknown_command_warning();
						break;
				}
				break;

			case 'M':
				switch (parser.codenum) {
					case 0: // M0: Unconditional stop - Wait for user button press on LCD
					case 1:
						M0_M1();
						break; // M1: Conditional stop - Wait for user button press on LCD

					case 3:
						M3_M4(false);
						break; // M3: Turn ON Laser | Spindle (clockwise), set Power | Speed
					case 4:
						M3_M4(true);
						break; // M4: Turn ON Laser | Spindle (counter-clockwise), set Power | Speed
					case 5:
						M5();
						break; // M5: Turn OFF Laser | Spindle

					case 6:
						M6();
						break;

					case 7:
						M7();
						break; // M7: Mist coolant ON
						/*
						          case 8:
						            M8();
						            break; // M8: Flood coolant ON
						*/
					case 9:
						M9();
						break; // M9: Coolant OFF
					case 12:
						M12();
						break;
					case 13:
						M13();
						break;

					case 17:
						M17();
						break; // M17: Enable all stepper motors

					case 20:
						M20();
						break; // M20: List SD card
					case 21:
						M21();
						break; // M21: Init SD card
					case 22:
						M22();
						break; // M22: Release SD card
					case 23:
						M23();
						break; // M23: Select file
					case 24:
						M24();
						break; // M24: Start SD print
					case 25:
						M25();
						break; // M25: Pause SD print
					case 26:
						M26();
						break; // M26: Set SD index
					case 27:
						M27();
						break; // M27: Get SD status
					case 28:
						M28();
						break; // M28: Start SD write
					case 29:
						M29();
						break; // M29: Stop SD write
					case 30:
						M30();
						break; // M30 <filename> Delete File

					case 32:
						M32();
						break; // M32: Select file and start SD print

					case 33:
						M33();
						break; // M33: Get the long full path to a file or folder

					case 34:
						M34();
						break; // M34: Set SD card sorting options

					case 928:
						M928();
						break; // M928: Start SD write

					case 31:
						M31();
						break; // M31: Report time since the start of SD print or last M109

#if ENABLED(DIRECT_PIN_CONTROL)
					case 42:
						M42();
						break; // M42: Change pin state
#endif

					case 43:
						M43();
						break; // M43: Read pin state

					case 75:
						M75();
						break; // M75: Start print timer
					case 76:
						M76();
						break; // M76: Pause print timer
					case 77:
						M77();
						break; // M77: Stop print timer

#if ENABLED(PRINTCOUNTER)
					case 78:
						M78();
						break; // M78: Show print statistics
#endif

#if ENABLED(M100_FREE_MEMORY_WATCHER)
					case 100:
						M100();
						break; // M100: Free Memory Report
#endif

					case 110:
						M110();
						break; // M110: Set Current Line Number
					case 111:
						M111();
						break; // M111: Set debug level

					case 108:
					case 112:
					case 410:
						TERN_(HOST_PROMPT_SUPPORT, case 876
						      :)
						break;

					case 113:
						M113();
						break; // M113: Set Host Keepalive interval

#if ENABLED(PSU_CONTROL)
					case 80:
						M80();
						break; // M80: Turn on Power Supply
#endif

					case 81:
						M81();
						break; // M81: Turn off Power, including Power Supply, if possible

					case 82:
						M82();
						break; // M82: Unlock ATC
					case 18:
					case 84:
						M18_M84();
						break; // M18/M84: Disable Steppers / Set Timeout
					case 85:
						M85();
						break; // M85: Set inactivity stepper shutdown timeout
					case 86:
						M86();
						break; // M86: Lock ATC
					case 92:
						M92();
						break; // M92: Set the steps-per-unit for one or more axes
					case 114:
						M114();
						break; // M114: Report current position
					case 115:
						M115();
						break; // M115: Report capabilities
					case 117:
						M117();
						break; // M117: Section name
					case 118:
						M118();
						break; // M118: Display a message in the host console
					case 119:
						M119();
						break; // M119: Report endstop states
					case 120:
						M120();
						break; // M120: Enable endstops
					case 121:
						M121();
						break; // M121: Disable endstops

					case 201:
						M201();
						break; // M201: Set max acceleration for print moves (units/s^2)

					case 203:
						M203();
						break; // M203: Set max feedrate (units/sec)
					case 204:
						M204();
						break; // M204: Set acceleration
					case 205:
						M205();
						break; // M205: Set advanced settings

#if HAS_M206_COMMAND
					case 206:
						M206();
						break; // M206: Set home offsets
#endif

#if HAS_SOFTWARE_ENDSTOPS
					case 211:
						M211();
						break; // M211: Enable, Disable, and/or Report software endstops
					case 212:
						M212();
						break;
					case 213:
						M213();
						break;
#endif

					case 220:
						M220();
						break; // M220: Set Feedrate Percentage: S<percent> ("FR" on your LCD)

#if HAS_FAST_MOVES
					case 222:
						M222();
						break;
#endif

#if ENABLED(DIRECT_PIN_CONTROL)
					case 226:
						M226();
						break; // M226: Wait until a pin reaches a state
#endif

					case 400:
						M400();
						break; // M400: Finish all moves

#if ENABLED(BACKLASH_GCODE)
					case 425:
						M425();
						break; // M425: Tune backlash compensation
#endif

					case 428:
						M428();
						break; // M428: Apply current_position to home_offset

					case 500:
						M500();
						break; // M500: Store settings in EEPROM
					case 501:
						M501();
						break; // M501: Read settings from EEPROM
					case 502:
						M502();
						break; // M502: Revert to default settings
					case 503:
						M503();
						break; // M503: print settings currently in memory
					case 504:
						M504();
						break; // M504: Validate EEPROM contents

#if ENABLED(SDSUPPORT)
					case 524:
						M524();
						break; // M524: Abort the current SD print job
#endif

#if ENABLED(SD_ABORT_ON_ENDSTOP_HIT)
					case 540:
						M540();
						break; // M540: Set abort on endstop hit for SD printing
#endif

#if HAS_ETHERNET
					case 552:
						M552();
						break; // M552: Set IP address
					case 553:
						M553();
						break; // M553: Set gateway
					case 554:
						M554();
						break; // M554: Set netmask
#endif

#if ENABLED(BAUD_RATE_GCODE)
					case 575:
						M575();
						break; // M575: Set serial baudrate
#endif

#if ENABLED(ADVANCED_PAUSE_FEATURE)
					case 600:
						M600();
						break; // M600: Pause for Filament Change
					case 603:
						M603();
						break; // M603: Configure Filament Change
#endif

					case 666:
						M666();
						break; // M666: Set delta or multiple endstop adjustment
					case 810:
					case 811:
					case 812:
					case 813:
					case 814:
					case 815:
					case 816:
					case 817:
					case 818:
					case 819:
						M810_819();
						break; // M810-M819: Define/execute G-code macro

#if ENABLED(DEBUG_GCODE_PARSER)
					case 800:
						parser.debug();
						break; // M800: GCode Parser Test for M
#endif

#if ENABLED(GCODE_REPEAT_MARKERS)
					case 808:
						M808();
						break; // M808: Set / Goto repeat markers
#endif

#if ALL(HAS_SPI_FLASH, SDSUPPORT, MARLIN_DEV_MODE)
					case 993:
						M993();
						break; // M993: Backup SPI Flash to SD
					case 994:
						M994();
						break; // M994: Load a Backup from SD to SPI Flash
#endif

#if ENABLED(PLATFORM_M997_SUPPORT)
					case 997:
						M997();
						break; // M997: Perform in-application firmware update
#endif

					case 999:
						M999();
						break; // M999: Restart after being Stopped

#if ENABLED(POWER_LOSS_RECOVERY)
					case 413:
						M413();
						break; // M413: Enable/disable/query Power-Loss Recovery
					case 1000:
						M1000();
						break; // M1000: [INTERNAL] Resume from power-loss
#endif

#if ENABLED(SDSUPPORT)
					case 1001:
						M1001();
						break; // M1001: [INTERNAL] Handle SD completion
#endif

					case 2000:
						M2000(writeResult);

						return;

					case 2001: {
						M2001(writeResult);

						return;
					}

					default:
						parser.unknown_command_warning();
						break;
				}
				break;

			case 'T':
				T(parser.codenum);
				break; // Tn: Tool Change

#if ENABLED(MARLIN_DEV_MODE)
			case 'D':
				D(parser.codenum);
				break; // Dn: Debug codes
#endif

			default:
#if ENABLED(WIFI_CUSTOM_COMMAND)
				if (wifi_custom_command(parser.command_ptr))
					break;
#endif
				parser.unknown_command_warning();
		}
	} catch (const Exception& e) {
		if (!no_ok) {
			queue.error_to_send(e);
		}

		throw;
	}

	if (!no_ok)
		queue.ok_to_send();
}

/**
 * Process a single command and dispatch it to its handler
 * This is called from the main loop()
 */
void GcodeSuite::process_next_command() {
	char* const current_command = queue.command_buffer[queue.index_r];

	PORT_REDIRECT(queue.port[queue.index_r]);

#if ENABLED(POWER_LOSS_RECOVERY)
	recovery.queue_index_r = queue.index_r;
#endif

	if (DEBUGGING(ECHO)) {
		SERIAL_ECHO_START();
		SERIAL_ECHOLN(current_command);
#if ENABLED(M100_FREE_MEMORY_DUMPER)
		SERIAL_ECHOPAIR("slot:", queue.index_r);
		M100_dump_routine(PSTR("   Command Queue:"), &queue.command_buffer[0][0], &queue.command_buffer[BUFSIZE - 1][MAX_CMD_SIZE - 1]);
#endif
	}

	// Parse the next command in the queue
	parser.parse(current_command);
	process_parsed_command();
}

/**
 * Run a series of commands, bypassing the command queue to allow
 * G-code "macros" to be called from within other G-code handlers.
 */

void GcodeSuite::process_subcommands_now_P(const char* pgcode) {
	char* const saved_cmd = parser.command_ptr; // Save the parser state
	for (;;) {
		const char* delim = strchr(pgcode, '\n'); // Get address of next newline
		const size_t len = delim ? delim - pgcode : strlen(pgcode); // Get the command length
		char cmd[len + 1]; // Allocate a stack buffer
		memcpy(cmd, pgcode, len); // Copy the command to the stack
		cmd[len] = '\0'; // End with a nul
		parser.parse(cmd); // Parse the command
		process_parsed_command(true); // Process it
		if (!delim)
			break; // Last command?
		pgcode = delim + 1; // Get the next command
	}
	parser.parse(saved_cmd); // Restore the parser state
}

void GcodeSuite::process_subcommands_now(char* gcode) {
	char* const saved_cmd = parser.command_ptr; // Save the parser state
	for (;;) {
		char* const delim = strchr(gcode, '\n'); // Get address of next newline
		if (delim)
			*delim = '\0'; // Replace with null
		parser.parse(gcode); // Parse the current command
		if (delim)
			*delim = '\n'; // Put back the newline
		process_parsed_command(true); // Process it
		if (!delim)
			break; // Last command?
		gcode = delim + 1; // Get the next command
	}
	parser.parse(saved_cmd); // Restore the parser state
}

const char* GcodeSuite::get_state() {
	auto state = StatusModule::getInstance().peek_state();

	switch (state) {
		case MachineState::Idle: {
			return "idle";
		}

		case MachineState::EmergencyStop: {
			return "estop";
		}

		case MachineState::FeedMove: {
			return "busy";
		}

		case MachineState::RapidMove: {
			return "busy";
		}

		case MachineState::Homing: {
			return "homing";
		}

		case MachineState::AwaitingInput: {
			return "waiting";
		}

		case MachineState::SpindleRamp: {
			return "spindle:ramping";
		}

		case MachineState::Probing: {
			return "probing";
		}

		default: {
			return "idle";
		}
	}
}

inline void report_axis(Axis axis, const Vector6f32& pos) {
	SERIAL_PRINTF("\"%c\":%lf", tolower(axis.to_char()), isnan(pos[axis]) ? 0.0 : pos[axis]);
}

inline void report_position_json(const Vector6f32& pos) {
	bool first = true;

	SERIAL_ECHO("{");

	for (auto axis : all_axes) {
		if (!first) {
			SERIAL_ECHO(",");
		}

		first = false;

		report_axis(axis, pos);
	}

	SERIAL_ECHO("}");
}

extern int16_t rapidrate_percentage;

void GcodeSuite::report_state() {
	// get_cartesian_from_steppers();

	auto pos = current_position;
	// xyz_pos_t pos = cartes;

	const char* state = get_state();
	auto& toolManager = ToolsModule::getInstance();
	auto& motionManager = MotionModule::getInstance();
	auto& driver = toolManager.getCurrentDriver();

	SERIAL_ECHO("{\"state\":\"");
	SERIAL_ECHO(state);
	SERIAL_ECHO("\",\"mpos\":");
	report_position_json(pos);
	SERIAL_ECHO(",\"wpos\":");
	report_position_json(toLogical(pos));
	// SERIAL_ECHO(",\"spos\":");
	// SERIAL_PRINTF("{\"x\":%lf,\"y\":%lf,\"z\":%lf}", Stepper::count_position.x, Stepper::count_position.y, Stepper::count_position.z);
	SERIAL_PRINTF(",\"wcs\":%ld", motionManager.getActiveCoordinateSystem().getIndex() + 1);
	SERIAL_PRINTF(",\"ovR\":%ld", rapidrate_percentage);
	SERIAL_PRINTF(",\"ovF\":%ld", feedrate_percentage);
	SERIAL_PRINTF(",\"ovS\":%ld", (uint32_t) driver.getPowerOverride());
	SERIAL_PRINTF(",\"spindle\":{\"freq\":%d,\"rpm\":%d,\"dir\":%d}", driver.getOutputFrequency(), 0/*driver.getCurrentPower()*/, driver.getCurrentDirection());
	SERIAL_PRINTF(",\"tool\":%ld", toolManager.getCurrentToolIndex() + 1);
	SERIAL_PRINTF(",\"ntool\":%ld", toolManager.getNextToolIndex() + 1);
	SERIAL_ECHO("}\n");
}

#if ENABLED(HOST_KEEPALIVE_FEATURE)

/**
 * Output a "busy" message at regular intervals
 * while the machine is not accepting commands.
 */
void GcodeSuite::host_keepalive() {
	const millis_t ms = millis();
	static millis_t next_keepalive = 0;

	if (!autoreport_paused && host_keepalive_interval) {
		if (PENDING(ms, next_keepalive)) {
			return;
		}

		report_state();
	}

	next_keepalive = ms + SEC_TO_MS(host_keepalive_interval);
}

#endif // HOST_KEEPALIVE_FEATURE
