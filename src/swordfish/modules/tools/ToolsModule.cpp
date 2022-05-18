/*
 * Module.cpp
 *
 * Created: 29/07/2021 12:36:15 pm
 *  Author: smohekey
 */

#include <charconv>

#include <Eigen/Core>

#include <marlin/gcode/gcode.h>
#include <marlin/feature/host_actions.h>
#include <marlin/feature/spindle_laser.h>
#include <marlin/MarlinCore.h>
#include <marlin/module/motion.h>
#include <marlin/module/endstops.h>
#include <marlin/module/planner.h>

#include <swordfish/debug.h>
#include <swordfish/Controller.h>
#include <swordfish/core/Console.h>
#include <swordfish/core/InvalidOperationException.h>
#include <swordfish/io/StringOutputStream.h>
#include <swordfish/io/Writer.h>
#include <swordfish/modules/estop/EStopException.h>
#include <swordfish/modules/motion/MotionModule.h>

#include "ToolsModule.h"

extern bool run_probe(AxisEnum axis, EndstopEnum endstop, float distance, float retract);

namespace swordfish::tools {
	using namespace Eigen;

	using namespace swordfish::core;
	using namespace swordfish::io;
	using namespace swordfish::estop;
	using namespace swordfish::motion;
	using namespace swordfish::utils;

	ToolsModule* ToolsModule::__instance = nullptr;

	NullDriver ToolsModule::__nullDriver = {};

	core::ValueField<bool> ToolsModule::__automaticField = { "automatic", 0, false };
	core::ObjectField<PocketTable> ToolsModule::__pocketsField = { "pockets", 0 };
	core::ObjectField<ToolTable> ToolsModule::__toolsField = { "tools", 1 };
	core::ObjectField<DriverTable> ToolsModule::__driversField = { "drivers", 2 };
	core::ObjectField<DriverParameterTable> ToolsModule::__driverParametersField = { "driverParameters", 3 };

	core::Schema ToolsModule::__schema = {
		utils::typeName<ToolsModule>(),
		&(Module::__schema),
		{__automaticField       },
		{ __pocketsField, __toolsField,
                        __driversField,
                        __driverParametersField }
	};

	void ToolsModule::handleEjectSensor() {
		debug()("eject: ", READ(ATC_EJECT_SENSOR_PIN) != ATC_EJECT_SENSOR_PIN_INVERTED);

		_flags[EjectSensorFlag] = READ(ATC_EJECT_SENSOR_PIN) != ATC_EJECT_SENSOR_PIN_INVERTED;
	}

	void ToolsModule::handleLockSensor() {
		debug()("lock: ", READ(ATC_LOCK_SENSOR_PIN) != ATC_LOCK_SENSOR_PIN_INVERTED);

		_flags[LockSensorFlag] = READ(ATC_LOCK_SENSOR_PIN) != ATC_LOCK_SENSOR_PIN_INVERTED;
	}

	void ToolsModule::handleManualSwitch() {
		debug()("manual: ", READ(ATC_MANUAL_SWITCH_PIN) != ATC_MANUAL_SWITCH_PIN_INVERTED);

		_flags[ManualSwitchFlag] = READ(ATC_MANUAL_SWITCH_PIN) != ATC_MANUAL_SWITCH_PIN_INVERTED;
	}

	void ToolsModule::handleCaddySensor() {
		debug()("dock: ", READ(ATC_DOCK_SENSOR_PIN) != ATC_DOCK_SENSOR_PIN_INVERTED);

		_flags[CaddySensorFlag] = READ(ATC_DOCK_SENSOR_PIN) != ATC_DOCK_SENSOR_PIN_INVERTED;

		if (_flags[HomingFlag] && !_flags[CaddySensorFlag]) {
			planner.quick_stop();
		}
	}

