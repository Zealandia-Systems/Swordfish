/*
 * Pin.cpp
 *
 * Created: 30/09/2021 8:51:06 am
 *  Author: smohekey
 */ 

#include "Pin.h"

namespace swordfish::gpio {
	core::ValueField<int8_t> Pin::__portField = { "port", 0, -1 };
	core::ValueField<int8_t> Pin::__pinField = { "pin", 1, -1 };
	core::ValueField<bool> Pin::__inputField = { "input", 16, true };
	core::ValueField<bool> Pin::__pullUpField = { "pullUp", 17, false };
	core::ValueField<bool> Pin::__pullDownField = { "pullDown", 18, false };
	core::ValueField<bool> Pin::__totemPoleField = { "totemPole", 19, false };
	
	core::Schema Pin::__schema = {
		utils::typeName<Pin>(),
		nullptr, {
			__portField,
			__pinField,
			__inputField,
			__pullUpField,
			__pullDownField,
			__totemPoleField
			}, {
			
		}
	};
}