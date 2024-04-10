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
 * parser.h - Parser for a GCode line, providing a parameter interface.
 *           Codes like M149 control the way the GCode parser behaves,
 *           so settings for these codes are located in this class.
 */

#include "../inc/MarlinConfig.h"

#include "../core/serial.h"
#include <string_view>
#include <stdlib.h>

#include <swordfish/types.h>
#include <swordfish/modules/motion/FeedRate.h>

#if ENABLED(DEBUG_GCODE_PARSER)
#	include "../libs/hex_print.h"
#endif

#if ENABLED(TEMPERATURE_UNITS_SUPPORT)
typedef enum : uint8_t { TEMPUNIT_C,
	                       TEMPUNIT_K,
	                       TEMPUNIT_F } TempUnit;
#endif

#if ENABLED(INCH_MODE_SUPPORT)
typedef enum : uint8_t { LINEARUNIT_MM,
	                       LINEARUNIT_INCH } LinearUnit;
#endif

/**
 * GCode parser
 *
 *  - Parse a single gcode line for its letter, code, subcode, and parameters
 *  - FASTER_GCODE_PARSER:
 *    - Flags existing params (1 bit each)
 *    - Stores value offsets (1 byte each)
 *  - Provide accessors for parameters:
 *    - Parameter exists
 *    - Parameter has value
 *    - Parameter value in different units and types
 */
class GCodeParser {

private:
	static char* value_ptr; // Set by seen, used to fetch the value

#if ENABLED(FASTER_GCODE_PARSER)
	static uint32_t codebits; // Parameters pre-scanned
	static uint8_t param[26]; // For A-Z, offsets into command args
#else
	static char* command_args; // Args start here, for slow scan
#endif

public:
	// Global states for GCode-level units features

	static f32 linear_unit_factor;
	static f32 radial_unit_factor;
	static swordfish::motion::FeedRateType feedrate_type;

	// Command line state
	static char* command_ptr; // The command, so it can be echoed
	static char* string_arg; // string of command line
	static char command_letter; // G, M, or T

	static std::string_view selector_string;
	static std::string_view parameter_string;
	static std::string_view id_string;

	static uint16_t codenum; // 123
#if ENABLED(USE_GCODE_SUBCODES)
	static uint8_t subcode; // .1
#endif

#if ENABLED(GCODE_MOTION_MODES)
	static int16_t motion_mode_codenum;
#	if ENABLED(USE_GCODE_SUBCODES)
	static uint8_t motion_mode_subcode;
#	endif
	FORCE_INLINE static void cancel_motion_mode() {
		motion_mode_codenum = -1;
	}
#endif

#if ENABLED(DEBUG_GCODE_PARSER)
	static void debug();
#endif

	// Reset is done before parsing
	static void reset();

#define LETTER_BIT(N) ((N) - 'A')

	FORCE_INLINE static bool valid_signless(const char* const p) {
		return NUMERIC(p[0]) || (p[0] == '.' && NUMERIC(p[1])); // .?[0-9]
	}

	FORCE_INLINE static bool valid_float(const char* const p) {
		return valid_signless(p) || ((p[0] == '-' || p[0] == '+') && valid_signless(&p[1])); // [-+]?.?[0-9]
	}

	FORCE_INLINE static bool valid_hex(const char* const p) {
		return p[0] == 'x';
	}

#if ENABLED(FASTER_GCODE_PARSER)

	FORCE_INLINE static bool valid_int(const char* const p) {
		return NUMERIC(p[0]) || ((p[0] == '-' || p[0] == '+') && NUMERIC(p[1])); // [-+]?[0-9]
	}

	// Set the flag and pointer for a parameter
	static inline void set(const char c, char* const ptr) {
		const uint8_t ind = LETTER_BIT(c);
		if (ind >= COUNT(param))
			return; // Only A-Z
		SBI32(codebits, ind); // parameter exists
		param[ind] = ptr ? ptr - command_ptr : 0; // parameter offset or 0
#	if ENABLED(DEBUG_GCODE_PARSER)
		if (codenum == 800) {
			SERIAL_ECHOPAIR("Set bit ", (int) ind, " of codebits (", hex_address((void*) (codebits >> 16)));
			print_hex_word((uint16_t) (codebits & 0xFFFF));
			SERIAL_ECHOLNPAIR(") | param = ", (int) param[ind]);
		}
#	endif
	}

