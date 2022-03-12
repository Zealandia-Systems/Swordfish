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

typedef int64_t offset_t;

enum {
	X = 0,
	Y = 1,
	Z = 2
};

static_assert(std::numeric_limits<float32_t>::has_quiet_NaN);
static constexpr float32_t NaN = std::numeric_limits<float32_t>::quiet_NaN();