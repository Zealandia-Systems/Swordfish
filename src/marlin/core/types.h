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

#include <Eigen/Core>

#include <math.h>
#include <stddef.h>

#include "../inc/MarlinConfigPre.h"

class __FlashStringHelper;
typedef const __FlashStringHelper* progmem_str;

//
// Enumerated axis indices
//
enum AxisEnum : uint8_t {
	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2,
	A_AXIS = 3,
	// B_AXIS = 4,
	// C_AXIS = 5,
	ALL_AXES = 0xFE,
	NO_AXIS = 0xFF
};

//
// Loop over XYZA axes
//
#define LOOP_XYZ(VAR)    LOOP_S_LE_N(VAR, X_AXIS, Z_AXIS)
#define LOOP_XYZA(VAR)   LOOP_S_LE_N(VAR, X_AXIS, A_AXIS)
#define LOOP_XYZA_N(VAR) LOOP_S_L_N(VAR, X_AXIS, XYZA_N)

//
// Conditional type assignment magic. For example...
//
// typename IF<(MYOPT==12), int, float>::type myvar;
//
template<bool, class L, class R>
struct IF { typedef R type; };
template<class L, class R>
struct IF<true, L, R> { typedef L type; };

//
// feedRate_t is just a humble float
//
typedef float feedRate_t;

// Conversion macros
#define MMM_TO_MMS(MM_M) feedRate_t(float(MM_M) / 60.0f)
#define MMS_TO_MMM(MM_S) (float(MM_S) * 60.0f)

//
// Coordinates structures for XY, XYZ, XYZA...
//

// Helpers
#define _RECIP(N)        ((N) ? 1.0f / float(N) : 0.0f)
#define _ABS(N)          ((N) < 0 ? -(N) : (N))
#define _LS(N)           (N = (T) (uint32_t(N) << v))
#define _RS(N)           (N = (T) (uint32_t(N) >> v))
#define FI               FORCE_INLINE

// Forward declarations
template<typename T>
struct XYval;
template<typename T>
struct XYZval;
template<typename T>
struct XYZAval;

typedef struct XYval<bool> xy_bool_t;
typedef struct XYZval<bool> xyz_bool_t;
typedef struct XYZAval<bool> xyza_bool_t;

typedef struct XYval<char> xy_char_t;
typedef struct XYZval<char> xyz_char_t;
typedef struct XYZAval<char> xyza_char_t;

typedef struct XYval<unsigned char> xy_uchar_t;
typedef struct XYZval<unsigned char> xyz_uchar_t;
typedef struct XYZAval<unsigned char> xyza_uchar_t;

typedef struct XYval<int8_t> xy_int8_t;
typedef struct XYZval<int8_t> xyz_int8_t;
typedef struct XYZAval<int8_t> xyza_int8_t;

typedef struct XYval<uint8_t> xy_uint8_t;
typedef struct XYZval<uint8_t> xyz_uint8_t;
typedef struct XYZAval<uint8_t> xyza_uint8_t;

typedef struct XYval<int16_t> xy_int_t;
typedef struct XYZval<int16_t> xyz_int_t;
typedef struct XYZAval<int16_t> xyza_int_t;

typedef struct XYval<uint16_t> xy_uint_t;
typedef struct XYZval<uint16_t> xyz_uint_t;
typedef struct XYZAval<uint16_t> xyza_uint_t;

typedef struct XYval<int32_t> xy_long_t;
typedef struct XYZval<int32_t> xyz_long_t;
typedef struct XYZAval<int32_t> xyza_long_t;

typedef struct XYval<uint32_t> xy_ulong_t;
typedef struct XYZval<uint32_t> xyz_ulong_t;
typedef struct XYZAval<uint32_t> xyza_ulong_t;

typedef struct XYZval<volatile int32_t> xyz_vlong_t;
typedef struct XYZAval<volatile int32_t> xyza_vlong_t;

typedef struct XYval<float> xy_float_t;
typedef struct XYZval<float> xyz_float_t;
typedef struct XYZAval<float> xyza_float_t;

