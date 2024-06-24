/*
 * StatusModule.h
 *
 * Created: 30/09/2021 8:52:21 am
 *  Author: smohekey
 */


#pragma once

#include <swordfish/Module.h>
#include <swordfish/core/Schema.h>
#include <swordfish/utils/TypeInfo.h>
#include <swordfish/modules/estop/EStopModule.h>

#include "WS2812Driver.h"

namespace swordfish::status {
	enum class MachineState {
		Idle,
		EmergencyStop,
		FeedMove,
		RapidMove,
		Homing,
		AwaitingInput,
		SpindleRamp,
		Probing,
	};

	class StatusModule : public Module {
		private:
			static core::ValidatedValueField<uint16_t> led_count_field__;
			static core::ValidatedValueField<uint8_t> led_brightness_field__;
			static core::ValidatedValueField<uint16_t> led_sweep_time_field__;

			static StatusModule* instance__;

			static void led_count_changed(uint16_t old_led_count, uint16_t led_count);
			static void led_brightness_changed(uint8_t old_led_brightness, uint8_t led_brightness);
			static void led_sweep_time_changed(uint16_t old_led_sweep_time, uint16_t led_sweep_time);

			StatusModule(core::Object* parent);

		protected:
			friend WS2812Driver;

			static core::Schema schema__;

			core::Pack pack_;
			WS2812Driver driver_;
			//std::vector<MachineState> state_;
			MachineState state_;

			core::Pack& getPack() override {
				return pack_;
			}

			void update();

		public:
			virtual ~StatusModule() { }

			WS2812Driver& driver() {
				return driver_;
			}

			void set_state(MachineState state);
			//void push_state(MachineState state);
			MachineState& peek_state();
			//void pop_state();

			virtual const char* name() override { return "Status"; }
			virtual void init() override;
			virtual void read(io::InputStream& stream) override;

			static StatusModule& getInstance(core::Object* parent = nullptr);
	};

	class TemporaryState {
		private:
			MachineState old_state_;

		public:
			TemporaryState(MachineState temporary_state) {
				auto& status_module = StatusModule::getInstance();

				old_state_ = status_module.peek_state();

				status_module.set_state(temporary_state);
			}

			~TemporaryState() {
				restore();
			}

			inline void restore() {
				if (!swordfish::estop::EStopModule::getInstance().checkOrClear()) {
					StatusModule::getInstance().set_state(MachineState::EmergencyStop);
				} else {
					StatusModule::getInstance().set_state(old_state_);
				}
			}
	};
}