	ToolsModule::ToolsModule(core::Object* parent) :
			Module(parent),
			_pack(__schema, *this, &(Module::_pack)),
			_spindlePocket(nullptr),
			__ejectSensorISR(std::bind(&ToolsModule::handleEjectSensor, this), ATC_EJECT_SENSOR_PIN, CHANGE),
			__lockSensorISR(std::bind(&ToolsModule::handleLockSensor, this), ATC_LOCK_SENSOR_PIN, CHANGE),
			__manualSwitchISR(std::bind(&ToolsModule::handleManualSwitch, this), ATC_MANUAL_SWITCH_PIN, CHANGE),
			__caddySensorISR(std::bind(&ToolsModule::handleCaddySensor, this), ATC_DOCK_SENSOR_PIN, CHANGE) {
	}

	void ToolsModule::ensureSpindlePocket() {
		auto& pockets = getPockets();
		auto toolIndex = -1;
start:

		for (auto& pocket : pockets) {
			if (pocket.getIndex() == -2) {
				toolIndex = pocket.getToolIndex();

				pockets.remove(pocket);

				goto start;
			}
		}

		auto& pocket = pockets.emplaceBack();

		pocket.setIndex(-2);
		pocket.setToolIndex(toolIndex);
		pocket.setReadOnly(true);
		pocket.setDepth(0);

		_spindlePocket = &pocket;
	}

	void ToolsModule::read(io::InputStream& stream) {
		Module::read(stream);

		ensureSpindlePocket();
	}

	void ToolsModule::write(io::OutputStream& stream) {
		ensureSpindlePocket();

		Module::write(stream);
	}

	void ToolsModule::init() {
		SET_OUTPUT(ATC_LOCK_PIN);
		SET_OUTPUT(ATC_SEAL_PIN);

		SET_INPUT_PULLDOWN(ATC_EJECT_SENSOR_PIN);
		SET_INPUT_PULLDOWN(ATC_LOCK_SENSOR_PIN);
		SET_INPUT_PULLDOWN(ATC_MANUAL_SWITCH_PIN);
		SET_INPUT_PULLDOWN(ATC_DOCK_SENSOR_PIN);

		WRITE(ATC_SEAL_PIN, !ATC_SEAL_PIN_INVERTED);

		__ejectSensorISR.attach();
		__lockSensorISR.attach();
		__caddySensorISR.attach();
		__manualSwitchISR.attach();

		_flags[EjectSensorFlag] = READ(ATC_EJECT_SENSOR_PIN) != ATC_EJECT_SENSOR_PIN_INVERTED;
		_flags[LockSensorFlag] = READ(ATC_LOCK_SENSOR_PIN) != ATC_LOCK_SENSOR_PIN_INVERTED;
		_flags[ManualSwitchFlag] = READ(ATC_MANUAL_SWITCH_PIN) != ATC_MANUAL_SWITCH_PIN_INVERTED;
		_flags[CaddySensorFlag] = READ(ATC_DOCK_SENSOR_PIN) != ATC_DOCK_SENSOR_PIN_INVERTED;
	}

	void ToolsModule::idle() {
		if (!_flags[UnlockedFlag] && _flags[ManualSwitchFlag] && !_flags[EjectSensorFlag]) {
			debug()("manual unlock");
			unlock(true);
		}

		if (!_flags[UnlockedFlag] && _flags[EjectSensorFlag] && !_flags[ManualSwitchFlag]) {
			debug()("manual lock");
			lock();
		}

		getCurrentDriver().idle();
	}

	void ToolsModule::unlock(bool manual) {
		debug()();

		if (driverIsEnabled() || _flags[UnlockedFlag]) {
			return;
		}

		WRITE(ATC_LOCK_PIN, !ATC_LOCK_PIN_INVERTED);

		while (READ(ATC_EJECT_SENSOR_PIN) == ATC_EJECT_SENSOR_PIN_INVERTED) {
			safe_delay(0);
		}

		_flags[UnlockedFlag] = !manual;
	}

