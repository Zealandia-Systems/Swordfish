/*
 * EmergencyStop.cpp
 *
 * Created: 16/09/2021 8:41:26 am
 *  Author: smohekey
 */

#define HAS_Z_BRAKE 1

#include <marlin/gcode/gcode.h>
#include <marlin/module/planner.h>
#include <marlin/module/motion.h>
#include <marlin/module/stepper.h>
#include <marlin/feature/spindle_laser.h>
#include <marlin/module/stepper/indirection.h>

#include <Arduino.h>

#include <swordfish/debug.h>
#include <swordfish/modules/status/StatusModule.h>
#include <swordfish/modules/tools/ToolsModule.h>

namespace swordfish::estop {
	using namespace swordfish::tools;
	using namespace swordfish::status;

	EStopModule* EStopModule::__instance = nullptr;

	core::Schema EStopModule::__schema = {
		utils::typeName<EStopModule>(),
		&(Module::__schema),
		{

		},
		{

		}
	};

	EStopModule::EStopModule(core::Object* parent) :
			Module(parent),
			_pack(__schema, *this, &(Module::_pack)),
			_estopISR(std::bind(&EStopModule::handleEStop, this), ESTOP_PIN, CHANGE, false),
			_triggered(readPin()) {

		if (_triggered) {
			StatusModule::getInstance().set_state(MachineState::EmergencyStop);
		}
	}

	void EStopModule::idle() {
		checkOrClear();
	}

	void EStopModule::handleEStop() {
		auto& toolsModule = ToolsModule::getInstance();
		auto& driver = toolsModule.getCurrentDriver();

		if (!_triggered && readPin()) {
			_triggered = true;

			debug()("estop triggered");

			driver.emergencyStop();

			DISABLE_AXIS_Z();

			stepper.suspend();
			planner.quick_stop();
			planner.clear_block_buffer();
			queue.clear();
			gcode.abort_current();

			set_axis_never_homed(X_AXIS);
			set_axis_never_homed(Y_AXIS);
			set_axis_never_homed(Z_AXIS);

			auto& statusModule = StatusModule::getInstance();

			statusModule.set_state(MachineState::EmergencyStop);
		}
	}

	bool EStopModule::checkOrClear() {
		__disable_irq();

		if (/*_triggered && */readPin()) {
			__enable_irq();

			return false;
		}

		if (_triggered) {
			_triggered = false;

			auto& toolsModule = ToolsModule::getInstance();
			auto& statusModule = StatusModule::getInstance();
			auto& driver = toolsModule.getCurrentDriver();

			stepper.wake_up();
			driver.emergencyClear();
			statusModule.set_state(MachineState::Idle);

			debug()("estop cleared");
		}

		__enable_irq();

		return true;
	}

	void EStopModule::throwIfTriggered() {
		GcodeSuite::throwIfAborted();

		if (!checkOrClear()) {
			throw EStopException();
		}
	}

	void EStopModule::init() {
		SET_INPUT_PULLDOWN(ESTOP_PIN);

		_estopISR.attach();
	}

	swordfish::estop::EStopModule& EStopModule::getInstance(core::Object* parent /*= nullptr*/) {
		return *(__instance ?: __instance = new EStopModule(parent));
	}

} // namespace swordfish::estop