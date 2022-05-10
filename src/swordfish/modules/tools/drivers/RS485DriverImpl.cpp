/*
 * VariableFrequencyDriver.cpp
 *
 * Created: 14/10/2021 1:31:12 pm
 *  Author: smohekey
 */

#include <charconv>

#include <marlin/gcode/gcode.h>
#include <marlin/libs/modbus.h>

#include <swordfish/debug.h>
#include <swordfish/modules/estop/EStopModule.h>

#include "../DriverParameter.h"
#include "../DriverException.h"
#include "RS485DriverImpl.h"

#define SPINDLE_FAN_ON  !SPINDLE_FAN_PIN_INVERTED
#define SPINDLE_FAN_OFF SPINDLE_FAN_PIN_INVERTED

namespace swordfish::tools::drivers {
	using namespace swordfish::estop;

	void RS485DriverImpl::init(uint16_t index, DriverParameterTable& parameters) {
		for (auto& parameter : parameters) {
			if (parameter.getDriverIndex() != index) {
				continue;
			}

			auto id = (IDriver::Parameter) parameter.getId();
			auto value = parameter.getValue().value();

			if (id == IDriver::Parameter::RS485_SlaveAddress) {
				auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), _slaveAddress);

				(void) ptr;
				(void) ec;
			} else if (id == IDriver::Parameter::RS485_BaudRate) {
				auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), _baudRate);

				(void) ptr;
				(void) ec;
			} else if (id == IDriver::Parameter::MaxPower) {
				auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), _maxPower);

				(void) ptr;
				(void) ec;
			} else if (id == IDriver::Parameter::HasFan) {
				_hasFan = value.data()[0] != '0';
			} else if (id == IDriver::Parameter::FanTimeout) {
				auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), _fanTimeoutMs);

				(void) ptr;
				(void) ec;
			}
		}

		if (_hasFan) {
			SET_OUTPUT(SPINDLE_FAN_PIN);
			WRITE(SPINDLE_FAN_PIN, SPINDLE_FAN_OFF);
		}

		modbus::init(_baudRate);
	}

	uint8_t RS485DriverImpl::getSlaveAddress() const {
		return _slaveAddress;
	}

	uint16_t RS485DriverImpl::getMaxPower() const {
		return _maxPower;
	}

	void RS485DriverImpl::idle() {
		auto ms = millis();

		if (((int64_t) _nextRefresh - (int64_t) ms) < 0) {
			// if (!(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)) {
			refresh();
			//}

			auto delta = _enabled ? 100 : 1000;

			_nextRefresh = ms + delta;
		}

		if (_hasFan && !_enabled && ((int64_t) _fanOffTime - (int64_t) ms) < 0) {
			WRITE(SPINDLE_FAN_PIN, SPINDLE_FAN_OFF);
		}
	}

	bool RS485DriverImpl::isEnabled() const {
		return _enabled;
	}

	void RS485DriverImpl::setEnabled(bool enabled) {
		_enabled = enabled;
	}

	bool RS485DriverImpl::hasDirection() const {
		return true;
	}

	Direction RS485DriverImpl::getTargetDirection() const {
		return _targetDirection;
	}

	void RS485DriverImpl::setTargetDirection(const Direction direction) {
		_targetDirection = direction;
	}

	Direction RS485DriverImpl::getCurrentDirection() const {
		return _outputDirection;
	}

	float32_t RS485DriverImpl::getTargetPower() const {
		return _targetPower;
	}

	void RS485DriverImpl::setTargetPower(float32_t targetPower) {
		_targetPower = targetPower;
	}

	float32_t RS485DriverImpl::getCurrentPower() const {
		return _currentPower;
	}

	uint16_t RS485DriverImpl::getOutputFrequency() const {
		return _outputFrequency;
	}

	float32_t RS485DriverImpl::getPowerOverride() const {
		return _powerOverride;
	}

	void RS485DriverImpl::setPowerOverride(float32_t powerOverride) {
		_powerOverride = powerOverride;
	}

	void RS485DriverImpl::waitForSpindle(uint16_t targetFrequency) {
		auto& estop = EStopModule::getInstance();
		uint16_t lastFrequency = _outputFrequency;
		uint8_t retryCount = 0;

		while (!estop.isTriggered()) {
			refresh();

			GcodeSuite::host_keepalive();

			debug()("currentFrequency: ", _outputFrequency, ", targetFrequency: ", targetFrequency);

			float32_t threshold = (float32_t) targetFrequency * 0.01f;

			debug()("threshold: ", threshold, ", min: ", targetFrequency - threshold, ", max: ", targetFrequency + threshold);

			if (_outputFrequency >= (targetFrequency - threshold) && _outputFrequency <= (targetFrequency + threshold)) {
				break;
			}

			if (lastFrequency == _outputFrequency) {
				retryCount++;

				if (retryCount > 10) {
					throw DriverException("Timed out waiting for spindle.");
				}
			}

			safe_delay(10);
		}
	}

	uint16_t RS485DriverImpl::calculateTargetFrequency() const {
		float32_t overiddenPower = _targetPower * 0.01f * _powerOverride;

		uint16_t maximumFrequency = readMaximumFrequency();

		if (maximumFrequency == 0) {
			return 0;
		}

		uint16_t frequencyUpperLimit = readFrequencyUpperLimit();

		if (frequencyUpperLimit > maximumFrequency) {
			frequencyUpperLimit = maximumFrequency;
		}

		uint16_t frequencyLowerLimit = readFrequencyLowerLimit();

		if (frequencyLowerLimit > maximumFrequency) {
			frequencyLowerLimit = 0;
		}

		debug()("maxPower:", _maxPower, ", overriddenPower: ", overiddenPower, ", maximumFrequency: ", maximumFrequency, ", upperLimit: ", frequencyUpperLimit, ", lowerLimit: ", frequencyLowerLimit);
		/*
		SERIAL_ECHOLNPAIR("maxPower: ", _maxPower);
		SERIAL_ECHOLNPAIR("overriddenPower: ", overiddenPower);
		SERIAL_ECHOLNPAIR("maximumFrequency: ", maximumFrequency);
		SERIAL_ECHOLNPAIR("upperLimit: ", frequencyLowerLimit);
		SERIAL_ECHOLNPAIR("lowerLimit: ", frequencyLowerLimit);
		*/
		// scale overriddenPower to targetFrequency
		uint32_t targetFrequency = (uint16_t) (((float32_t) maximumFrequency / (float32_t) _maxPower) * overiddenPower);

		debug()("targetFrequency: ", targetFrequency);
		// SERIAL_ECHOLNPAIR("targetFrequency: ", targetFrequency);

		targetFrequency = targetFrequency > frequencyUpperLimit ? frequencyUpperLimit : targetFrequency;
		targetFrequency = frequencyLowerLimit > targetFrequency ? frequencyLowerLimit : targetFrequency;
		targetFrequency = _enabled ? targetFrequency : 0;

		debug()("targetFrequency: ", targetFrequency);
		// SERIAL_ECHOLNPAIR("targetFrequency: ", targetFrequency);

		return targetFrequency;
	}

	void RS485DriverImpl::refresh() {
		if (_doReset) {
			writeTargetFrequency(0);
			writeStop();

			_doReset = false;
		}

		_state = readState();
		_fault = readFault();

		auto maximumFrequency = readMaximumFrequency();

		_outputFrequency = readOutputFrequency();
		_outputDirection = (uint8_t) _state & (uint8_t) State::Running ? ((uint8_t) _state & (uint8_t) State::Forward ? Direction::Forward : Direction::Reverse) : Direction::Forward;

		_outputCurrent = readOutputCurrent();
		_outputVoltage = readOutputVoltage();
		_dcBusVoltage = readDCBusVoltage();

		_currentPower = ((((float32_t) _maxPower) * 1.0f) / (float32_t) maximumFrequency) * (float32_t) _outputFrequency;
	}

	void RS485DriverImpl::apply() {
		_targetFrequency = calculateTargetFrequency();

		refresh();

		if (_outputFrequency == _targetFrequency && _outputDirection == _targetDirection) {
			return;
		}

		if (_enabled) {
			if (_hasFan) {
				WRITE(SPINDLE_FAN_PIN, SPINDLE_FAN_ON);
			}

			// if we're changing direction, we need to come to a complete stop first
			if (_outputFrequency > 0 && _outputDirection != _targetDirection) {
				writeTargetFrequency(0);
				writeStop();

				waitForSpindle(0);
			}

			writeTargetFrequency(_targetFrequency);
			writeStart(_targetDirection);

			waitForSpindle(_targetFrequency);
		} else {
			if (_hasFan) {
				auto ms = millis();

				_fanOffTime = ms + _fanTimeoutMs;
			}

			writeTargetFrequency(0);
			writeStop();

			waitForSpindle(0);
		}
	}

	// called in interrupt context
	void RS485DriverImpl::emergencyStop() {
		setEnabled(false);
		setTargetPower(0);
	}

	// called in interrupt context
	void RS485DriverImpl::emergencyClear() {
		_doReset = true;
	}
} // namespace swordfish::tools::drivers
