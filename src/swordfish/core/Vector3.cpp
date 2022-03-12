/*
 * Vector3.cpp
 *
 * Created: 30/11/2021 1:49:54 pm
 *  Author: smohekey
 */ 

#include "Vector3.h"

namespace swordfish::core {
	ValueField<float32_t> Vector3::__xField = { "x", 0, 0.0f };
	ValueField<float32_t> Vector3::__yField = { "y", 4, 0.0f };
	ValueField<float32_t> Vector3::__zField = { "z", 8, 0.0f };
	
	Schema Vector3::__schema = {
		utils::typeName<Vector3>(),
		nullptr, {
			__xField,
			__yField,
			__zField
		}, {
			
		}
	};
}