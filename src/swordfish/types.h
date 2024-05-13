/*
 * types.h
 *
 * Created: 8/08/2021 4:49:56 pm
 *  Author: smohekey
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <limits>

typedef float float32_t;
typedef double float64_t;

typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int64_t offset_t;

enum class AxisValue : u8 {
	X = 0,
	Y = 1,
	Z = 2,
	A = 3,
	B = 4,
	C = 5,
	All = 0xFF
};

enum EndstopValue : u8 {
  X_MIN       = 0,
	Y_MIN       = 1,
	Z_MIN       = 2,
	Z_MIN_PROBE = 3,
	WORK_PROBE  = 4,
	TOOL_PROBE  = 5,
	UNUSED			= 6,
  X_MAX       = 7,
	Y_MAX       = 8,
	Z_MAX       = 9,
  X2_MIN      = 10,
	X2_MAX      = 11,
  Y2_MIN      = 12,
	Y2_MAX      = 13,
  Z2_MIN      = 14,
	Z2_MAX      = 15,
	A_MIN       = 16,
	B_MIN       = 17,
	C_MIN       = 18,
	A_MAX       = 19,
	B_MAX       = 20,
	C_MAX       = 21
};

enum class AxisSelector {
	None = 0,
	X = 1 << (u8)AxisValue::X,
	Y = 1 << (u8)AxisValue::Y,
	Z = 1 << (u8)AxisValue::Z,
	A = 1 << (u8)AxisValue::A,
	B = 1 << (u8)AxisValue::B,
	C = 1 << (u8)AxisValue::C,
	All = X | Y | Z | A | B | C,
	__size__ = 6
};

class Axis {
private:
	AxisValue value_;

	constexpr Axis(AxisValue value) : value_(value) {}

public:
	static constexpr u8 COUNT = 6;

	static constexpr Axis X() {
		return Axis(AxisValue::X);
	}

	static constexpr Axis Y() {
		return Axis(AxisValue::Y);
	}

	static constexpr Axis Z() {
		return Axis(AxisValue::Z);
	}

	static constexpr Axis A() {
		return Axis(AxisValue::A);
	}

	static constexpr Axis B() {
		return Axis(AxisValue::B);
	}

	static constexpr Axis C() {
		return Axis(AxisValue::C);
	}

	constexpr AxisValue value() const {
		return value_;
	}

	constexpr operator usize() const {
		return (usize)value_;
	}

	constexpr const char to_char() const {
		constexpr const char chars[] = {
			'X',
			'Y',
			'Z',
			'A',
			'B',
			'C'
		};

		return chars[*this];
	}

	constexpr const bool is_linear() const {
		constexpr const bool is_linear[] = {
			true,
			true,
			true,
			false,
			false,
			false
		};

		return is_linear[*this];
	}

	constexpr const bool is_radial() const {
		return !is_linear();
	}

	constexpr const u8 bit_value() const {
		return 1 << *this;
	}
};

static constexpr Axis all_axes[] = {
	Axis::X(),
	Axis::Y(),
	Axis::Z(),
	Axis::A(),
	Axis::B(),
	Axis::C()
};

static constexpr Axis linear_axes[] = {
	Axis::X(),
	Axis::Y(),
	Axis::Z()
};

static constexpr Axis radial_axes[] = {
	Axis::A(),
	Axis::B(),
	Axis::C()
};


static_assert(std::numeric_limits<float32_t>::has_quiet_NaN);
static constexpr float32_t NaN = std::numeric_limits<float32_t>::quiet_NaN();