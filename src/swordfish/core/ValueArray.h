/*
 * ValueArray.h
 *
 * Created: 9/08/2021 1:39:53 pm
 *  Author: smohekey
 */ 

#pragma once

#include <stdexcept>

#include "Array.h"

namespace pack {
	template<typename T>
	class ValueArray : public Array<T> {
	public:
		ValueArray(uintptr_t pointer) : Array<T>(pointer) {
			
		}
		
		T operator[](uint32_t index) override {
			ArrayPointer* pointer = Array<T>::_pointer;
			
			uint32_t length = pointer->getElementCount();
			
			if(index >= length) {
				throw new std::out_of_range("");
			}
			
			return reinterpret_cast<T>((uintptr_t)this + pointer->getOffset() + index * pointer->getElementSize());
		}
	};	
}