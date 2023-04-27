/*
 * ChangzouH100Driver.cpp
 *
 * Created: 14/10/2021 12:44:16 pm
 *  Author: smohekey
 */

#include <marlin/libs/modbus.h>

#include <swordfish/debug.h>

#include "ChangzouH100DriverImpl.h"

namespace swordfish::tools::drivers {
	uint32_t ChangzouH100DriverImpl::readMaximumFrequency() const {
		debug()();

		return (uint32_t) readHoldingRegister(3, (uint16_t) 0);
	}

	uint32_t ChangzouH100DriverImpl::readFrequencyUpperLimit() const {
		debug()();

		return (uint32_t) readHoldingRegister(5, (uint16_t) 0);
	}

	uint32_t ChangzouH100DriverImpl::readFrequencyLowerLimit() const {
		debug()();

		return (uint32_t) readHoldingRegister(11, (uint16_t) 0);
	}

	uint32_t ChangzouH100DriverImpl::readOutputFrequency() const {
		debug()();

		return (uint32_t) readInputRegister(0x0000, 0);
	}

	uint32_t ChangzouH100DriverImpl::readOutputVoltage() const {
		debug()();

		return (uint32_t) readInputRegister(0x0005, 0);
	}

	uint32_t ChangzouH100DriverImpl::readOutputCurrent() const {
		debug()();

		return (uint32_t) readInputRegister(0x0002, 0);
	}

	uint32_t ChangzouH100DriverImpl::readDCBusVoltage() const {
		debug()();

		return (uint32_t) readInputRegister(0x0004, 0);
	}

	ChangzouH100DriverImpl::ControlState ChangzouH100DriverImpl::readControlState() const {
		debug()();

		const_cast<ChangzouH100DriverImpl*>(this)->_currentControlState = (ControlState) readHoldingRegister(0x0210, (uint16_t) ControlState::Unknown);

		return _currentControlState;
	}

	void ChangzouH100DriverImpl::writeTargetFrequency(uint32_t targetFrequency) const {
		debug()("targetFrequency: ", targetFrequency);

		return writeHoldingRegister(0x0201, targetFrequency * 10);
	}

	void ChangzouH100DriverImpl::writeStart(Direction targetDirection) const {
		debug()("targetDirection: ", (uint16_t) targetDirection);

		if (targetDirection == Direction::Forward) {
			writeParameter(0x0049, 0xFF00);
		} else {
			writeParameter(0x004A, 0xFF00);
		}

		// writeParameter(0x0049, targetDirection == Direction::Forward ? 0xFF00 : 0x0000);
		// writeParameter(0x004A, targetDirection == Direction::Reverse ? 0xFF00 : 0x0000);

		writeParameter(0x0048, 0xFF00);
	}

	void ChangzouH100DriverImpl::writeStop() const {
		debug()();

		writeParameter(0x004B, 0xFF00);
		writeParameter(0x0048, 0x0000);
	}

	State ChangzouH100DriverImpl::readState() const {
		auto controlState = (uint8_t) readControlState();

		return (State) (((controlState & (uint8_t) ControlState::Running || controlState & (uint8_t) ControlState::InRunning) ? (uint8_t) State::Running : 0) | ((controlState & (uint8_t) ControlState::Reverse || controlState & (uint8_t) ControlState::InReverse) ? 0 : (uint8_t) State::Forward) | (controlState & (uint8_t) ControlState::InBraking ? (uint8_t) State::Braking : 0) | ((controlState & (uint8_t) ControlState::Jogging || controlState & (uint8_t) ControlState::InJogging) ? (uint8_t) State::Jogging : 0));
	}

	uint32_t ChangzouH100DriverImpl::readFault() const {
		debug()();

		return readInputRegister(0x000A, 0);
	}

	void ChangzouH100DriverImpl::clearFault() const {
		debug()();

		// TODO: Clear fault
	}
} // namespace swordfish::tools::drivers
