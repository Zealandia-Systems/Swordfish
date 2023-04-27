/*
 * VFD.h
 *
 * Created: 12/10/2021 9:35:12 am
 *  Author: smohekey
 */

#pragma once

#include "RS485DriverImpl.h"

namespace swordfish::tools::drivers {
	class FolinnH1DriverImpl : public RS485DriverImpl {
	protected:
		enum class ControlCommand : uint16_t {
			None = 0,
			StartUp = 1 << 0,
			Reverse = 1 << 1,
			StartReverse = 1 << 2,
			Jog = 1 << 3,
			Stop = 1 << 4,
			EmergencyStop = 1 << 5,
			SafeStop = 1 << 6,
			Reset = 1 << 7,
			ParameterSelfLearning = 1 << 9,
			TimeOut = 1 << 11,
			UpIncremental = 1 << 13,
			DownIncremental = 1 << 14
		};

		enum class ControlState : uint16_t {
			None = 0,
			PoweringOff = 1 << 0,
			Stopping = 1 << 1,
			Running = 1 << 2,
			StartFunctionStart = 1 << 3,
			ParameterSelfLearningStart = 1 << 4,
			Operating = 1 << 5,
			Ready = 1 << 6,
			Fault = 1 << 10,
			Alarm = 1 << 11,
			STOStatus = 1 << 12
		};

		enum class Fault : uint32_t {
			None = 0,
			SystemAbnormality = 1 << 1,
			GroundFault = 1 << 4,
			ShortCircuitToGround = 1 << 5,
			OutputShortCircuit = 1 << 6,
			OutputOverCurrent = 1 << 7,
			DCBusOverVoltage = 1 << 8,
			DCBusUnderVoltage = 1 << 9,
			InverterOverheating = 1 << 10,
			RectifierBridgeOverheating = 1 << 13,
			UPhaseMissingPhase = 1 << 14,
			VPhaseMissingPhase = 1 << 15,
			WPhaseMissingPhase = 1 << 16,
			NoMotorConnection = 1 << 19,
			InputPhaseLoss = 1 << 20,
			InverterOverload = 1 << 21,
			OverTorque = 1 << 22,
			MotorOverheating = 1 << 24,
			MotorOverload = 1 << 25,
			CurrentLimit = 1 << 26,
			InputPowerDown = 1 << 27
		};

		ControlCommand readControlCommand() const;
		void writeControlCommand(ControlCommand controlCommand) const;
		ControlState readControlState() const;

		// uint16_t readMaximumPercentage() const;
		// uint16_t readMinimumPercentage() const;

		// uint16_t readMaximumPower() const;
		// virtual uint16_t getMaximumPower() const override;

		virtual uint32_t readMaximumFrequency() const override;
		virtual uint32_t readFrequencyUpperLimit() const override;
		virtual uint32_t readFrequencyLowerLimit() const override;

		virtual uint32_t readOutputFrequency() const override;
		virtual uint32_t readOutputVoltage() const override;
		virtual uint32_t readOutputCurrent() const override;
		virtual uint32_t readDCBusVoltage() const override;

		virtual State readState() const override;

		virtual uint32_t readFault() const override;
		virtual void clearFault() const override;

		virtual void writeTargetFrequency(uint32_t targetFrequency) const override;
		virtual void writeStart(Direction direction) const override;
		virtual void writeStop() const override;
	};
} // namespace swordfish::tools::drivers