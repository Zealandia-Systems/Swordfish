/*
 * PWMLaserDriverImpl.cpp
 *
 * Created: 27/10/2021 8:36:04 am
 *  Author: smohekey
 */

#include <Arduino.h>
#include <sam.h>

#include <swordfish/debug.h>

#include "PWMLaserDriverImpl.h"

namespace swordfish::tools::drivers {
	void PWMLaserDriverImpl::init([[maybe_unused]] uint16_t index, [[maybe_unused]] DriverParameterTable& parameters) {

		_cyclesPerPeriod = 800;

		debug()("cyclesPerPeriod: ", _cyclesPerPeriod);

		MCLK->APBAMASK.reg =
				MCLK->APBAMASK.reg |
				MCLK_APBDMASK_TC7;

		GCLK->PCHCTRL[TC7_GCLK_ID].reg = GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK7_Val) | (1 << GCLK_PCHCTRL_CHEN_Pos);
		while (GCLK->PCHCTRL[TC7_GCLK_ID].bit.CHEN == 0)
			;

		// Enable the peripheral multiplexer on pin D7
		PORT->Group[g_APinDescription[11].ulPort].PINCFG[g_APinDescription[11].ulPin].bit.PMUXEN = 1;

		// Set the D11 (PORT_PB23) peripheral multiplexer to peripheral (odd port number) O(4): TC7, Channel 0
		PORT->Group[g_APinDescription[11].ulPort].PMUX[g_APinDescription[11].ulPin >> 1].reg =
				PORT->Group[g_APinDescription[11].ulPort].PMUX[g_APinDescription[11].ulPin >> 1].reg |
				PORT_PMUX_PMUXO(4);

		auto& tc = TC7->COUNT16;

		tc.CTRLA.bit.SWRST = 1;
		while (tc.SYNCBUSY.bit.SWRST)
			;

		tc.CTRLA.bit.ENABLE = 0;
		while (tc.SYNCBUSY.bit.ENABLE)
			;

		tc.INTENCLR.reg = TC_INTENCLR_MASK;

		tc.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1;

		tc.WAVE.reg = TC_WAVE_WAVEGEN_MPWM;

		while (tc.SYNCBUSY.bit.CC0 || tc.SYNCBUSY.bit.CC1)
			;

		tc.CC[0].reg = _cyclesPerPeriod;
		while (tc.SYNCBUSY.bit.CC0)
			;

		tc.CC[1].reg = 0;
		while (tc.SYNCBUSY.bit.CC1)
			;

		tc.CTRLA.bit.ENABLE = 1;
		while (tc.SYNCBUSY.bit.ENABLE)
			;

		_targetPower = 0;

		apply();
	}

	void PWMLaserDriverImpl::idle() {
		/*static uint32_t _nextRefresh = 0;

		if(_enabled) {
		  auto ms = millis();

		  if(ms > _nextRefresh) {
		    auto& tc = TC7->COUNT16;

		    tc.CTRLBSET.reg = TC_CTRLBSET_CMD_READSYNC;
		    while(tc.SYNCBUSY.bit.CTRLB);
		    while(tc.SYNCBUSY.bit.COUNT);

		    debug()("count: ", tc.COUNT.reg);

		    auto delta = 100;

		    _nextRefresh = ms + delta;
		  }
		}*/
	}

	bool PWMLaserDriverImpl::isEnabled() const {
		return _enabled;
	}

	void PWMLaserDriverImpl::setEnabled(bool enabled) {
		_enabled = enabled;
	}

	float32_t PWMLaserDriverImpl::getTargetPower() const {
		return _targetPower;
	}

	void PWMLaserDriverImpl::setTargetPower(float32_t targetPower) {
		_targetPower = targetPower;
	}

	float32_t PWMLaserDriverImpl::getCurrentPower() const {
		return _currentPower;
	}

	uint32_t PWMLaserDriverImpl::getOutputFrequency() const {
		return _cyclesPerPeriod;
	}

	float32_t PWMLaserDriverImpl::getPowerOverride() const {
		return _powerOverride;
	}

	void PWMLaserDriverImpl::setPowerOverride(float32_t powerOverride) {
		_powerOverride = powerOverride;
	}

	void PWMLaserDriverImpl::apply() {
		auto& tc = TC7->COUNT16;

		uint16_t targetCC = (uint16_t) ((float32_t) _cyclesPerPeriod / 99 * _targetPower);

		debug()("enabled: ", _enabled, ", targetPower: ", _targetPower, ", targetCC: ", targetCC);

		if (_enabled) {
			tc.CC[1].reg = targetCC;
			while (tc.SYNCBUSY.bit.CC1)
				;

			tc.CTRLBSET.reg = TC_CTRLBSET_CMD_RETRIGGER;
			while (tc.SYNCBUSY.bit.CTRLB)
				;
		} else {
			tc.CC[1].reg = 0;
			while (tc.SYNCBUSY.bit.CC1)
				;

			tc.CTRLBSET.reg = TC_CTRLBSET_CMD_STOP;
			while (tc.SYNCBUSY.bit.CTRLB)
				;
		}

		// analogWrite(11, _enabled ? _targetPower : 0);

		// while(tc.SYNCBUSY.bit.CC1);

		_currentPower = _enabled ? _targetPower : 0;
	}
} // namespace swordfish::tools::drivers
