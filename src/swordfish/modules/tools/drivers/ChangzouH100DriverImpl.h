/*
 * ChangzouH100Driver.h
 *
 * Created: 12/10/2021 9:42:09 am
 *  Author: smohekey
 */

#pragma once

#include <type_traits>

#include "RS485DriverImpl.h"

namespace swordfish::tools::drivers {
	class ChangzouH100DriverImpl : public RS485DriverImpl {
	protected:
		enum class ControlState {
			Unknown = 0,
			Running = (1 << 0),
			Jogging = (1 << 1),
			Reverse = (1 << 2),
			InRunning = (1 << 3),
			InJogging = (1 << 4),
			InReverse = (1 << 5),
			InBraking = (1 << 6),
			Tracking = (1 << 7)
		};

		enum class Fault {
			None = 0,
			RunningOverCurrent_S = 64,
			RunningOverCurrent_A = 65,
			RunningOverCurrent_d = 66,
			RunningOverCurrent_n = 67,
			RunningOverVoltage_S = 80,
			RunningOverVoltage_A = 81,
			RunningOverVoltage_d = 82,
			RunningOverVoltage_n = 83,
			LowVoltage_S = 88,
			LowVoltage_A = 89,
			LowVoltage_d = 90,
			LowVoltage_n = 91,
			InverterOverHeat_S = 92,
			InverterOverHeat_A = 93,
			InverterOverHeat_d = 94,
			InverterOverHeat_n = 95,
			OverLoad_S = 96,
			OverLoad_A = 97,
			OverLoad_d = 98,
			OverLoad_n = 99,
			OverA_S = 100,
			OverA_A = 101,
			OverA_d = 102,
			OverA_n = 103,
			OverT_S = 104,
			OverT_A = 105,
			OverT_d = 106,
			OverT_n = 107
		};

		ControlState readControlState() const;

		ControlState _currentControlState;

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