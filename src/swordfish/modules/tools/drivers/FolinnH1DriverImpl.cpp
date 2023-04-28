/*
 * FulingDZBDriver.cpp
 *
 * Created: 14/10/2021 12:43:48 pm
 *  Author: smohekey
 */

#include <swordfish/modules/estop/EStopModule.h>

#include <marlin/libs/modbus.h>

#include "FolinnH1DriverImpl.h"

namespace swordfish::tools::drivers {
	template<typename T>
	T swap_order(T u) {
		union {
			T u;
			uint16_t u16[sizeof(T) / 2];
		} source, dest;

		source.u = u;

		for (size_t k = 0; k < sizeof(T) / 2; k++)
			dest.u16[k] = source.u16[sizeof(T) / 2 - k - 1];

		return dest.u;
	}

	using namespace swordfish::estop;

	FolinnH1DriverImpl::ControlCommand FolinnH1DriverImpl::readControlCommand() const {
		debug()();

		return (ControlCommand) swap_order(readHoldingRegister(0x43f1, (uint32_t) ControlCommand::None));
	}

	void FolinnH1DriverImpl::writeControlCommand(ControlCommand controlCommand) const {
		debug()();

		writeHoldingRegister(0x8122, (uint16_t) controlCommand);
	}

	FolinnH1DriverImpl::ControlState FolinnH1DriverImpl::readControlState() const {
		debug()();

		return (ControlState) readHoldingRegister(0x03F6, (uint32_t) ControlState::None);
	}

	uint32_t FolinnH1DriverImpl::readMaximumFrequency() const {
		debug()();

		return (uint32_t) readHoldingRegister(0x0264, (uint16_t) 0) * 10;
	}

	uint32_t FolinnH1DriverImpl::readFrequencyUpperLimit() const {
		debug()();

		return swap_order(readHoldingRegister(0x40D9, (uint32_t) 0));
	}

	uint32_t FolinnH1DriverImpl::readFrequencyLowerLimit() const {
		debug()();

		return swap_order(readHoldingRegister(0x40DA, (uint32_t) 0));
	}

	uint32_t FolinnH1DriverImpl::readOutputFrequency() const {
		debug()();

		return swap_order(readHoldingRegister(0x43FB, (uint32_t) 0));
	}

	uint32_t FolinnH1DriverImpl::readOutputVoltage() const {
		debug()();

		return swap_order(readHoldingRegister(0x43FE, (uint32_t) 0));
	}

	uint32_t FolinnH1DriverImpl::readOutputCurrent() const {
		debug()();

		return swap_order(readHoldingRegister(0x43FD, (uint32_t) 0));
	}

	uint32_t FolinnH1DriverImpl::readDCBusVoltage() const {
		debug()();

		return swap_order(readHoldingRegister(0x4400, (uint32_t) 0));
	}

	State FolinnH1DriverImpl::readState() const {
		debug()();

		auto state = State::None;

		auto controlCommand = readControlCommand();
		auto controlState = readControlState();

		if ((uint16_t) controlCommand & (uint16_t) ControlCommand::StartUp) {
			state = (State) ((uint8_t) state | (uint8_t) State::Forward);
		}

		if ((uint8_t) controlState & (uint8_t) ControlState::Running) {
			state = (State) ((uint8_t) state | (uint8_t) State::Running);
		}

		if ((uint8_t) controlState & (uint8_t) ControlState::Fault) {
			state = (State) ((uint8_t) state | (uint8_t) State::Fault);
		}

		return state;
	}

	uint32_t FolinnH1DriverImpl::readFault() const {
		debug()();

		auto fault = swap_order(readHoldingRegister(0x43F7, (uint32_t) 0));

		debug()("fault: ", (uint32_t) fault);

		return (uint32_t) fault;
	}

	void FolinnH1DriverImpl::writeTargetFrequency(uint32_t targetFrequency) const {
		debug()("targetFrequency: ", targetFrequency);

		uint16_t maximumFrequency = readMaximumFrequency();

		if (maximumFrequency == 0) {
			debug()("maximumFrequency can't be zero.");

			return;
		}

		uint32_t targetFrequencyPercentage = ((float32_t) targetFrequency) / (float32_t) maximumFrequency * 1000.0f;

		debug()("targetFrequencyPercentage: ", targetFrequencyPercentage);

		writeHoldingRegister(0x8121, targetFrequencyPercentage);
	}

	void FolinnH1DriverImpl::writeStart(Direction targetDirection) const {
		debug()("targetDirection: ", (uint16_t) targetDirection);

		writeHoldingRegister(0x8122, targetDirection == Direction::Forward ? 0x0001 : 0x0004);
	}

	void FolinnH1DriverImpl::writeStop() const {
		debug()();

		writeHoldingRegister(0x8122, 0x10);
	}
} // namespace swordfish::tools::drivers