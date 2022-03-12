/*
 * EmergencyStop.cpp
 *
 * Created: 16/09/2021 8:41:26 am
 *  Author: smohekey
 */ 

#define HAS_Z_BRAKE 1

#include "../../src/module/planner.h"
#include "../../src/module/motion.h"
#include "../../src/feature/spindle_laser.h"
#include "../../src/module/stepper/indirection.h"

#include <Arduino.h>

#include <swordfish/debug.h>
#include <swordfish/modules/tools/ToolsModule.h>

namespace swordfish::estop {
	using namespace swordfish::tools;
	
	EStopModule* EStopModule::__instance = nullptr;
	
	core::Schema EStopModule::__schema = {
		utils::typeName<EStopModule>(),
		&(Module::__schema), {
			
		}, {
			
		}
	};
	
	EStopModule::EStopModule(core::Object* parent) :
		Module(parent),
		_pack(__schema, *this, &(Module::_pack)),
		_estopISR(std::bind(&EStopModule::handleEStop, this), ESTOP_PIN, CHANGE) {
		
	}
	
	void EStopModule::handleEStop() {
		_triggered = READ(ESTOP_PIN) != ESTOP_ENDSTOP_INVERTING;
		
		if(_triggered) {
			auto& toolsModule = ToolsModule::getInstance();
			auto& driver = toolsModule.getCurrentDriver();
			
			debug()("estop triggered");
			
			driver.emergencyStop();
			
			DISABLE_AXIS_Z();
			
			planner.quick_stop();
			
			set_axis_never_homed(X_AXIS);
			set_axis_never_homed(Y_AXIS);
			set_axis_never_homed(Z_AXIS);
		}
	}
	
	void EStopModule::init() {
		SET_INPUT_PULLDOWN(ESTOP_PIN);
		
		_triggered = READ(ESTOP_PIN) != ESTOP_ENDSTOP_INVERTING;
		
		_estopISR.attach();
	}

	swordfish::estop::EStopModule& EStopModule::getInstance(core::Object* parent /*= nullptr*/) {
		return *(__instance ?: __instance = new EStopModule(parent));
	}

}