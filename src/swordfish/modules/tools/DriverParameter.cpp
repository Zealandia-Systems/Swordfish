/*
 * DriverParameter.cpp
 *
 * Created: 30/11/2021 10:04:14 am
 *  Author: smohekey
 */ 

#include "DriverParameter.h"

namespace swordfish::tools {
	core::ValueField<int16_t> DriverParameter::__indexField = { "index", 0, 0 };
	core::ValueField<int16_t> DriverParameter::__driverIndexField = { "driver", 2, 0 };
	core::ValueField<uint16_t> DriverParameter::__idField = { "id", 4, 0 };
	
	core::ObjectField<core::String> DriverParameter::__valueField = { "value", 0 };
	
	core::Schema DriverParameter::__schema = {
		utils::typeName<DriverParameter>(),
		nullptr, {
			__indexField,
			__driverIndexField,
			__idField
			}, {
			__valueField
		}
	};
}