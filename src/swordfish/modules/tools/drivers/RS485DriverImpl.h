/*
 * VariableFrequencyDriver.h
 *
 * Created: 14/10/2021 12:48:04 pm
 *  Author: smohekey
 */

#pragma once

#include <Arduino.h>

#include <swordfish/debug.h>

#include <marlin/core/serial.h>
#include <marlin/libs/modbus.h>
#include "../IDriver.h"

namespace swordfish::tools::drivers {
	class RS485DriverImpl : public IDriver {
	private:
		uint32_t _nextRefresh = 0;
		uint32_t _fanTimeoutMs = 60000;
		int32_t _fanOffTime = 0;
		bool _doReset = true;

	protected:
		uint8_t _slaveAddress = 1;
		uint32_t _baudRate = 9600;

		uint16_t _maxPower = 0;
		bool _hasFan = false;
		bool _enabled = false;

		uint16_t _targetFrequency = 0;
		Direction _targetDirection = Direction::Forward;
		float32_t _targetPower = 0; // in RPM

		float32_t _powerOverride = 100.0f;

		uint16_t _outputFrequency = 0;
		Direction _outputDirection = Direction::Forward;
		uint16_t _outputVoltage = 0;
		uint16_t _outputCurrent = 0;
		uint16_t _dcBusVoltage = 0;

		float32_t _currentPower = 0;

		State _state = State::None;
		uint16_t _fault = 0;

		template<typename T>
		T readInputRegister(uint16_t address, T defaultValue) const {
			uint16_t result;

			debug()("address: ", address);

			if (1 != modbus::read_input_registers(_slaveAddress, address, 1, &result)) {
				debug()("no result");

				return defaultValue;
			}

			debug()("result: ", result);

			safe_delay(10);

			return (T) result;
		}

		template<typename T>
		T readHoldingRegister(uint16_t address, T defaultValue) const {
			uint16_t result;

			debug()("address: ", address);

			if (1 != modbus::read_holding_registers(_slaveAddress, address, 1, &result)) {
				debug()("no result");

				return defaultValue;
			}

			debug()("result: ", result);

			safe_delay(10);

			return (T) result;
		}

		template<typename T>
		void writeHoldingRegister(uint16_t address, T value) const {
			debug()("address: ", address);

			debug()("value: ", (uint16_t) value);

			modbus::write_holding_register(_slaveAddress, address, (uint16_t) value);

			safe_delay(10);
		}

		template<typename T>
		void writeParameter(uint16_t address, T value) const {
			debug()("address: ", address);

			debug()("value: ", (uint16_t) value);

			modbus::write_parameter(_slaveAddress, address, (uint16_t) value);

			safe_delay(10);
		}

		virtual uint16_t readMaximumFrequency() const = 0;
		virtual uint16_t readFrequencyUpperLimit() const = 0;
		virtual uint16_t readFrequencyLowerLimit() const = 0;

		virtual uint16_t readOutputFrequency() const = 0;
		virtual uint16_t readOutputCurrent() const = 0;
		virtual uint16_t readOutputVoltage() const = 0;
		virtual uint16_t readDCBusVoltage() const = 0;

		virtual State readState() const = 0;

		virtual uint16_t readFault() const = 0;

		virtual void writeTargetFrequency(uint16_t targetFrequency) const = 0;
		virtual void writeStart(Direction targetDirection) const = 0;
		virtual void writeStop() const = 0;

		virtual uint16_t calculateTargetFrequency() const;

		virtual void waitForSpindle(uint16_t targetFrequency);

		virtual void refresh();

	public:
		virtual void init(uint16_t index, DriverParameterTable& parameters);

		~RS485DriverImpl() {
		}

		virtual State getState() const override {
			return _state;
		}

		uint8_t getSlaveAddress() const;

		uint16_t getMaxPower() const;

		virtual void idle() override;

		virtual bool isEnabled() const override;

		virtual void setEnabled(bool enabled) override;

		virtual bool hasDirection() const override;

		virtual Direction getTargetDirection() const override;

		virtual void setTargetDirection(const Direction direction) override;

		virtual Direction getCurrentDirection() const override;

		virtual float32_t getTargetPower() const override;

		virtual void setTargetPower(float32_t targetPower) override;

		virtual float32_t getCurrentPower() const override;

		virtual uint16_t getOutputFrequency() const override;

		virtual float32_t getPowerOverride() const override;

		virtual void setPowerOverride(float32_t powerOverride) override;

		virtual void apply() override;

		virtual void emergencyStop() override;
		virtual void emergencyClear() override;
	};
} // namespace swordfish::tools::drivers