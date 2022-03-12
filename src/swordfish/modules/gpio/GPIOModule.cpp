/*
 * GPIOModule.cpp
 *
 * Created: 30/09/2021 9:48:01 am
 *  Author: smohekey
 */ 

#include "GPIOModule.h"

namespace swordfish::gpio {
	GPIOModule* GPIOModule::__instance = nullptr;
	
	core::ObjectField<PinTable> GPIOModule::__pinTableField = { "pins" , 0 };
	
	core::Schema GPIOModule::__schema = {
		utils::typeName<GPIOModule>(),
		&(Module::__schema), {
			
		}, {
			__pinTableField
		}
	};
	
	GPIOModule::GPIOModule(core::Object* parent) :
		Module(parent),
		_pack(__schema, *this, &(Module::_pack)) {

	}
	
	void GPIOModule::init() {
		
	}
	
	void GPIOModule::idle() {
		
	}

	swordfish::gpio::GPIOModule& GPIOModule::getInstance(core::Object* parent) {
		return *(__instance ?: __instance = new GPIOModule(parent));
	}

}