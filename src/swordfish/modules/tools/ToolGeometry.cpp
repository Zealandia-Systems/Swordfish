/*
 * ToolGeometry.cpp
 *
 * Created: 30/11/2021 9:57:45 am
 *  Author: smohekey
 */ 

#include "ToolGeometry.h"

namespace swordfish::tools {
	core::ValueField<float32_t> ToolGeometry::__diameterField = { "diameter", 0, 0.0f };
	core::ValueField<float32_t> ToolGeometry::__lengthField = { "length", 4, 0.0f };
	
	core::Schema ToolGeometry::__schema = {
		utils::typeName<ToolGeometry>(),
		nullptr, {
			__diameterField,
			__lengthField
			}, {
			
		}
	};
}