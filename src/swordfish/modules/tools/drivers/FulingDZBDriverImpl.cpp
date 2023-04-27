/*
 * FulingDZBDriver.cpp
 *
 * Created: 14/10/2021 12:43:48 pm
 *  Author: smohekey
 */

#include <swordfish/modules/estop/EStopModule.h>

#include <marlin/libs/modbus.h>

#include "FulingDZBDriverImpl.h"

namespace swordfish::tools::drivers {
	using namespace swordfish::estop;

	FulingDZBDriverImpl::ControlCommand FulingDZBDriverImpl::readControlCommand() const {
		debug()();

		return (ControlCommand) readHoldingRegister(0x1000, (uint16_t) ControlCommand::Unknown);
	}

	void FulingDZBDriverImpl::writeControlCommand(ControlCommand controlCommand) const {
		debug()();

		writeHoldingRegister(0x1000, (uint16_t) controlCommand);
	}

	FulingDZBDriverImpl::ControlState FulingDZBDriverImpl::readControlState() const {
		debug()();

		return (ControlState) readHoldingRegister(0x1001, (uint16_t) ControlState::Unknown);
	}

	uint16_t FulingDZBDriverImpl::readFrequencyPercentage() const {
		debug()();

		return readHoldingRegister(0x2000, (uint16_t) 0);
	}

	uint32_t FulingDZBDriverImpl::readMaximumFrequency() const {
		debug()();

		return (uint32_t) readHoldingRegister(4, (uint16_t) 0);
	}

	uint32_t FulingDZBDriverImpl::readFrequencyUpperLimit() const {
		debug()();

		return (uint32_t) readHoldingRegister(5, (uint16_t) 0);
	}

	uint32_t FulingDZBDriverImpl::readFrequencyLowerLimit() const {
		debug()();

		return (uint32_t) readHoldingRegister(6, (uint16_t) 0);
	}

	uint32_t FulingDZBDriverImpl::readOutputFrequency() const {
		debug()();

		return (uint32_t) readHoldingRegister(0x3001, (uint16_t) 0);
	}

	uint32_t FulingDZBDriverImpl::readOutputVoltage() const {
		debug()();

		return (uint32_t) readHoldingRegister(0x3003, 0);
	}

	uint32_t FulingDZBDriverImpl::readOutputCurrent() const {
		debug()();

		return (uint32_t) readHoldingRegister(0x3002, 0);
	}

	uint32_t FulingDZBDriverImpl::readDCBusVoltage() const {
		debug()();

		return (uint32_t) readHoldingRegister(0x3007, 0);
	}

	State FulingDZBDriverImpl::readState() const {
		debug()();

		auto controlState = readControlState();

		switch (controlState) {
			case ControlState::ForwardRunning: {
				return (State) ((uint8_t) State::Running | (uint8_t) State::Forward);
			}

			case ControlState::ReverseRunning: {
				return State::Running;
			}

			case ControlState::Standby: {
				return State::None;
			}

			case ControlState::Fault: {
				return State::Fault;
			}

			default: {
				return State::Fault;
			}
		}
	}

	uint32_t FulingDZBDriverImpl::readFault() const {
		debug()();

		auto fault = readHoldingRegister(0x5000, 0);

		debug()("fault: ", fault);

		if (fault == 17) {
			auto& estopModule = EStopModule::getInstance();

			debug()("External fault detected.");

			if (!estopModule.isTriggered()) {
				writeHoldingRegister(0x1000, 0x0007);
			}
		}

		return fault;
	}

	void FulingDZBDriverImpl::writeTargetFrequency(uint32_t targetFrequency) const {
		debug()("targetFrequency: ", targetFrequency);

		uint16_t maximumFrequency = readMaximumFrequency();

		if (maximumFrequency == 0) {
			debug()("maximumFrequency can't be zero.");

			return;
		}

		uint16_t targetFrequencyPercentage = ((float32_t) targetFrequency) / (float32_t) maximumFrequency * 10000.0f;

		debug()("targetFrequencyPercentage: ", targetFrequencyPercentage);

		writeHoldingRegister(0x2000, targetFrequencyPercentage);
	}

	void FulingDZBDriverImpl::writeStart(Direction targetDirection) const {
		debug()("targetDirection: ", (uint16_t) targetDirection);

		writeHoldingRegister(0x1000, targetDirection == Direction::Forward ? 0x0001 : 0x0002);
	}

	void FulingDZBDriverImpl::writeStop() const {
		debug()();

		writeHoldingRegister(0x1000, 0x0005);
	}

	void FulingDZBDriverImpl::clearFault() const {
		writeHoldingRegister(0x1000, 0x0007);
	}
} // namespace swordfish::tools::drivers