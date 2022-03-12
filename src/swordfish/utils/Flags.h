/*
 * EnumBitwiseOperators.h
 *
 * Created: 15/12/2021 9:26:18 am
 *  Author: smohekey
 */ 

#pragma once

#include <bitset>
#include <type_traits>

namespace swordfish::utils {
	template<typename TEnum>
	class Flags {
	private:
		static_assert(std::is_enum_v<TEnum>, "Flags can only be specialized for enum types.");
		
		using underlying_type = typename std::make_unsigned_t<typename std::underlying_type_t<TEnum>>;
		
		underlying_type _value;
		
		Flags(underlying_type value) : _value(value) {
			
		}
		
	public:
		Flags() {
		
		}
		
		Flags(TEnum e) : _value(underlying_type(e)) {
			
		}
	
		Flags<TEnum> operator |(TEnum rhs) {
			return { _value | underlying(rhs) };
		}

		Flags<TEnum> operator &(TEnum rhs) {
			return { _value & underlying(rhs) };
		}
	
		Flags<TEnum> operator ^(TEnum rhs) {
			return { _value ^ underlying(rhs) };
		}
	
		Flags<TEnum>& operator=(TEnum rhs) {
			_value = underlying(rhs);
		
			return *this;
		}
		
		Flags<TEnum>& operator |=(TEnum rhs) {
			_value |= underlying(rhs);
		
			return *this;
		}
	
		Flags<TEnum>& operator &=(TEnum rhs) {
			_value &= underlying(rhs);
		
			return *this;
		}
	
		Flags<TEnum>& operator ^=(TEnum rhs) {
			_value ^= underlying(rhs);
		
			return *this;
		}
		
		operator bool() {
			return _value;
		}
		
	private:
		static constexpr underlying_type underlying(TEnum& e) {
			return static_cast<underlying_type>(e);
		}
	};
}