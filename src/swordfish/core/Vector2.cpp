/*
 * Vector2.cpp
 *
 * Created: 30/11/2021 1:51:13 pm
 *  Author: smohekey
 */ 

#include "Vector2.h"

namespace swordfish::core {
	ValueField<float32_t> Vector2::__xField = { "x", 0, 0.0f };
	ValueField<float32_t> Vector2::__yField = { "y", 4, 0.0f };
	
	Schema Vector2::__schema = {
		utils::typeName<Vector2>(),
		nullptr, {
			__xField,
			__yField
			}, {
			
		}
	};
}