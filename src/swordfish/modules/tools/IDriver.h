/*
 * Driver.h
 *
 * Created: 12/08/2021 10:03:12 am
 *  Author: smohekey
 */ 

#pragma once

#include <memory>
#include <type_traits>

#include <swordfish/types.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/NotCopyable.h>
#include <swordfish/utils/NotMovable.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Schema.h>

#include "DriverParameterTable.h"

namespace swordfish::tools {
	enum class Direction : uint8_t {
		Forward,
		Reverse
	};
	
	enum class State : uint8_t {
		None = 0,
		Running = (1<<0), // Running = 1, Stopped = 0
		Forward = (1<<1), // Forward = 1, Reverse = 0
		Braking = (1<<2), // Braking = 1, Not Braking = 0,
		Fault = (1<<3), // Faulted = 1, Not Faulted = 0
		Jogging = (1<<4) // Jogging = 1, Not Jogging = 0
	};
	
	class IDriver  {
	public:
		enum class Parameter : uint16_t {
			RS485_SlaveAddress = 1,
			RS485_BaudRate = 2,
			MaxPower = 10,
			HasFan = 11,
			FanTimeout = 12
		};
		
		virtual void init(uint16_t index, DriverParameterTable& parameters) = 0;
		
		virtual void idle() = 0;
		virtual State getState() const = 0;
		
		virtual bool hasDirection() const = 0;
		virtual Direction getTargetDirection() const = 0;
		virtual void setTargetDirection(Direction direction) = 0;
		virtual Direction getCurrentDirection() const = 0;
		
		virtual bool isEnabled() const = 0;
		virtual void setEnabled(bool enabled) = 0;
		
		virtual float32_t getTargetPower() const = 0;
		virtual void setTargetPower(float32_t power) = 0;
		virtual float32_t getCurrentPower() const = 0;
		
		virtual uint16_t getOutputFrequency() const = 0;
		
		virtual float32_t getPowerOverride() const = 0;
		virtual void setPowerOverride(float32_t powerOverride) = 0;
		
		virtual void apply() = 0;
		
		virtual void emergencyStop() = 0;
	};
	
	class NullDriver : public IDriver {
	public:
		virtual void init([[maybe_unused]] uint16_t index, [[maybe_unused]] DriverParameterTable& parameters) override { }
		virtual void idle() override { }
		virtual State getState() const override { return State::None; }
		virtual bool hasDirection() const override { return false; }
		virtual Direction getTargetDirection() const override { return Direction::Forward; }
		virtual void setTargetDirection([[maybe_unused]] Direction direction) override { }
		virtual Direction getCurrentDirection() const override { return Direction::Forward; }
		virtual bool isEnabled() const override { return false; }
		virtual void setEnabled([[maybe_unused]] bool enabled) override { }
		virtual float32_t getTargetPower() const override { return 0; }
		virtual void setTargetPower([[maybe_unused]] float32_t power) override { }
		virtual float32_t getCurrentPower() const override { return 0; }
		virtual uint16_t getOutputFrequency() const override { return 0; }
		virtual float32_t getPowerOverride() const override;
		virtual void setPowerOverride([[maybe_unused]] float32_t powerOverride) override { }
		virtual void apply() { }
		virtual void emergencyStop() { }
	};
}