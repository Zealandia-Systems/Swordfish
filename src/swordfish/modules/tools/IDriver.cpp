/*
 * IDriver.cpp
 *
 * Created: 3/11/2021 9:30:31 am
 *  Author: smohekey
 */ 

#include "IDriver.h"

namespace swordfish::tools {
	
	float32_t NullDriver::getPowerOverride() const {
		return 100.0f;
	}

}