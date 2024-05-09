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

enum {
	X = 0,
	Y = 1,
	Z = 2
};

static_assert(std::numeric_limits<float32_t>::has_quiet_NaN);
static constexpr float32_t NaN = std::numeric_limits<float32_t>::quiet_NaN();