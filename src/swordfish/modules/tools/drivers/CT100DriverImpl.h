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
			Stop = 0x0003,
			EmergencyStop = 0x0004,
			FaultReset = 0x0005,
			ForwardJogging = 0x0006,
			ReverseJogging = 0x0007,
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
			None = 0,
			AccelOverCurrent = 1,
			DecelOverCurrent = 2,
			RunningOverCurrent = 3,
			AccelOverVoltage = 4,
			DecelOverVoltage = 5,
			RunningOverVoltage = 6,
			BusUnderVoltage = 7,
			UPhaseProtection = 8,
			VPhaseProtection = 9,
			WPhaseProtection = 10,
			MotorOverload = 11,
			InverterOverload = 12,
			OverloadPreWarning = 13,
			Radiator1OverHeating = 14,
			Radiator2OverHeating = 15,
			InputSidePhaseLoss = 16,
			OutputSidePhaseLoss = 17,
			BrakingUnit = 18,
			AutoTuning = 19,
			PIDDisconnection = 20,
			Communication = 21,
			External = 22,
			EEPROM = 23,
			EndOfLife = 24,
			CurrentDetection = 25
		};

		uint32_t readFrequencyPercentage() const;
		void writeControlCommand(ControlCommand controlCommand) const;
		ControlCommand readControlCommand() const;
		ControlState readControlState() const;

		virtual uint32_t readMaximumFrequency() const override;
		virtual uint32_t readFrequencyUpperLimit() const override;
		virtual uint32_t readFrequencyLowerLimit() const override;

		virtual uint32_t readOutputFrequency() const override;
		virtual uint32_t readOutputVoltage() const override;
		virtual uint32_t readOutputCurrent() const override;
		virtual uint32_t readDCBusVoltage() const override;

		virtual State readState() const override;

		virtual uint32_t readFault() const override;

		virtual void writeTargetFrequency(uint32_t targetFrequency) const override;
		virtual void writeStart(Direction direction) const override;
		virtual void writeStop() const override;
	};
} // namespace swordfish::tools::drivers