typedef struct XYval<feedRate_t> xy_feedrate_t;
typedef struct XYZval<feedRate_t> xyz_feedrate_t;
typedef struct XYZAval<feedRate_t> xyza_feedrate_t;

typedef xy_uint8_t xy_byte_t;
typedef xyz_uint8_t xyz_byte_t;
typedef xyza_uint8_t xyza_byte_t;

typedef xyz_long_t abc_long_t;
typedef xyza_long_t abce_long_t;
typedef xyz_ulong_t abc_ulong_t;
typedef xyza_ulong_t abce_ulong_t;

typedef xy_float_t xy_pos_t;
typedef xyz_float_t xyz_pos_t;
typedef xyza_float_t xyza_pos_t;

typedef xy_float_t ab_float_t;
typedef xyz_float_t abc_float_t;
typedef xyza_float_t abce_float_t;

// External conversion methods
void toLogical(xy_pos_t& raw);
void toLogical(xyz_pos_t& raw);
void toLogical(xyza_pos_t& raw);
void toNative(xy_pos_t& raw);
void toNative(xyz_pos_t& raw);
void toNative(xyza_pos_t& raw);

//
// XY coordinates, counters, etc.
//
template<typename T>
struct XYval {
	union {
		struct {
			T x, y;
		};
		T pos[2];
	};
	FI void set(const T px) {
		x = px;
	}
	FI void set(const T px, const T py) {
		x = px;
		y = py;
	}
	FI void set(const T (&arr)[XY_N]) {
		x = arr[0];
		y = arr[1];
	}
	FI void set(const T (&arr)[XYZ_N]) {
		x = arr[0];
		y = arr[1];
	}
	FI void set(const T (&arr)[XYZA_N]) {
		x = arr[0];
		y = arr[1];
	}
	FI void reset() {
		x = y = 0;
	}
	FI T magnitude() const {
		return (T) sqrtf(x * x + y * y);
	}
	FI operator T*() {
		return pos;
	}
	FI operator bool() {
		return x || y;
	}
	FI XYval<T> copy() const {
		return *this;
	}
	FI XYval<T> ABS() const {
		return { T(_ABS(x)), T(_ABS(y)) };
	}
	FI XYval<int16_t> asInt() {
		return { int16_t(x), int16_t(y) };
	}
	FI XYval<int16_t> asInt() const {
		return { int16_t(x), int16_t(y) };
	}
	FI XYval<int32_t> asLong() {
		return { int32_t(x), int32_t(y) };
	}
	FI XYval<int32_t> asLong() const {
		return { int32_t(x), int32_t(y) };
	}
	FI XYval<int32_t> ROUNDL() {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)) };
	}
	FI XYval<int32_t> ROUNDL() const {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)) };
	}
	FI XYval<float> asFloat() {
		return { float(x), float(y) };
	}
	FI XYval<float> asFloat() const {
		return { float(x), float(y) };
	}
	FI XYval<float> reciprocal() const {
		return { _RECIP(x), _RECIP(y) };
	}
	FI XYval<float> asLogical() const {
		XYval<float> o = asFloat();
		toLogical(o);
		return o;
	}
	FI XYval<float> asNative() const {
		XYval<float> o = asFloat();
		toNative(o);
		return o;
	}
	FI operator XYZval<T>() {
		return { x, y };
	}
	FI operator XYZval<T>() const {
		return { x, y };
	}
	FI operator XYZAval<T>() {
		return { x, y };
	}
	FI operator XYZAval<T>() const {
		return { x, y };
	}
	FI T& operator[](const int i) {
		return pos[i];
	}
	FI const T& operator[](const int i) const {
		return pos[i];
	}
	FI XYval<T>& operator=(const T v) {
		set(v, v);
		return *this;
	}
	FI XYval<T>& operator=(const XYZval<T>& rs) {
		set(rs.x, rs.y);
		return *this;
	}
	FI XYval<T>& operator=(const XYZAval<T>& rs) {
		set(rs.x, rs.y);
		return *this;
	}
	FI XYval<T> operator+(const XYval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator+(const XYval<T>& rs) {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYval<T>& rs) {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYval<T>& rs) {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYval<T>& rs) {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator+(const XYZval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator+(const XYZval<T>& rs) {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYZval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYZval<T>& rs) {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYZval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYZval<T>& rs) {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYZval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYZval<T>& rs) {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator+(const XYZAval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator+(const XYZAval<T>& rs) {
		XYval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYZAval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator-(const XYZAval<T>& rs) {
		XYval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYZAval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const XYZAval<T>& rs) {
		XYval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYZAval<T>& rs) const {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator/(const XYZAval<T>& rs) {
		XYval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYval<T> operator*(const float& v) const {
		XYval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		return ls;
	}
	FI XYval<T> operator*(const float& v) {
		XYval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		return ls;
	}
	FI XYval<T> operator*(const int& v) const {
		XYval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		return ls;
	}
	FI XYval<T> operator*(const int& v) {
		XYval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		return ls;
	}
	FI XYval<T> operator/(const float& v) const {
		XYval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		return ls;
	}
	FI XYval<T> operator/(const float& v) {
		XYval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		return ls;
	}
	FI XYval<T> operator/(const int& v) const {
		XYval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		return ls;
	}
	FI XYval<T> operator/(const int& v) {
		XYval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		return ls;
	}
	FI XYval<T> operator>>(const int& v) const {
		XYval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		return ls;
	}
	FI XYval<T> operator>>(const int& v) {
		XYval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		return ls;
	}
	FI XYval<T> operator<<(const int& v) const {
		XYval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		return ls;
	}
	FI XYval<T> operator<<(const int& v) {
		XYval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		return ls;
	}
	FI XYval<T>& operator+=(const XYval<T>& rs) {
		x += rs.x;
		y += rs.y;
		return *this;
	}
	FI XYval<T>& operator-=(const XYval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		return *this;
	}
	FI XYval<T>& operator*=(const XYval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		return *this;
	}
	FI XYval<T>& operator+=(const XYZval<T>& rs) {
		x += rs.x;
		y += rs.y;
		return *this;
	}
	FI XYval<T>& operator-=(const XYZval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		return *this;
	}
	FI XYval<T>& operator*=(const XYZval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		return *this;
	}
	FI XYval<T>& operator+=(const XYZAval<T>& rs) {
		x += rs.x;
		y += rs.y;
		return *this;
	}
	FI XYval<T>& operator-=(const XYZAval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		return *this;
	}
	FI XYval<T>& operator*=(const XYZAval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		return *this;
	}
	FI XYval<T>& operator*=(const float& v) {
		x *= v;
		y *= v;
		return *this;
	}
	FI XYval<T>& operator*=(const int& v) {
		x *= v;
		y *= v;
		return *this;
	}
	FI XYval<T>& operator>>=(const int& v) {
		_RS(x);
		_RS(y);
		return *this;
	}
	FI XYval<T>& operator<<=(const int& v) {
		_LS(x);
		_LS(y);
		return *this;
	}
	FI bool operator==(const XYval<T>& rs) {
		return x == rs.x && y == rs.y;
	}
	FI bool operator==(const XYZval<T>& rs) {
		return x == rs.x && y == rs.y;
	}
	FI bool operator==(const XYZAval<T>& rs) {
		return x == rs.x && y == rs.y;
	}
	FI bool operator==(const XYval<T>& rs) const {
		return x == rs.x && y == rs.y;
	}
	FI bool operator==(const XYZval<T>& rs) const {
		return x == rs.x && y == rs.y;
	}
	FI bool operator==(const XYZAval<T>& rs) const {
		return x == rs.x && y == rs.y;
	}
	FI bool operator!=(const XYval<T>& rs) {
		return !operator==(rs);
	}
	FI bool operator!=(const XYZval<T>& rs) {
		return !operator==(rs);
	}
	FI bool operator!=(const XYZAval<T>& rs) {
		return !operator==(rs);
	}
	FI bool operator!=(const XYval<T>& rs) const {
		return !operator==(rs);
	}
	FI bool operator!=(const XYZval<T>& rs) const {
		return !operator==(rs);
	}
	FI bool operator!=(const XYZAval<T>& rs) const {
		return !operator==(rs);
	}
	FI XYval<T> operator-() {
		XYval<T> o = *this;
		o.x = -x;
		o.y = -y;
		return o;
	}
	FI const XYval<T> operator-() const {
		XYval<T> o = *this;
		o.x = -x;
		o.y = -y;
		return o;
	}
};

//
// XYZ coordinates, counters, etc.
//
template<typename T>
struct XYZval {
	union {
		struct {
			T x, y, z;
		};
		T pos[3];
	};
	FI void set(const T px) {
		x = px;
	}
	FI void set(const T px, const T py) {
		x = px;
		y = py;
	}
	FI void set(const T px, const T py, const T pz) {
		x = px;
		y = py;
		z = pz;
	}
	FI void set(const XYval<T> pxy, const T pz) {
		x = pxy.x;
		y = pxy.y;
		z = pz;
	}
	FI void set(const T (&arr)[XY_N]) {
		x = arr[0];
		y = arr[1];
	}
	FI void set(const T (&arr)[XYZ_N]) {
		x = arr[0];
		y = arr[1];
		z = arr[2];
	}
	FI void set(const T (&arr)[XYZA_N]) {
		x = arr[0];
		y = arr[1];
		z = arr[2];
	}
	FI void reset() {
		x = y = z = 0;
	}
	FI T magnitude() const {
		return (T) sqrtf(x * x + y * y + z * z);
	}
	FI operator T*() {
		return pos;
	}
	FI operator bool() {
		return z || x || y;
	}
	FI operator Eigen::Vector3<T>() {
		return { x, y, z };
	}
	FI XYZval<T> copy() const {
		XYZval<T> o = *this;
		return o;
	}
	FI XYZval<T> ABS() const {
		return { T(_ABS(x)), T(_ABS(y)), T(_ABS(z)) };
	}
	FI XYZval<int16_t> asInt() {
		return { int16_t(x), int16_t(y), int16_t(z) };
	}
	FI XYZval<int16_t> asInt() const {
		return { int16_t(x), int16_t(y), int16_t(z) };
	}
	FI XYZval<int32_t> asLong() {
		return { int32_t(x), int32_t(y), int32_t(z) };
	}
	FI XYZval<int32_t> asLong() const {
		return { int32_t(x), int32_t(y), int32_t(z) };
	}
	FI XYZval<int32_t> ROUNDL() {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)), int32_t(LROUND(z)) };
	}
	FI XYZval<int32_t> ROUNDL() const {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)), int32_t(LROUND(z)) };
	}
	FI XYZval<float> asFloat() {
		return { float(x), float(y), float(z) };
	}
	FI XYZval<float> asFloat() const {
		return { float(x), float(y), float(z) };
	}
	FI XYZval<float> reciprocal() const {
		return { _RECIP(x), _RECIP(y), _RECIP(z) };
	}
	FI XYZval<float> asLogical() const {
		XYZval<float> o = asFloat();
		toLogical(o);
		return o;
	}
	FI XYZval<float> asNative() const {
		XYZval<float> o = asFloat();
		toNative(o);
		return o;
	}
	FI operator XYval<T>&() {
		return *(XYval<T>*) this;
	}
	FI operator const XYval<T>&() const {
		return *(const XYval<T>*) this;
	}
	FI operator XYZAval<T>() const {
		return { x, y, z, 0 };
	}
	FI T& operator[](const int i) {
		return pos[i];
	}
	FI const T& operator[](const int i) const {
		return pos[i];
	}
	FI XYZval<T>& operator=(const T v) {
		set(v, v, v);
		return *this;
	}
	FI XYZval<T>& operator=(const XYval<T>& rs) {
		set(rs.x, rs.y);
		return *this;
	}
	FI XYZval<T>& operator=(const XYZAval<T>& rs) {
		set(rs.x, rs.y, rs.z);
		return *this;
	}
	FI XYZval<T> operator+(const XYval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYZval<T> operator+(const XYval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYZval<T> operator-(const XYval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYZval<T> operator-(const XYval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYZval<T> operator*(const XYval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYZval<T> operator*(const XYval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYZval<T> operator/(const XYval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYZval<T> operator/(const XYval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYZval<T> operator+(const XYZval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZval<T> operator+(const XYZval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZval<T> operator-(const XYZval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZval<T> operator-(const XYZval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZval<T> operator*(const XYZval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZval<T> operator*(const XYZval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZval<T> operator/(const XYZval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZval<T> operator/(const XYZval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZval<T> operator+(const XYZAval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZval<T> operator+(const XYZAval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZval<T> operator-(const XYZAval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZval<T> operator-(const XYZAval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZval<T> operator*(const XYZAval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZval<T> operator*(const XYZAval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZval<T> operator/(const XYZAval<T>& rs) const {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZval<T> operator/(const XYZAval<T>& rs) {
		XYZval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZval<T> operator*(const float& v) const {
		XYZval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		return ls;
	}
	FI XYZval<T> operator*(const float& v) {
		XYZval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		return ls;
	}
	FI XYZval<T> operator*(const int& v) const {
		XYZval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		return ls;
	}
	FI XYZval<T> operator*(const int& v) {
		XYZval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		return ls;
	}
	FI XYZval<T> operator/(const float& v) const {
		XYZval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		return ls;
	}
	FI XYZval<T> operator/(const float& v) {
		XYZval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		return ls;
	}
	FI XYZval<T> operator/(const int& v) const {
		XYZval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		return ls;
	}
	FI XYZval<T> operator/(const int& v) {
		XYZval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		return ls;
	}
	FI XYZval<T> operator>>(const int& v) const {
		XYZval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		_RS(ls.z);
		return ls;
	}
	FI XYZval<T> operator>>(const int& v) {
		XYZval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		_RS(ls.z);
		return ls;
	}
	FI XYZval<T> operator<<(const int& v) const {
		XYZval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		_LS(ls.z);
		return ls;
	}
	FI XYZval<T> operator<<(const int& v) {
		XYZval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		_LS(ls.z);
		return ls;
	}
	FI XYZval<T>& operator+=(const XYval<T>& rs) {
		x += rs.x;
		y += rs.y;
		return *this;
	}
	FI XYZval<T>& operator-=(const XYval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		return *this;
	}
	FI XYZval<T>& operator*=(const XYval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		return *this;
	}
	FI XYZval<T>& operator/=(const XYval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		return *this;
	}
	FI XYZval<T>& operator+=(const XYZval<T>& rs) {
		x += rs.x;
		y += rs.y;
		z += rs.z;
		return *this;
	}
	FI XYZval<T>& operator-=(const XYZval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		z -= rs.z;
		return *this;
	}
	FI XYZval<T>& operator*=(const XYZval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		z *= rs.z;
		return *this;
	}
	FI XYZval<T>& operator/=(const XYZval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		z /= rs.z;
		return *this;
	}
	FI XYZval<T>& operator+=(const XYZAval<T>& rs) {
		x += rs.x;
		y += rs.y;
		z += rs.z;
		return *this;
	}
	FI XYZval<T>& operator-=(const XYZAval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		z -= rs.z;
		return *this;
	}
	FI XYZval<T>& operator*=(const XYZAval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		z *= rs.z;
		return *this;
	}
	FI XYZval<T>& operator/=(const XYZAval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		z /= rs.z;
		return *this;
	}
	FI XYZval<T>& operator*=(const float& v) {
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}
	FI XYZval<T>& operator*=(const int& v) {
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}
	FI XYZval<T>& operator>>=(const int& v) {
		_RS(x);
		_RS(y);
		_RS(z);
		return *this;
	}
	FI XYZval<T>& operator<<=(const int& v) {
		_LS(x);
		_LS(y);
		_LS(z);
		return *this;
	}
	FI bool operator==(const XYZval<T>& rs) {
		return x == rs.x && y == rs.y && z == rs.z;
	}
	FI bool operator!=(const XYZval<T>& rs) {
		return !operator==(rs);
	}
	FI bool operator==(const XYZAval<T>& rs) const {
		return x == rs.x && y == rs.y && z == rs.z;
	}
	FI bool operator!=(const XYZAval<T>& rs) const {
		return !operator==(rs);
	}
	FI XYZval<T> operator-() {
		XYZval<T> o = *this;
		o.x = -x;
		o.y = -y;
		o.z = -z;
		return o;
	}
	FI const XYZval<T> operator-() const {
		XYZval<T> o = *this;
		o.x = -x;
		o.y = -y;
		o.z = -z;
		return o;
	}
};

//
// XYZA coordinates, counters, etc.
//
template<typename T>
struct XYZAval {
	union {
		struct {
			T x, y, z, a;
		};
		T pos[4];
	};
	FI void reset() {
		x = y = z = a = 0;
	}
	FI T magnitude() const {
		return (T) sqrtf(x * x + y * y + z * z + a * a);
	}
	FI operator T*() {
		return pos;
	}
	FI operator bool() {
		return a || z || x || y;
	}
	FI void set(const T px) {
		x = px;
	}
	FI void set(const T px, const T py) {
		x = px;
		y = py;
	}
	FI void set(const T px, const T py, const T pz) {
		x = px;
		y = py;
		z = pz;
	}
	FI void set(const T px, const T py, const T pz, const T pa) {
		x = px;
		y = py;
		z = pz;
		a = pa;
	}
	FI void set(const XYval<T> pxy) {
		x = pxy.x;
		y = pxy.y;
	}
	FI void set(const XYval<T> pxy, const T pz) {
		x = pxy.x;
		y = pxy.y;
		z = pz;
	}
	FI void set(const XYZval<T> pxyz) {
		x = pxyz.x;
		y = pxyz.y;
		z = pxyz.z;
	}
	FI void set(const XYval<T> pxy, const T pz, const T pa) {
		x = pxy.x;
		y = pxy.y;
		z = pz;
		a = pa;
	}
	FI void set(const XYval<T> pxy, const XYval<T> pza) {
		x = pxy.x;
		y = pxy.y;
		z = pza.z;
		a = pza.a;
	}
	FI void set(const XYZval<T> pxyz, const T pa) {
		x = pxyz.x;
		y = pxyz.y;
		z = pxyz.z;
		a = pa;
	}
	FI void set(const T (&arr)[XY_N]) {
		x = arr[0];
		y = arr[1];
	}
	FI void set(const T (&arr)[XYZ_N]) {
		x = arr[0];
		y = arr[1];
		z = arr[2];
	}
	FI void set(const T (&arr)[XYZA_N]) {
		x = arr[0];
		y = arr[1];
		z = arr[2];
		a = arr[3];
	}
	FI XYZAval<T> copy() const {
		return *this;
	}
	FI XYZAval<T> ABS() const {
		return { T(_ABS(x)), T(_ABS(y)), T(_ABS(z)), T(_ABS(a)) };
	}
	FI XYZAval<int16_t> asInt() {
		return { int16_t(x), int16_t(y), int16_t(z), int16_t(a) };
	}
	FI XYZAval<int16_t> asInt() const {
		return { int16_t(x), int16_t(y), int16_t(z), int16_t(a) };
	}
	FI XYZAval<int32_t> asLong() {
		return { int32_t(x), int32_t(y), int32_t(z), int32_t(a) };
	}
	FI XYZAval<int32_t> asLong() const {
		return { int32_t(x), int32_t(y), int32_t(z), int32_t(a) };
	}
	FI XYZAval<int32_t> ROUNDL() {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)), int32_t(LROUND(z)), int32_t(LROUND(a)) };
	}
	FI XYZAval<int32_t> ROUNDL() const {
		return { int32_t(LROUND(x)), int32_t(LROUND(y)), int32_t(LROUND(z)), int32_t(LROUND(a)) };
	}
	FI XYZAval<float> asFloat() {
		return { float(x), float(y), float(z), float(a) };
	}
	FI XYZAval<float> asFloat() const {
		return { float(x), float(y), float(z), float(a) };
	}
	FI XYZAval<float> reciprocal() const {
		return { _RECIP(x), _RECIP(y), _RECIP(z), _RECIP(a) };
	}
	FI XYZAval<float> asLogical() const {
		XYZAval<float> o = asFloat();
		toLogical(o);
		return o;
	}
	FI XYZAval<float> asNative() const {
		XYZAval<float> o = asFloat();
		toNative(o);
		return o;
	}
	FI operator XYval<T>&() {
		return *(XYval<T>*) this;
	}
	FI operator const XYval<T>&() const {
		return *(const XYval<T>*) this;
	}
	FI operator XYZval<T>&() {
		return *(XYZval<T>*) this;
	}
	FI operator const XYZval<T>&() const {
		return *(const XYZval<T>*) this;
	}
	FI T& operator[](const int i) {
		return pos[i];
	}
	FI const T& operator[](const int i) const {
		return pos[i];
	}
	FI XYZAval<T>& operator=(const T v) {
		set(v, v, v, v);
		return *this;
	}
	FI XYZAval<T>& operator=(const XYval<T>& rs) {
		set(rs.x, rs.y);
		return *this;
	}
	FI XYZAval<T>& operator=(const XYZval<T>& rs) {
		set(rs.x, rs.y, rs.z);
		return *this;
	}
	FI XYZAval<T> operator+(const XYval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYZAval<T> operator+(const XYval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		return ls;
	}
	FI XYZAval<T> operator-(const XYval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYZAval<T> operator-(const XYval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		return ls;
	}
	FI XYZAval<T> operator*(const XYval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYZAval<T> operator*(const XYval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		return ls;
	}
	FI XYZAval<T> operator/(const XYval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYZAval<T> operator/(const XYval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		return ls;
	}
	FI XYZAval<T> operator+(const XYZval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZAval<T> operator+(const XYZval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		return ls;
	}
	FI XYZAval<T> operator-(const XYZval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZAval<T> operator-(const XYZval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		return ls;
	}
	FI XYZAval<T> operator*(const XYZval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZAval<T> operator*(const XYZval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		return ls;
	}
	FI XYZAval<T> operator/(const XYZval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZAval<T> operator/(const XYZval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		return ls;
	}
	FI XYZAval<T> operator+(const XYZAval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		ls.a += rs.a;
		return ls;
	}
	FI XYZAval<T> operator+(const XYZAval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x += rs.x;
		ls.y += rs.y;
		ls.z += rs.z;
		ls.a += rs.a;
		return ls;
	}
	FI XYZAval<T> operator-(const XYZAval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		ls.a -= rs.a;
		return ls;
	}
	FI XYZAval<T> operator-(const XYZAval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x -= rs.x;
		ls.y -= rs.y;
		ls.z -= rs.z;
		ls.a -= rs.a;
		return ls;
	}
	FI XYZAval<T> operator*(const XYZAval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		ls.a *= rs.a;
		return ls;
	}
	FI XYZAval<T> operator*(const XYZAval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x *= rs.x;
		ls.y *= rs.y;
		ls.z *= rs.z;
		ls.a *= rs.a;
		return ls;
	}
	FI XYZAval<T> operator/(const XYZAval<T>& rs) const {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		ls.a /= rs.a;
		return ls;
	}
	FI XYZAval<T> operator/(const XYZAval<T>& rs) {
		XYZAval<T> ls = *this;
		ls.x /= rs.x;
		ls.y /= rs.y;
		ls.z /= rs.z;
		ls.a /= rs.a;
		return ls;
	}
	FI XYZAval<T> operator*(const float& v) const {
		XYZAval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		ls.a *= v;
		return ls;
	}
	FI XYZAval<T> operator*(const float& v) {
		XYZAval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		ls.a *= v;
		return ls;
	}
	FI XYZAval<T> operator*(const int& v) const {
		XYZAval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		ls.a *= v;
		return ls;
	}
	FI XYZAval<T> operator*(const int& v) {
		XYZAval<T> ls = *this;
		ls.x *= v;
		ls.y *= v;
		ls.z *= v;
		ls.a *= v;
		return ls;
	}
	FI XYZAval<T> operator/(const float& v) const {
		XYZAval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		ls.a /= v;
		return ls;
	}
	FI XYZAval<T> operator/(const float& v) {
		XYZAval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		ls.a /= v;
		return ls;
	}
	FI XYZAval<T> operator/(const int& v) const {
		XYZAval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		ls.a /= v;
		return ls;
	}
	FI XYZAval<T> operator/(const int& v) {
		XYZAval<T> ls = *this;
		ls.x /= v;
		ls.y /= v;
		ls.z /= v;
		ls.a /= v;
		return ls;
	}
	FI XYZAval<T> operator>>(const int& v) const {
		XYZAval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		_RS(ls.z);
		_RS(ls.a);
		return ls;
	}
	FI XYZAval<T> operator>>(const int& v) {
		XYZAval<T> ls = *this;
		_RS(ls.x);
		_RS(ls.y);
		_RS(ls.z);
		_RS(ls.a);
		return ls;
	}
	FI XYZAval<T> operator<<(const int& v) const {
		XYZAval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		_LS(ls.z);
		_LS(ls.a);
		return ls;
	}
	FI XYZAval<T> operator<<(const int& v) {
		XYZAval<T> ls = *this;
		_LS(ls.x);
		_LS(ls.y);
		_LS(ls.z);
		_LS(ls.a);
		return ls;
	}
	FI XYZAval<T>& operator+=(const XYval<T>& rs) {
		x += rs.x;
		y += rs.y;
		return *this;
	}
	FI XYZAval<T>& operator-=(const XYval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		return *this;
	}
	FI XYZAval<T>& operator*=(const XYval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		return *this;
	}
	FI XYZAval<T>& operator/=(const XYval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		return *this;
	}
	FI XYZAval<T>& operator+=(const XYZval<T>& rs) {
		x += rs.x;
		y += rs.y;
		z += rs.z;
		return *this;
	}
	FI XYZAval<T>& operator-=(const XYZval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		z -= rs.z;
		return *this;
	}
	FI XYZAval<T>& operator*=(const XYZval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		z *= rs.z;
		return *this;
	}
	FI XYZAval<T>& operator/=(const XYZval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		z /= rs.z;
		return *this;
	}
	FI XYZAval<T>& operator+=(const XYZAval<T>& rs) {
		x += rs.x;
		y += rs.y;
		z += rs.z;
		a += rs.a;
		return *this;
	}
	FI XYZAval<T>& operator-=(const XYZAval<T>& rs) {
		x -= rs.x;
		y -= rs.y;
		z -= rs.z;
		a -= rs.a;
		return *this;
	}
	FI XYZAval<T>& operator*=(const XYZAval<T>& rs) {
		x *= rs.x;
		y *= rs.y;
		z *= rs.z;
		a *= rs.a;
		return *this;
	}
	FI XYZAval<T>& operator/=(const XYZAval<T>& rs) {
		x /= rs.x;
		y /= rs.y;
		z /= rs.z;
		a /= rs.a;
		return *this;
	}
	FI XYZAval<T>& operator*=(const T& v) {
		x *= v;
		y *= v;
		z *= v;
		a *= v;
		return *this;
	}
	FI XYZAval<T>& operator>>=(const int& v) {
		_RS(x);
		_RS(y);
		_RS(z);
		_RS(a);
		return *this;
	}
	FI XYZAval<T>& operator<<=(const int& v) {
		_LS(x);
		_LS(y);
		_LS(z);
		_LS(a);
		return *this;
	}
	FI bool operator==(const XYZval<T>& rs) {
		return x == rs.x && y == rs.y && z == rs.z;
	}
	FI bool operator!=(const XYZval<T>& rs) {
		return !operator==(rs);
	}
	FI bool operator==(const XYZAval<T>& rs) const {
		return x == rs.x && y == rs.y && z == rs.z && a == rs.a;
	}
	FI bool operator!=(const XYZAval<T>& rs) const {
		return !operator==(rs);
	}
	FI XYZAval<T> operator-() {
		return { -x, -y, -z, -a };
	}
	FI const XYZAval<T> operator-() const {
		return { -x, -y, -z, -a };
	}
};

#undef _RECIP
#undef _ABS
#undef _LS
#undef _RS
#undef FI

const xyza_char_t axis_codes { 'X', 'Y', 'Z', 'A' };
#define XYZ_CHAR(A) ((char) ('X' + A))
#define ABC_CHAR(A) ((char) ('A' + A - 3))