	void ToolsModule::lock() {
		debug()();

		if (driverIsEnabled()) {
			return;
		}

		// WRITE(ATC_SEAL_PIN, ATC_SEAL_PIN_INVERTED);
		WRITE(ATC_LOCK_PIN, ATC_LOCK_PIN_INVERTED);

		uint8_t tries = 20;

		while (READ(ATC_LOCK_SENSOR_PIN) == ATC_LOCK_SENSOR_PIN_INVERTED && tries) {
			safe_delay(100);

			tries--;
		}

		// WRITE(ATC_SEAL_PIN, !ATC_SEAL_PIN_INVERTED);

		_flags[UnlockedFlag] = false;
	}

	bool ToolsModule::driverIsEnabled() {
		return getCurrentDriver().isEnabled();
	}

	void ToolsModule::setNextToolIndex(int16_t value) {
		if (!isAutomatic()) {
			_nextToolIndex = value;

			return;
		}

		if (value == -1) {
			_nextToolIndex = -1;

			return;
		}

		for (auto& tool : getTools()) {
			if (tool.getIndex() == value) {
				_nextToolIndex = value;

				return;
			}
		}

		throw InvalidOperationException("Invalid tool number.");

		return;
	}

	void ToolsModule::change() {
		if (driverIsEnabled()) {
			throw InvalidOperationException("Can't change tools while a tool is in use.");
		}

		if (!isAutomatic()) {
			manualChange();
		} else {
			automaticChange();
		}
	}

	Pocket* ToolsModule::findFreePocket(Tool& tool) {
		auto& pockets = getPockets();
		auto& offset = tool.getOffset();

		(void) offset;

		for (auto& pocket : pockets) {
			if (pocket.isEnabled() && (pocket.getToolIndex() == -1 /*&& pocket.getDepth() > abs(offset.z())*/)) {
				return &pocket;

				break;
			}
		}

		return nullptr;
	}

	Pocket* ToolsModule::findToolPocket(Tool& tool) {
		auto& pockets = getPockets();

		for (auto& pocket : pockets) {
			if (pocket.getToolIndex() == tool.getIndex()) {
				return &pocket;
			}
		}

		return nullptr;
	}

	void ToolsModule::promptUserToRemoveTool(uint16_t toolIndex) {
		char buffer[200];

		memset(buffer, 0, sizeof(buffer));

		StringOutputStream stream { buffer, sizeof(buffer) };
		Writer writer { stream };

		writer << "Please remove tool " << toolIndex + 1 << " from the spindle.";

		host_prompt(PromptReason::PROMPT_USER_CONTINUE, buffer, "Resume");

		wait_for_user_response(0);
	}

	void ToolsModule::promptUserToLoadTool(uint16_t toolIndex) {
		char buffer[200];

		memset(buffer, 0, sizeof(buffer));

		StringOutputStream stream { buffer, sizeof(buffer) };
		Writer writer { stream };

		writer << "Please load tool " << toolIndex + 1 << " into the spindle.";

		host_prompt(PromptReason::PROMPT_USER_CONTINUE, buffer, "Resume");

		wait_for_user_response(0);
	}

	void ToolsModule::promptUserToExchangeTool(uint16_t currentToolIndex, uint16_t nextToolIndex) {
		char buffer[200];

		memset(buffer, 0, sizeof(buffer));

		StringOutputStream stream { buffer, sizeof(buffer) };
		Writer writer { stream };

		writer << "Please remove tool " << currentToolIndex + 1 << " from the spindle, and replace with tool " << nextToolIndex + 1 << ".";

		host_prompt(PromptReason::PROMPT_USER_CONTINUE, buffer, "Resume");

		wait_for_user_response(0);
	}

	void ToolsModule::moveForManualChange() {
		auto& motionModule = MotionModule::getInstance();
		auto& mcs = motionModule.getMachineCoordiateSystem();
		auto& tcs = motionModule.getToolChangeCoordinateSystem();
		auto& tccs = motionModule.getToolChangeCoordinateSystem();

		motionModule.setActiveCoordinateSystem(mcs);
		motionModule.rapidMove({ .z = 0 });

		auto nativeClearanceX = motionModule.toNative(X_AXIS, CaddyClearanceX);

		if (current_position.x < nativeClearanceX) {
			motionModule.setActiveCoordinateSystem(tcs);
			motionModule.rapidMove({ .x = CaddyClearanceX });
		}

		motionModule.setActiveCoordinateSystem(tccs);
		motionModule.rapidMove({ .x = 0, .y = 0 });

		motionModule.synchronize();
	}

