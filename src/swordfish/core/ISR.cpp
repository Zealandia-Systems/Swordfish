/*
 * ISR.cpp
 *
 * Created: 30/11/2021 1:48:22 pm
 *  Author: smohekey
 */ 

#include "ISR.h"

namespace swordfish::core {
	std::function<void()> ISR::__handlers[16] = { };
}