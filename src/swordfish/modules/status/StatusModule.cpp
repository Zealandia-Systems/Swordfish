/*
 * StatusModule.cpp
 *
 * Created: 30/09/2021 9:48:01 am
 *  Author: smohekey
 */

#include "Color.h"
#include "StatusModule.h"
#include <swordfish/modules/estop/EStopModule.h>
#include <marlin/gcode/gcode.h>

namespace swordfish::status {
	using swordfish::estop::EStopModule;

	core::ValidatedValueField<uint16_t> StatusModule::led_count_field__ = { "ledCount", 0, 0, led_count_changed };
	core::ValidatedValueField<uint8_t> StatusModule::led_brightness_field__ = { "ledBrightness", 2, 128, led_brightness_changed };
	core::ValidatedValueField<uint16_t> StatusModule::led_sweep_time_field__ { "ledSweepTime", 3, 2000, led_sweep_time_changed };

	StatusModule* StatusModule::instance__ = nullptr;

	core::Schema StatusModule::schema__ = {
		utils::typeName<StatusModule>(),
		&(Module::__schema), {
			led_count_field__,
			led_brightness_field__,
			led_sweep_time_field__,
		}, {

		}
	};

	void StatusModule::led_count_changed(uint16_t old_led_count, uint16_t led_count) {
		auto& status_module = StatusModule::getInstance();

		status_module.driver_.set_led_count(led_count);
	}

	void StatusModule::led_brightness_changed(uint8_t old_led_brightness, uint8_t led_brightness) {
		auto& status_module = StatusModule::getInstance();

		status_module.driver_.set_brightness(led_brightness);
	}

	void StatusModule::led_sweep_time_changed(uint16_t old_led_sweep_time, uint16_t led_sweep_time) {
		auto& status_module = StatusModule::getInstance();

		status_module.driver_.set_sweep_time(led_sweep_time);
	}

	StatusModule::StatusModule(core::Object* parent) :
		Module(parent),
		pack_(schema__, *this, &(Module::_pack)),
		driver_(led_count_field__.get(pack_), led_brightness_field__.get(pack_), led_sweep_time_field__.get(pack_)),
		state_ { MachineState::Idle } {

	}

	void StatusModule::update() {
		switch (state_) {
			case MachineState::Idle: {
				//debug()("MachineState::Idle");

				driver_.set_color(0, 255, 0);
				driver_.set_sweep(true);

				break;
			}

			case MachineState::EmergencyStop: {
				//debug()("MachineState::EmergencyStop");

				driver_.set_color(255, 0, 0);
				driver_.set_sweep(false);

				break;
			}

			case MachineState::FeedMove: {
				//debug()("MachineState::FeedMove");

				driver_.set_color(0, 255, 0);
				driver_.set_sweep(false);

				break;
			}

			case MachineState::RapidMove: {
				//debug()("MachineState::RapidMove");

				driver_.set_color(128, 128, 0);
				driver_.set_sweep(false);

				break;
			}

			case MachineState::Homing: {
				//debug()("MachineState::Homing");

				driver_.set_color(0, 0, 255);
				driver_.set_sweep(true);

				break;
			}

			case MachineState::AwaitingInput: {
				//debug()("MaachineState::AwaitingInput");

				driver_.set_color(0, 128, 128);
				driver_.set_sweep(false);

				break;
			}

			case MachineState::SpindleRamp: {
				//debug()("MachineState::SpindleRamp");

				driver_.set_color(128, 128, 128);
				driver_.set_sweep(false);

				break;
			}

			case MachineState::Probing: {
				//debug()("MachineState::Probing");

				driver_.set_color(16, 16, 255);
				driver_.set_sweep(true);

				break;
			}
		}
	}

	void StatusModule::set_state(MachineState state) {
		__disable_irq();

		//state_.clear();
		//state_.push_back(state);
		state_ = state;

		update();

		__enable_irq();
	}

	/*void StatusModule::push_state(MachineState state) {
		__disable_irq();

		state_.push_back(state);

		update();

		__enable_irq();
	}*/

	MachineState& StatusModule::peek_state() {
		return state_;
	}

	/*void StatusModule::pop_state() {
		__disable_irq();

		if (state_.size() > 1) {
			state_.pop_back();

			update();
		} else {
			//debug()("no state to pop");
		}

		__enable_irq();
	}*/

	void StatusModule::read(io::InputStream& stream) {
		Module::read(stream);

		driver_.set_led_count(led_count_field__.get(pack_));
		driver_.set_brightness(led_brightness_field__.get(pack_));
		driver_.set_sweep_time(led_sweep_time_field__.get(pack_));
	}

	void StatusModule::init() {
		update();

		driver_.init();
	}

	swordfish::status::StatusModule& StatusModule::getInstance(core::Object* parent) {
		return *(instance__ ?: instance__ = new StatusModule(parent));
	}
}