	// Code seen bit was set. If not found, value_ptr is unchanged.
	// This allows "if (seen('A')||seen('B'))" to use the last-found value.
	static inline bool seen(const char c) {
		const uint8_t ind = LETTER_BIT(c);
		if (ind >= COUNT(param))
			return false; // Only A-Z
		const bool b = TEST32(codebits, ind);
		if (b) {
			if (param[ind]) {
				char* ptr = command_ptr + param[ind];
				// value_ptr = valid_number(ptr) ? ptr : nullptr;
				value_ptr = ptr;
			} else
				value_ptr = nullptr;
		}
		return b;
	}

	FORCE_INLINE static constexpr uint32_t letter_bits(const char* const str) {
		return (str[0] ? _BV32(LETTER_BIT(str[0])) |
		                     (str[1] ? _BV32(LETTER_BIT(str[1])) |
		                                   (str[2] ? _BV32(LETTER_BIT(str[2])) |
		                                                 (str[3] ? _BV32(LETTER_BIT(str[3])) |
		                                                               (str[4] ? _BV32(LETTER_BIT(str[4])) |
		                                                                             (str[5] ? _BV32(LETTER_BIT(str[5])) |
		                                                                                           (str[6] ? _BV32(LETTER_BIT(str[6])) |
		                                                                                                         (str[7] ? _BV32(LETTER_BIT(str[7])) |
		                                                                                                                       (str[8] ? _BV32(LETTER_BIT(str[8])) |
		                                                                                                                                     (str[9] ? _BV32(LETTER_BIT(str[9]))
		                                                                                                                                             : 0)
		                                                                                                                               : 0)
		                                                                                                                 : 0)
		                                                                                                   : 0)
		                                                                                     : 0)
		                                                                       : 0)
		                                                         : 0)
		                                           : 0)
		                             : 0)
		               : 0);
	}

// At least one of a list of code letters was seen
#	ifdef CPU_32_BIT
	FORCE_INLINE static bool seen(const char* const str) {
		return !!(codebits & letter_bits(str));
	}
#	else
	FORCE_INLINE static bool seen(const char* const str) {
		const uint32_t letrbits = letter_bits(str);
		const uint8_t* const cb = (uint8_t*) &codebits;
		const uint8_t* const lb = (uint8_t*) &letrbits;
		return (cb[0] & lb[0]) || (cb[1] & lb[1]) || (cb[2] & lb[2]) || (cb[3] & lb[3]);
	}
#	endif

	static inline bool seen_any() {
		return !!codebits;
	}

	FORCE_INLINE static bool seen_test(const char c) {
		return TEST32(codebits, LETTER_BIT(c));
	}

#else // !FASTER_GCODE_PARSER

#	if ENABLED(GCODE_CASE_INSENSITIVE)
	FORCE_INLINE static char* strgchr(char* p, char g) {
		auto uppercase = [](char c) {
			return c + (WITHIN(c, 'a', 'z') ? 'A' - 'a' : 0);
		};
		const char d = uppercase(g);
		for (char cc; (cc = uppercase(*p)); p++)
			if (cc == d)
				return p;
		return nullptr;
	}
#	else
#		define strgchr strchr
#	endif

	// Code is found in the string. If not found, value_ptr is unchanged.
	// This allows "if (seen('A')||seen('B'))" to use the last-found value.
	static inline bool seen(const char c) {
		char* p = strgchr(command_args, c);
		const bool b = !!p;
		if (b)
			value_ptr = valid_number(&p[1]) ? &p[1] : nullptr;
		return b;
	}

	static inline bool seen_any() {
		return *command_args == '\0';
	}

