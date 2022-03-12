/*
 * Module.cpp
 *
 * Created: 27/08/2021 5:01:16 pm
 *  Author: smohekey
 */ 

#include "Module.h"

namespace swordfish {
	core::ValueField<bool> Module::__enabledField = { "enabled", 0, true };

	core::Schema Module::__schema = {
		utils::typeName<Module>(),
		nullptr, {
			__enabledField
		}, {
					
		}
	};
			
	core::Pack& Module::getPack() {
		return _pack;
	}
}
