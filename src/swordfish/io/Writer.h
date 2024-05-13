/*
 * Writer.h
 *
 * Created: 9/08/2021 3:09:25 pm
 *  Author: smohekey
 */

#pragma once

#include <string_view>

#include <swordfish/math.h>
#include <swordfish/types.h>

#include "OutputStream.h"

namespace swordfish::io {
	template <typename T>
	class HasToString {
	private:
		typedef char YesType[1];
		typedef char NoType[2];

		template <typename C> static YesType& test( decltype(&C::toString) ) ;
		template <typename C> static NoType& test(...);


	public:
		enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
	};

	extern char nl;

	class Writer {
	private:
		OutputStream& _stream;

	public:
		Writer(OutputStream& stream) : _stream(stream) {

		}

		inline OutputStream& stream() const { return _stream; }

		Writer& operator<<(bool value);

		Writer& operator<<(const char value);

		Writer& operator<<(int value);
		Writer& operator<<(int8_t value);
		Writer& operator<<(int16_t value);
		Writer& operator<<(int32_t value);
		Writer& operator<<(int64_t value);

		Writer& operator<<(unsigned int value);
		Writer& operator<<(uint8_t value);
		Writer& operator<<(uint16_t value);
		Writer& operator<<(uint32_t value);
		Writer& operator<<(uint64_t value);

		Writer& operator<<(float32_t value);
		Writer& operator<<(float64_t value);

		Writer& operator<<(const char* value);

		Writer& operator<<(const std::string_view value);

		Writer& operator<<(const void* value);

		template<typename T, std::enable_if_t<HasToString<T>::value, bool> = true>
		Writer& operator<<(const T& value) {
			*this << value.toString();

			return *this;
		}
	};
}