	void ToolsModule::manualChange() {
		debug()("manual tool change");

		auto& tools = getTools();
		auto& motionModule = MotionModule::getInstance();
		auto& oldWCS = motionModule.getActiveCoordinateSystem();
		auto currentToolIndex = getCurrentToolIndex();
		auto nextToolIndex = getNextToolIndex();

		auto* tool = tools.get(nextToolIndex);

		if (tool && tool->isFixed()) {
			_spindlePocket->setToolIndex(nextToolIndex);

			// tools.writeRecordJson(out, *tool);

			Controller::getInstance().save();

			return;
		}

		moveForManualChange();

		if (currentToolIndex >= 0 && nextToolIndex >= 0) {
			promptUserToExchangeTool(currentToolIndex, nextToolIndex);
		} else if (nextToolIndex >= 0) {
			promptUserToLoadTool(nextToolIndex);
		} else {
			host_prompt(PromptReason::PROMPT_USER_CONTINUE, "Please change tool.", "Resume");

			wait_for_user_response(0);
		}

		auto offsetZ = probeTool();

		// Activate the tool offset
		motionModule.setToolOffset({ .x = 0, .y = 0, .z = offsetZ });

		motionModule.setActiveCoordinateSystem(oldWCS);

		_spindlePocket->setToolIndex(nextToolIndex);

		Controller::getInstance().save();
	}

	void ToolsModule::automaticChange() {
		auto& motionModule = MotionModule::getInstance();

		auto& oldWCS = motionModule.getActiveCoordinateSystem();

		auto currentToolIndex = getCurrentToolIndex();
		auto nextToolIndex = getNextToolIndex();

		if (currentToolIndex == nextToolIndex) {
			throw InvalidOperationException("Requested tool is already selected.");

			debug()("nothing to change");

			return;
		}

		try {
			motionModule.setLimitsEnabled(false);
			motionModule.setToolOffset({ 0, 0, 0 });

			debug()("automatic tool change");

			auto& tools = getTools();
			auto* currentTool = currentToolIndex >= 0 ? tools.get(currentToolIndex) : nullptr;
			auto* nextTool = nextToolIndex >= 0 ? tools.get(nextToolIndex) : nullptr;

			auto* freePocket = currentTool ? findFreePocket(*currentTool) : nullptr;

			auto* nextPocket = nextTool ? findToolPocket(*nextTool) : nullptr;

			if (currentToolIndex >= 0 && !currentTool) {
				SERIAL_ERROR_MSG("Current tool is invalid.");

				return;
			}

			if (nextToolIndex >= 0 && !nextTool) {
				SERIAL_ERROR_MSG("Next tool is invalid.");

				return;
			}

			if (
					currentTool &&
					!currentTool->isFixed() &&
					!freePocket &&
					nextTool &&
					!nextTool->isFixed() &&
					nextPocket) {
				// no free pocket for current and next tool is not in a pocket
				// ask user to swap

				moveForManualChange();

				promptUserToExchangeTool(currentToolIndex, nextToolIndex);
			} else {
				if (currentTool && !currentTool->isFixed()) {
					if (freePocket) {
						storeTool(*currentTool, *freePocket);
					} else {
						moveForManualChange();

						promptUserToRemoveTool(currentToolIndex);
					}
				}

				if (nextTool && !nextTool->isFixed()) {
					loadTool(*nextTool);
				}
			}
		} catch (EStopException& e) {
		}

		motionModule.setActiveCoordinateSystem(oldWCS);
		motionModule.setLimitsEnabled(true);

		Controller::getInstance().save();
	}

