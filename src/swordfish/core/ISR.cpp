/*
 * ISR.cpp
 *
 * Created: 30/11/2021 1:48:22 pm
 *  Author: smohekey
 */

#include "ISR.h"

namespace swordfish::core {
	std::function<void()> ISR::__handlers[16] = {};
	bool ISR::__debounce[16] = { false };
	uint32_t ISR::__last_time[16] = { 0 };
} // namespace swordfish::core