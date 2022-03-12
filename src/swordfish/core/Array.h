/*
 * Array.h
 *
 * Created: 9/08/2021 9:25:44 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>

namespace swordfish::core {
	template<typename T>
	class Array : public Object {
	public:
		
		virtual uint32_t length() const = 0;
		
		virtual T operator[](uint32_t index) = 0;
		
		virtual T emplaceBack() = 0;
		
		virtual void popBack() = 0;
	};
}