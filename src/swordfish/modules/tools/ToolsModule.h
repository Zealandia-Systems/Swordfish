/*
 * Module.h
 *
 * Created: 8/08/2021 5:08:35 pm
 *  Author: smohekey
 */

#pragma once

#include <bitset>
#include <memory>

#include <swordfish/Module.h>
#include <swordfish/types.h>

#include <swordfish/io/Writer.h>

#include <swordfish/core/ObjectList.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/ISR.h>

#include <swordfish/utils/TypeInfo.h>

#include "PocketTable.h"
#include "Pocket.h"
#include "ToolTable.h"
#include "Tool.h"
//#include "Driver.h"
#include "DriverTable.h"
#include "DriverParameter.h"
#include "DriverParameterTable.h"

namespace swordfish::tools {
	class Driver;

	class ToolsModule : public swordfish::Module {
	private:
		static constexpr uint32_t EjectSensorFlag = 0;
		inline static constexpr uint32_t LockSensorFlag = 1;
		inline static constexpr uint32_t ManualSwitchFlag = 2;
		inline static constexpr uint32_t CaddySensorFlag = 3;
		inline static constexpr uint32_t UnlockedFlag = 4;
		inline static constexpr uint32_t HomingFlag = 5;
		inline static constexpr uint32_t IgnoreCaddySensorFlag = 6;

		inline static constexpr float32_t CaddyClearanceX = 50.0f;
		inline static constexpr float32_t ToolClipClearanceX = 30.0f;

		static core::ValueField<bool> __automaticField;
		static core::ObjectField<PocketTable> __pocketsField;
		static core::ObjectField<ToolTable> __toolsField;
		static core::ObjectField<DriverTable> __driversField;
		static core::ObjectField<DriverParameterTable> __driverParametersField;

		static ToolsModule* __instance;

		static NullDriver __nullDriver;

		ToolsModule(core::Object* parent);

		std::bitset<8> _flags = 0;

		void handleEjectSensor();
		void handleLockSensor();
		void handleManualSwitch();
		void handleCaddySensor();

	protected:
		static core::Schema __schema;

		core::Pack _pack;
		int16_t _nextToolIndex = -1;
		Pocket* _spindlePocket;

		core::Pack& getPack() override {
			return _pack;
		}

		void ensureSpindlePocket();
		bool driverIsEnabled();

	private:
		core::ISR __ejectSensorISR;
		core::ISR __lockSensorISR;
		core::ISR __manualSwitchISR;
		core::ISR __caddySensorISR;

		void manualChange();
		void automaticChange();
		float32_t probeTool();

		void moveForManualChange();

		void promptUserToExchangeTool(uint16_t currentToolIndex, uint16_t nextToolIndex);
		void promptUserToLoadTool(uint16_t toolIndex);
		void promptUserToRemoveTool(uint16_t toolIndex);

		Pocket* findFreePocket(Tool& tool);
		Pocket* findToolPocket(Tool& tool);

		void storeTool(Tool& tool, Pocket& pocket);
		void loadTool(Tool& tool);

	public:
		virtual ~ToolsModule() {
		}

		virtual const char* name() override {
			return "Tools Module";
		}
		virtual void init() override;
		virtual void idle() override;

		bool isAutomatic() {
			return __automaticField.get(_pack);
		}

		void setAutomatic(bool automatic) {
			__automaticField.set(_pack, automatic);
		}

		int16_t getCurrentToolIndex();

		void setNextToolIndex(int16_t value);

		int16_t getNextToolIndex() {
			return _nextToolIndex;
		}

		PocketTable& getPockets() {
			return __pocketsField.get(_pack);
		}

		ToolTable& getTools() {
			return __toolsField.get(_pack);
		}

		Tool* getCurrentTool();

		DriverTable& getDrivers() {
			return __driversField.get(_pack);
		}

		IDriver& getCurrentDriver();

		DriverParameterTable& getDriverParameters() {
			return __driverParametersField.get(_pack);
		}

		void ensureClearOfCaddy();

		void lock();
		void unlock(bool manual = false);
		void change();

		virtual void read(io::InputStream& stream) override;
		virtual void write(io::OutputStream& stream) override;

		static ToolsModule& getInstance(Object* parent = nullptr);
	};
} // namespace swordfish::tools