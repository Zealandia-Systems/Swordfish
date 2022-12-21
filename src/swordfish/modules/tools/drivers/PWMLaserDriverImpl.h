/*
 * PWMLaserImpl.h
 *
 * Created: 27/10/2021 8:29:46 am
 *  Author: smohekey
 */

#pragma once

#include "../IDriver.h"

namespace swordfish::tools::drivers {
	class PWMLaserDriverImpl : public IDriver {
	protected:
		uint32_t _cyclesPerPeriod;

		bool _enabled;

		float32_t _targetPower = 0; // in %

		float32_t _powerOverride = 100.0f;

		float32_t _currentPower;

	public:
		virtual void init(uint16_t index, DriverParameterTable& parameters);

		~PWMLaserDriverImpl() {
		}

		virtual State getState() const override {
			return _enabled ? State::Running : State::None;
		}

		uint16_t getMaxPower() const;

		virtual void idle() override;

		virtual bool isEnabled() const override;

		virtual void setEnabled(bool enabled) override;

		virtual bool hasDirection() const override {
			return false;
		}

		virtual Direction getTargetDirection() const override {
			return Direction::Forward;
		}

		virtual void setTargetDirection([[maybe_unused]] const Direction direction) override {
		}

		virtual Direction getCurrentDirection() const override {
			return Direction::Forward;
		}

		virtual uint32_t getMaximumPower() const override {
			return 100;
		}

		virtual float32_t getTargetPower() const override;

		virtual void setTargetPower(float32_t targetPower) override;

		virtual float32_t getCurrentPower() const override;

		virtual uint32_t getOutputFrequency() const override;

		virtual float32_t getPowerOverride() const override;

		virtual void setPowerOverride(float32_t powerOverride) override;

		virtual void apply() override;

		virtual void emergencyStop() override {
		}
		virtual void emergencyClear() override {
		}
	};
} // namespace swordfish::tools::drivers