	FORCE_INLINE static bool seen_test(const char c) {
		return (bool) strgchr(command_args, c);
	}

	// At least one of a list of code letters was seen
	static inline bool seen(const char* const str) {
		for (uint8_t i = 0; const char c = str[i]; i++)
			if (seen_test(c))
				return true;
		return false;
	}

#endif // !FASTER_GCODE_PARSER

	// Seen any axis parameter
	static inline bool seen_axis() {
		return seen_test('X') || seen_test('Y') || seen_test('Z') || seen_test('E');
	}

#if ENABLED(GCODE_QUOTED_STRINGS)
	static char* unescape_string(char*& src);
#else
	FORCE_INLINE static char* unescape_string(char*& src) {
		return src;
	}
#endif

	// Populate all fields by parsing a single line of GCode
	// This uses 54 bytes of SRAM to speed up seen/value
	static void parse(char* p);

#if ENABLED(CNC_COORDINATE_SYSTEMS)
	// Parse the next parameter as a new command
	static bool chain();
#endif

	// Test whether the parsed command matches the input
	static inline bool is_command(const char ltr, const uint16_t num) {
		return command_letter == ltr && codenum == num;
	}

	// The code value pointer was set
	FORCE_INLINE static bool has_value() {
		return !!value_ptr;
	}

	// Seen a parameter with a value
	static inline bool seenval(const char c) {
		return seen(c) && has_value();
	}

	// The value as a string
	static inline char* value_string() {
		return value_ptr;
	}

	// Float removes 'E' to prevent scientific notation interpretation
	static float value_float() {
		if (value_ptr) {
			char* e = value_ptr;
			for (;;) {
				const char c = *e;
				if (c == '\0' || c == ' ')
					break;
				if (c == 'E' || c == 'e') {
					*e = '\0';
					const float ret = std::strtof(value_ptr, nullptr);
					*e = c;
					return ret;
				}
				++e;
			}
			const float ret = std::strtof(value_ptr, nullptr);

			return ret;
		}
		return 0;
	}

	// Code value as a long or ulong
	static inline int32_t value_long() {
		switch (*value_ptr) {
			case '\0': {
				return 0;
			}

			case 'x': {
				int32_t out = 0;
				for (char* vp = value_ptr + 1; HEXCHR(*vp) >= 0; vp++) {
					out = ((out) << 4) | HEXCHR(*vp);
				}
				return out;
			}

			default: {
				return strtol(value_ptr, nullptr, 10);
			}
		}
	}

	static inline uint32_t value_ulong() {
		switch (*value_ptr) {
			case '\0': {
				return 0;
			}

			case 'x': {
				uint32_t out = 0;
				for (char* vp = value_ptr + 1; HEXCHR(*vp) >= 0; vp++) {
					out = ((out) << 4) | HEXCHR(*vp);
				}
				return out;
			}

			default: {
				return strtoul(value_ptr, nullptr, 10);
			}
		}
	}

	// Code value for use as time
	static inline millis_t value_millis() {
		return value_ulong();
	}
	static inline millis_t value_millis_from_seconds() {
		return (millis_t) (value_float() * 1000);
	}

	// Reduce to fewer bits
	static inline int16_t value_int() {
		return (int16_t) value_long();
	}
	static inline uint16_t value_ushort() {
		return (uint16_t) value_long();
	}
	static inline uint8_t value_byte() {
		return (uint8_t) constrain(value_long(), 0, 255);
	}

	// Bool is true with no value or non-zero
	static inline bool value_bool() {
		return !has_value() || !!value_byte();
	}

	// Units modes: Inches, Fahrenheit, Kelvin

	static inline float mm_to_linear_unit(const float mm) {
		return mm / linear_unit_factor;
	}

	// Init linear units by constructor
	GCodeParser() {
		set_input_linear_units(LINEARUNIT_MM);
		set_feed_rate_type(swordfish::motion::FeedRateType::MillimetersPerSecond);
	}

