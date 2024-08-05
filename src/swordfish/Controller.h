/*
 * Controller.h
 *
 * Created: 8/08/2021 1:32:05 pm
 *  Author: smohekey
 */

#pragma once

#include <array>

#include <swordfish/math.h>
#include <swordfish/utils/NotCopyable.h>
#include <swordfish/utils/NotMovable.h>
#include <swordfish/utils/TypeInfo.h>
#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/modules/tools/ToolsModule.h>
#include <swordfish/modules/motion/MotionModule.h>
#include <swordfish/modules/estop/EStopModule.h>
#include <swordfish/modules/gcode/CommandException.h>
#include <swordfish/modules/gpio/GPIOModule.h>
#include <swordfish/modules/status/StatusModule.h>

#include "PersistentStore.h"

namespace swordfish {
	namespace io {
		class WrappingInputStream;
	}

	class Controller : public core::Object {
	private:
		static tools::ToolsModule* getToolsModule([[maybe_unused]] Object* parent);

		static motion::MotionModule* getMotionModule([[maybe_unused]] Object* parent);

		static estop::EStopModule* getEStopModule([[maybe_unused]] Object* parent);

		static gpio::GPIOModule* getGPIOModule([[maybe_unused]] Object* parent);

		static status::StatusModule* getStatusModule([[maybe_unused]] Object* parent);

		static core::ObjectField<tools::ToolsModule> __toolingModuleField;
		static core::ObjectField<motion::MotionModule> __motionModuleField;
		static core::ObjectField<estop::EStopModule> __estopModuleField;
		static core::ObjectField<gpio::GPIOModule> __gpioModuleField;
		static core::ObjectField<status::StatusModule> __statusModuleField;

		static Controller* __instance;

		Controller();

		bool findNewestConfig(PersistentStoreInputStream& store, io::WrappingInputStream& stream, offset_t& offset);
		void loadConfig(PersistentStoreInputStream& store, io::WrappingInputStream& stream, offset_t offset);

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		uint32_t _configVersion;
		uint32_t _configStart;
		uint32_t _configEnd;

		std::array<Module*, 5> _modules;

		virtual core::Pack& getPack() override;

	public:
		virtual ~Controller() {

		}

		void init();
		void idle();
		void load();
		void save();
		void reset();

		static Controller& getInstance();
	};
}