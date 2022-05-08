/*
 * VFD.h
 *
 * Created: 12/10/2021 9:35:12 am
 *  Author: smohekey
 */

#pragma once

#include "RS485DriverImpl.h"

namespace swordfish::tools::drivers {
	class CT100DriverImpl : public RS485DriverImpl {
	protected:
		enum class ControlCommand : uint16_t {
			Unknown = 0x0000,
			ForwardRunning = 0x0001,
			ReverseRunning = 0x0002,
			ForwardJogging = 0x0003,
			ReverseJogging = 0x0004,
			Stop = 0x0005,
			EmergencyStop = 0x0006,
			FaultReset = 0x0007,
			JoggingStop = 0x0008
		};

		enum class ControlState : uint16_t {
			Unknown = 0x0000,
			ForwardRunning = 0x0001,
			ReverseRunning = 0x0002,
			Standby = 0x0003,
			Fault = 0x0004
		};

		enum class Fault : uint16_t {
			None = 0x0000,
			DCBusUnderVoltage = 0x0001,
			AccelOverVoltage = 0x0002,
			RunningOverVoltage = 0x0003,
			AccelOverCurrent = 0x0004,
			DecelOverCurrent = 0x0005,
			RunningOverCurrent = 0x0006,
			MotorOverLoad = 0x0007,
			InverterOverLoad = 0x0008,
			InverterProtection1 = 0x0009,
			InverterProtection2 = 0x0019,
			InverterProtection3 = 0x0029,
			DecelOverVoltage = 0x000A,
			External = 0x000D,
			DiodeOverHeat = 0x000E,
			EEPROMWrite = 0x000F,
			InputPhase = 0x0012,
			OutputPhase = 0x0013,
			CurrentSensor = 0x0015,
			SelfLearning = 0x0016,
			Communication = 0x0018
		};

		uint16_t readFrequencyPercentage() const;
		void writeControlCommand(ControlCommand controlCommand) const;
		ControlCommand readControlCommand() const;
		ControlState readControlState() const;

		virtual uint16_t readMaximumFrequency() const override;
		virtual uint16_t readFrequencyUpperLimit() const override;
		virtual uint16_t readFrequencyLowerLimit() const override;

		virtual uint16_t readOutputFrequency() const override;
		virtual uint16_t readOutputVoltage() const override;
		virtual uint16_t readOutputCurrent() const override;
		virtual uint16_t readDCBusVoltage() const override;

		virtual State readState() const override;

		virtual uint16_t readFault() const override;

		virtual void writeTargetFrequency(uint16_t targetFrequency) const override;
		virtual void writeStart(Direction direction) const override;
		virtual void writeStop() const override;
	};
} // namespace swordfish::tools::drivers