	float32_t ToolsModule::probeTool() {
		auto& motionModule = MotionModule::getInstance();

		auto& mcs = motionModule.getMachineCoordiateSystem();
		auto& tcs = motionModule.getToolCoordinateSystem();
		auto homeOffset = motionModule.getHomeOffset();
		auto workOffset = motionModule.getWorkOffset();

		// Clear any active tool offset
		motionModule.setToolOffset({ 0, 0, 0 });

		motionModule.setActiveCoordinateSystem(mcs);
		motionModule.rapidMove({ .z = 0 });

		motionModule.setActiveCoordinateSystem(tcs);

		if (isAutomatic()) {
			motionModule.rapidMove({ .x = CaddyClearanceX });
			motionModule.rapidMove({ .y = 0 });
			motionModule.rapidMove({ .x = 0 });
		} else {
			motionModule.rapidMove({ .x = 0, .y = 0 });
		}

		// Run the tool probe
		remember_feedrate_scaling_off();

		feedrate_mm_s = homing_feedrate(Z_AXIS) * 0.3;

		const auto delta = -abs(home_dir(Z_AXIS) > 0 ? Z_MAX_POS - Z_MIN_POS : Z_MIN_POS - Z_MAX_POS);

		endstops.enable_tool_probe(true);

		run_probe(Z_AXIS, TOOL_PROBE, delta /*, 5*/);

		endstops.enable_tool_probe(false);

		restore_feedrate_and_scaling();

		debug()("z: ", current_position[Z_AXIS]);

		// auto length = motionModule.toNative(Z_AXIS, current_position[Z_AXIS]);
		auto offsetZ = -homeOffset[Z_AXIS] - workOffset[Z_AXIS] - current_position[Z_AXIS];
		// auto offsetZ = motionModule.toLogical(Z_AXIS, current_position[Z_AXIS]);

		debug()("tool offsetZ: ", offsetZ);

		// Raise the Z axis
		motionModule.setActiveCoordinateSystem(mcs);
		motionModule.rapidMove({ .z = 0 });

		if (isAutomatic()) {
			auto& limits = motionModule.getLimits();
			auto& minObj = limits.getMin();

			motionModule.move({ .x = minObj.x(), .feedRate = homing_feedrate(X_AXIS) });
		}

		return offsetZ;
	}

	int16_t ToolsModule::getCurrentToolIndex() {
		return _spindlePocket ? _spindlePocket->getToolIndex() : -1;
	}

	Tool* ToolsModule::getCurrentTool() {
		if (!_spindlePocket) {
			return nullptr;
		}

		auto& tools = getTools();

		return tools.get(_spindlePocket->getToolIndex());
	}

	IDriver& ToolsModule::getCurrentDriver() {
		auto* tool = getCurrentTool();

		uint16_t driverIndex = tool ? tool->getDriverIndex() : 0;

		auto& drivers = getDrivers();
		auto* driver = drivers.get(driverIndex);

		if (driver) {
			auto* impl = driver->getImplementation();

			if (impl) {
				return *impl;
			} else {
				return __nullDriver;
			}
		} else {
			return __nullDriver;
		}
	}

	void ToolsModule::ensureClearOfCaddy() {
		if (isAutomatic() && _flags[CaddySensorFlag]) {
			auto& motionModule = MotionModule::getInstance();

			_flags[HomingFlag] = true;

			motionModule.move({ .x = 100, .feedRate = homing_feedrate(X_AXIS), .relativeAxes = AxisSelector::All });

			planner.synchronize();

			_flags[HomingFlag] = !false;
		}
	}

