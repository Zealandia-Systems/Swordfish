/*
 * ISR.h
 *
 * Created: 29/09/2021 9:23:37 am
 *  Author: smohekey
 */

#pragma once

#include <functional>

#include <sam.h>
#include <Arduino.h>

#include <swordfish/debug.h>

namespace swordfish::core {
	class ISR {
	private:
		static std::function<void()> __handlers[16];
		static bool __debounce[16];
		static uint32_t __last_time[16];

		static void __handler() {
			uint8_t active = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) - 14 + NonMaskableInt_IRQn;

			debug()("active: ", active);

			if (__debounce[active]) {
				auto const time = micros();
				auto const last_time = __last_time[active];

				__last_time[active] = time;

				if (!(time - last_time > 1000)) {
					return;
				}
			}

			if (__handlers[active]) {
				__handlers[active]();
			}
		}

		const uint32_t _pin;
		const uint32_t _mode;

	public:
		ISR(std::function<void()> handler, uint32_t pin, uint32_t mode, bool debounce = false) :
				_pin(pin), _mode(mode) {
			auto const index = g_APinDescription[pin].ulExtInt;

			__handlers[index] = handler;
			__debounce[index] = debounce;
		}

		void attach() {
			attachInterrupt(_pin, __handler, _mode);
		}

		void detach() {
			detachInterrupt(_pin);
		}
	};
} // namespace swordfish::core