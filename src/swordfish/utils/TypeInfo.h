/*
 * TypeInfo.h
 *
 * Created: 18/08/2021 12:36:20 pm
 *  Author: smohekey
 */ 

#pragma once

#include <string_view>

namespace swordfish::utils {
	template<class T>
	constexpr std::string_view typeName() {
		char const* p = __PRETTY_FUNCTION__;
		
		while (*p++ != '=');
		while (*++p == ' ');
		
		char const* p2 = p;
		
		int count = 1;
		
		while(*p2++) {
			switch (*p2) {
				case '[': {
					++count;
					
					break;
				}
				
				case ']': {
					--count;
					
					if (!count) {
						return { p, std::size_t(p2 - p) };
					}
					
					break;
				}
				
				case ';': {
					return { p, std::size_t(p2 - p) };
				}
			}
		}
		
		return { };
	}
}