	void ToolsModule::storeTool(Tool& tool, Pocket& pocket) {
		auto& motionModule = MotionModule::getInstance();
		auto& mcs = motionModule.getMachineCoordiateSystem();
		auto& tcs = motionModule.getToolCoordinateSystem();
		auto& wcs = motionModule.getActiveCoordinateSystem();

		debug()("Storing tool: ", tool.getIndex());

		debug()("storing in pocket: ", pocket.getIndex());

		Vector3f target = pocket.getOffset();

		// move z to top
		motionModule.setActiveCoordinateSystem(mcs);
		motionModule.rapidMove({ .z = 0 });

		motionModule.setActiveCoordinateSystem(tcs);

		// first we have to move to a position inline with the Y position of the pocket,
		// and clear of the caddy on the X axis.
		// If the current X position is already clear of the caddy, we do it in one move.
		// Otherwise we move to the clearance position in the X axis first, and then move to
		// the target position in the Y axis.

		auto nativeClearanceX = motionModule.toNative(X_AXIS, CaddyClearanceX);

		if (current_position.x >= nativeClearanceX) {
			motionModule.rapidMove({ .x = CaddyClearanceX, .y = target(Y) });
		} else {
			motionModule.rapidMove({ .x = CaddyClearanceX });

			motionModule.rapidMove({ .y = target(Y) });
		}

		// move to the tool clip clearance position on the X axis, this is slightly closer to the pocket than
		// the caddy clearance position.
		motionModule.move({ .x = ToolClipClearanceX, .feedRate = homing_feedrate(X_AXIS) });

		// move z to pocket offset.z
		motionModule.move({ .z = target(Z), .feedRate = homing_feedrate(Z_AXIS) });

		// move to x position of pocket offset
		motionModule.move({ .x = target(X), .feedRate = homing_feedrate(X_AXIS) });

		motionModule.synchronize();

		unlock();

		motionModule.setActiveCoordinateSystem(mcs);
		motionModule.rapidMove({ .z = 0 });

		motionModule.synchronize();

		lock();

		pocket.setToolIndex(tool.getIndex());

		_spindlePocket->setToolIndex(-1);

		motionModule.setActiveCoordinateSystem(wcs);
	}

	void ToolsModule::loadTool(Tool& tool) {
		auto& motionModule = MotionModule::getInstance();
		auto& mcs = motionModule.getMachineCoordiateSystem();
		auto& tcs = motionModule.getToolCoordinateSystem();
		auto& wcs = motionModule.getActiveCoordinateSystem();

		debug()("loading tool: ", tool.getIndex());

		auto doProbe = tool.getNeedsProbe();

		Pocket* sourcePocket = findToolPocket(tool);

		debug()("sourcePocket: ", sourcePocket);

		if (sourcePocket == nullptr) {
			debug()("manual tool change");

			lock();

			moveForManualChange();

			promptUserToLoadTool(tool.getIndex());
		} else {
			// move z to top
			motionModule.setActiveCoordinateSystem(mcs);
			motionModule.move({ .z = 0 });

			motionModule.setActiveCoordinateSystem(tcs);

			Vector3f target = sourcePocket->getOffset();

			debug()("target -> x: ", target(X), ", y: ", target(Y));

			// move to x y of pocket offset
			motionModule.rapidMove({ .x = target(X), .y = target(Y) });

			motionModule.synchronize();

			unlock();

			// move z to pocket offset.z
			motionModule.move({ .z = target(Z), .feedRate = homing_feedrate(Z_AXIS) });

			motionModule.synchronize();

			lock();

			motionModule.move({ .x = ToolClipClearanceX, .feedRate = homing_feedrate(X_AXIS) });

			motionModule.setActiveCoordinateSystem(mcs);
			motionModule.move({ .z = 0, .feedRate = homing_feedrate(Z_AXIS) });

			motionModule.setActiveCoordinateSystem(tcs);
			motionModule.rapidMove({ .x = CaddyClearanceX });

			sourcePocket->setToolIndex(-1);
		}

		auto& offset = tool.getOffset();

		if (doProbe) {
			auto offsetZ = probeTool();

			// Store the tool offset
			offset.z(offsetZ);

			tool.setNeedsProbe(false);
		}

		// Activate the tool offset
		motionModule.setToolOffset(offset);

		_spindlePocket->setToolIndex(tool.getIndex());

		motionModule.setActiveCoordinateSystem(wcs);
	}

	swordfish::tools::ToolsModule& ToolsModule::getInstance(Object* parent /*= nullptr*/) {
		return *(__instance ?: __instance = new ToolsModule(parent));
	}
} // namespace swordfish::tools