	static inline void set_input_linear_units(const LinearUnit units) {
		switch (units) {
			default:
			case LINEARUNIT_MM:
				linear_unit_factor = 1.0f;
				break;
			case LINEARUNIT_INCH:
				linear_unit_factor = 25.4f;
				break;
		}
	}

	static inline void set_feed_rate_type(const swordfish::motion::FeedRateType type) {
		feedrate_type = type;
	}

	static inline float axis_unit_factor(const Axis axis) {
		return (axis.is_radial() ? radial_unit_factor : linear_unit_factor);
	}

	static inline float linear_value_to_mm(const f32 v) {
		return v * linear_unit_factor;
	}

	static inline f32 radial_value_to_degrees(const f32 v) {
		return v * radial_unit_factor;
	}

	static inline float axis_value_to_mm(const Axis axis, const f32 v) {
		return v * axis_unit_factor(axis);
	}

	static inline float per_axis_value(const Axis axis, const f32 v) {
		return v / axis_unit_factor(axis);
	}

	static inline bool using_inch_units() {
		return mm_to_linear_unit(1.0f) != 1.0f;
	}

#define IN_TO_MM(I)        ((I) *25.4f)
#define MM_TO_IN(M)        ((M) / 25.4f)

	static inline float value_linear_units() {
		return linear_value_to_mm(value_float());
	}

	static inline float value_radial_units() {
		return radial_value_to_degrees(value_float());
	}

	static inline float value_axis_units(const Axis axis) {
		return axis_value_to_mm(axis, value_float());
	}
	static inline float value_per_axis_units(const Axis axis) {
		return per_axis_value(axis, value_float());
	}



	static inline swordfish::motion::FeedRate value_feedrate() {
		switch (feedrate_type) {
			case swordfish::motion::FeedRateType::InverseTime: {
				return swordfish::motion::FeedRate::InverseTime(value_float());
			}

			default:
			case swordfish::motion::FeedRateType::MillimetersPerSecond: {
				return swordfish::motion::FeedRate::MillimetersPerSecond(MMM_TO_MMS(value_linear_units()));
			}
		}
	}

	void unknown_command_warning();

	// Provide simple value accessors with default option
	static inline char* stringval(const char c, char* const dval = nullptr) {
		return seenval(c) ? value_string() : dval;
	}
	static inline float floatval(const char c, const float dval = 0.0) {
		return seenval(c) ? value_float() : dval;
	}
	static inline bool boolval(const char c, const bool dval = false) {
		return seenval(c) ? value_bool() : (seen(c) ? true : dval);
	}
	static inline uint8_t byteval(const char c, const uint8_t dval = 0) {
		return seenval(c) ? value_byte() : dval;
	}
	static inline int16_t intval(const char c, const int16_t dval = 0) {
		return seenval(c) ? value_int() : dval;
	}
	static inline uint16_t ushortval(const char c, const uint16_t dval = 0) {
		return seenval(c) ? value_ushort() : dval;
	}
	static inline int32_t longval(const char c, const int32_t dval = 0) {
		return seenval(c) ? value_long() : dval;
	}
	static inline uint32_t ulongval(const char c, const uint32_t dval = 0) {
		return seenval(c) ? value_ulong() : dval;
	}
	static inline float linearval(const char c, const float dval = 0) {
		return seenval(c) ? value_linear_units() : dval;
	}

	static inline uint8_t* hex_adr_val(const char c, uint8_t* const dval = nullptr) {
		if (!seen(c) || *value_ptr != 'x')
			return dval;
		uint8_t* out = nullptr;
		for (char* vp = value_ptr + 1; HEXCHR(*vp) >= 0; vp++)
			out = (uint8_t*) ((uintptr_t(out) << 4) | HEXCHR(*vp));
		return out;
	}

	static inline uint16_t hex_val(const char c, uint16_t const dval = 0) {
		if (!seen(c) || *value_ptr != 'x')
			return dval;

		uint16_t out = 0;
		for (char* vp = value_ptr + 1; HEXCHR(*vp) >= 0; vp++)
			out = ((out) << 4) | HEXCHR(*vp);
		return out;
	}
};

extern GCodeParser parser;
