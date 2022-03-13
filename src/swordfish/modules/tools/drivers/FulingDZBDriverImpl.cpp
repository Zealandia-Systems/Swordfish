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
		
		return readHoldingRegister(0x1000, ControlCommand::Unknown);
	}
	
	void FulingDZBDriverImpl::writeControlCommand(ControlCommand controlCommand) const {
		debug()();
		
		writeHoldingRegister(0x1000, controlCommand);
	}
	
	FulingDZBDriverImpl::ControlState FulingDZBDriverImpl::readControlState() const {
		debug()();
		
		return readHoldingRegister(0x1001, ControlState::Unknown);
	}
	
	uint16_t FulingDZBDriverImpl::readFrequencyPercentage() const {
		debug()();
		
		return readHoldingRegister(0x2000, (uint16_t)0);
	}

	uint16_t FulingDZBDriverImpl::readMaximumFrequency() const {
		debug()();
		
		return readHoldingRegister(4, (uint16_t)0);
	}
	
	uint16_t FulingDZBDriverImpl::readFrequencyUpperLimit() const {
		debug()();
		
		return readHoldingRegister(5, (uint16_t)0);
	}

	uint16_t FulingDZBDriverImpl::readFrequencyLowerLimit() const {
		debug()();
		
		return readHoldingRegister(6, (uint16_t)0);
	}

	uint16_t FulingDZBDriverImpl::readOutputFrequency() const {
		debug()();
		
		return readHoldingRegister(0x3001, (uint16_t)0);
	}
	
	uint16_t FulingDZBDriverImpl::readOutputVoltage() const {
		debug()();
		
		return readHoldingRegister(0x3003, 0);
	}
	
	uint16_t FulingDZBDriverImpl::readOutputCurrent() const {
		debug()();
		
		return readHoldingRegister(0x3002, 0);
	}
	
	uint16_t FulingDZBDriverImpl::readDCBusVoltage() const {
		debug()();
		
		return readHoldingRegister(0x3007, 0);
	}
	
	State FulingDZBDriverImpl::readState() const {
		debug()();
		
		auto controlState = (uint8_t)readControlState();
		
		return (State)(
				((controlState & (uint8_t)ControlState::ForwardRunning || controlState & (uint8_t)ControlState::ReverseRunning) ? (uint8_t)State::Running : 0)
			| (controlState & (uint8_t)ControlState::ForwardRunning ? (uint8_t)State::Forward : 0)
			| (controlState & (uint8_t)ControlState::Fault ? (uint8_t)State::Fault : 0)
		);
	}
	
	uint16_t FulingDZBDriverImpl::readFault() const {
		debug()();
		
		auto fault = readHoldingRegister(0x5000, 0);
		
		debug()("fault: ", fault);
		
		if(fault == 17) {
			auto& estopModule = EStopModule::getInstance();
			
			debug()("External fault detected.");
			
			if(!estopModule.isTriggered()) {
				writeHoldingRegister(0x1000, 0x0007);
			}
		}
		
		return fault;
	}
	
	void FulingDZBDriverImpl::writeTargetFrequency(uint16_t targetFrequency) const {
		debug()("targetFrequency: ", targetFrequency);
			
		uint16_t maximumFrequency = readMaximumFrequency();
			
		if(maximumFrequency == 0) {
			debug()("maximumFrequency can't be zero.");
			
			return;
		}
			
		uint16_t targetFrequencyPercentage = ((float32_t)targetFrequency) / (float32_t)maximumFrequency * 10000.0f;
			
		debug()("targetFrequencyPercentage: ", targetFrequencyPercentage);
			
		writeHoldingRegister(0x2000, targetFrequencyPercentage);
	}
		
	void FulingDZBDriverImpl::writeStart(Direction targetDirection) const {
		debug()("targetDirection: ", (uint16_t)targetDirection);
			
		writeHoldingRegister(0x1000, targetDirection == Direction::Forward ? 0x0001 : 0x0002);
	}

	void FulingDZBDriverImpl::writeStop() const {
		debug()();
			
		writeHoldingRegister(0x1000, 0x0005);
	}
}