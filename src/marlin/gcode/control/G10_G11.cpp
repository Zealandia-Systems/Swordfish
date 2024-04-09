/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(CNC_COORDINATE_SYSTEMS)

#	include <swordfish/Controller.h>
#	include <swordfish/core/Console.h>
#	include <swordfish/io/ConsoleOutputStream.h>
#	include <swordfish/io/Writer.h>

using namespace swordfish;
using namespace swordfish::core;
using namespace swordfish::data;
using namespace swordfish::tools;
using namespace swordfish::motion;

#	include "../gcode.h"
#	include "../../module/motion.h"

// #define DEBUG_CNC_G10

template<typename TRecord, typename TTable>
static TRecord& getRecord(Table<TRecord, TTable>& table, uint16_t index, bool& created) {
	created = false;

	for (auto& record : table) {
		if (record.getIndex() == index) {
			return record;
		}
	}

	created = true;

	auto& record = table.emplaceBack();

	record.setIndex(index);

	return record;
}

template<typename TList>
static void handleTableGet(TList& table, uint32_t p) {
	auto& out = Console::out();

	if (p == 0) {
		table.writeJson(out);

	} else if (p >= 1 && p <= table.length()) {
		auto* record = table.get(p - 1);

		if (record) {
			table.writeRecordJson(out, *record);
			out << '\n';
		}
	} else {
		SERIAL_ERROR_MSG("P parameter is invalid.");

		return;
	}
}

void GcodeSuite::G10() {
	auto& out = Console::out();

#	ifdef DEBUG_CNC_G10
	SERIAL_ECHOLNPGM("DEBUG_CNC_G10 enabled.");
#	endif

	const uint16_t l = parser.longval('L', 0);

#	ifdef DEBUG_CNC_G10
	SERIAL_ECHOLN("Received parameters: ");
	SERIAL_ECHO("L: ");
	SERIAL_ECHOLN(l);
	SERIAL_ECHO("P: ");
	SERIAL_ECHOLN(p);
	SERIAL_ECHO("X: ");
	SERIAL_ECHOLN(x);
	SERIAL_ECHO("Y: ");
	SERIAL_ECHOLN(y);
	SERIAL_ECHO("Z: ");
	SERIAL_ECHOLN(z);
#	endif

	auto& toolManager = ToolsModule::getInstance();
	auto& motionManager = MotionModule::getInstance();

	switch (l) {
		case 1: { // Set Tool Wear
			SERIAL_ECHO("ERROR: ");
			SERIAL_ECHO("Set Tool Table");
			SERIAL_ECHOLN(" not yet implemented");

			break;
		}

		case 2: { // Set Coordinate System
			const uint16_t p = parser.longval('P', 0);

			set_work_offsets(p);

			break;
		}

		case 10: { // Set Tool Offset
			const uint16_t p = parser.longval('P', 0);

			auto& tools = toolManager.getTools();

			if (p < 1 || p > tools.length()) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			auto* tool = tools.get(p - 1);

			if (!tool) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			xyz_pos_t offset { 0, 0, 0 };

			for (auto i : linear_axes) {
				if (parser.seenval(i.to_char())) {
					const float v = parser.value_linear_units();

					offset[i] = axis_is_relative(i) ? toLogical(current_position[i], i) + v : v;
				} else {
					offset[i] = toLogical(current_position[i], i);
				}
			}

#	ifdef DEBUG_CNC_G10
			SERIAL_ECHO("tool_offset:");
			SERIAL_ECHOPAIR(" x=", offset[0]);
			SERIAL_ECHOPAIR(", y=", offset[1]);
			SERIAL_ECHOLNPAIR(", z=", offset[2]);
#	endif

			tool->getGeometry().length(offset[2]);

			tools.writeRecordJson(out, *tool);

			break;
		}

		case 11: { // Set Tool Wear
			const uint16_t p = parser.longval('P', 0);

			if (p < 1 || p > MAX_TOOL_OFFSETS) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			break;
		}

		case 20: { // Set position in Coordinate System
			const uint16_t p = parser.longval('P', 0);

			set_work_offsets(p);

			break;
		}

		case 51: { // set wcs
			const uint16_t w = parser.longval('W', 0);

			auto& coordinateSystems = motionManager.getWorkCoordinateSystems();

			if (w < 1 || w > coordinateSystems.length()) {
				SERIAL_ERROR_MSG("W parameter is invalid.");

				return;
			}

			const uint16_t index = w - 1;

			auto* coordinateSystem = coordinateSystems.get(index);

			if (!coordinateSystem) {
				return;
			}

			auto& offset = coordinateSystem->getOffset();

			if (parser.seenval('X')) {
				offset.x(parser.value_linear_units());
			}

			if (parser.seenval('Y')) {
				offset.y(parser.value_linear_units());
			}

			if (parser.seenval('Z')) {
				offset.z(parser.value_linear_units());
			}

			Controller::getInstance().save();

			coordinateSystems.writeRecordJson(out, *coordinateSystem);

			break;
		}

		case 52: { // set tool
			const uint16_t t = parser.longval('T', 0);

			auto& tools = toolManager.getTools();
			// auto& pockets = toolManager.getPockets();

			if (t < 1 || t > tools.length() + 1) {
				SERIAL_ERROR_MSG("T parameter is invalid.");

				return;
			}

			const uint16_t index = t - 1;
			bool created;
			Pocket* pocket = nullptr;
			bool pocketChanged = false;
			// bool driverChanged = false;

			Tool& tool = getRecord<Tool, ToolTable>(tools, index, created);

			if (parser.seenval('I')) {
				auto driverIndex = parser.value_long() - 1;
				auto& drivers = toolManager.getDrivers();

				auto* driver = drivers.get(driverIndex);

				if (driver) {
					tool.setDriverIndex(driverIndex);
				} else {
					SERIAL_ERROR_MSG("Driver doesn't exist.");

					if (created) {
						if (pocketChanged) {
							pocket->setToolIndex(-1);
						}

						tools.remove(tool);
					}

					return;
				}
			}

			if (parser.seenval('F')) {
				tool.setFixed(parser.value_bool());
			}

			auto& geometry = tool.getGeometry();

			if (parser.seenval('W')) {
				geometry.diameter(parser.value_linear_units());
			}

			if (parser.seenval('H')) {
				geometry.length(parser.value_linear_units());
			}

			if (parser.seenval('D')) {
				tool.setDescription(parser.value_string());
			}

			if (parser.seenval('N')) {
				tool.setNeedsProbe(parser.value_bool());
			}

			Controller::getInstance().save();

			tools.writeRecordJson(out, tool);

			break;
		}

		case 53: { // set pocket
			const uint16_t p = parser.longval('P', 0);

			auto& pockets = toolManager.getPockets();

			if (p < 1 || p > pockets.length() + 1) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			const uint16_t index = p - 1;
			bool created;

			Pocket& pocket = getRecord<Pocket, PocketTable>(pockets, index, created);

			if (parser.seenval('E')) {
				pocket.setEnabled(parser.value_bool());
			}

			if (parser.seenval('T')) {
				auto newToolIndex = parser.value_long() - 1;
				auto oldToolIndex = pocket.getToolIndex();

				if (newToolIndex != oldToolIndex) {
					auto& tools = toolManager.getTools();

					if (newToolIndex == -1) {
						pocket.setToolIndex(-1);
					} else {
						bool success = false;

						auto* tool = tools.get(newToolIndex);

						if (!tool) {
							SERIAL_ERROR_MSG("Target tool doesn't exist.");

							return;
						} else {
							for (auto& oldPocket : pockets) {
								if (oldPocket.getToolIndex() == newToolIndex) {
									oldPocket.setToolIndex(-1);
								}
							}

							pocket.setToolIndex(newToolIndex);

							success = true;
						}

						if (!success) {
							if (created) {
								pockets.remove(pocket);
							}

							return;
						}
					}
				}
			}

			if (parser.seenval('D')) {
				pocket.setDepth(parser.value_linear_units());
			}

			auto& offset = pocket.getOffset();

			if (parser.seenval('X')) {
				offset.x(parser.value_linear_units());
			}

			if (parser.seenval('Y')) {
				auto yVal = parser.value_linear_units();

				debug()("y: ", (float32_t) yVal);

				offset.y(yVal);
			}

			if (parser.seenval('Z')) {
				offset.z(parser.value_linear_units());
			}

			Controller::getInstance().save();

			pockets.writeRecordJson(out, pocket);

			break;
		}

		case 54: { // set driver
			const uint16_t d = parser.longval('D', 0);

			auto& drivers = toolManager.getDrivers();

			if (d < 1 || d > drivers.length() + 1) {
				SERIAL_ERROR_MSG("D parameter is invalid.");

				return;
			}

			const uint16_t index = d - 1;
			bool created;

			auto& driver = getRecord<Driver, DriverTable>(drivers, index, created);

			if (parser.seen('T')) {
				auto type = parser.value_long() - 1;

				if (type < 0 || type > 2) {
					SERIAL_ERROR_MSG("T parameter is invalid.");

					if (created) {
						drivers.remove(driver);
					}

					return;
				}

				driver.setType(type);
			}

			Controller::getInstance().save();

			break;
		}

		case 55: { // set driver parameter
			const uint16_t p = parser.longval('P', 0);

			auto& driverParameters = toolManager.getDriverParameters();
			auto& drivers = toolManager.getDrivers();

			if (p < 1 || p > driverParameters.length() + 1) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			const uint16_t index = p - 1;
			bool created;

			auto id = parser.longval('I', -1);
			auto driverIndex = parser.longval('D', 0) - 1;
			auto value = parser.stringval('V', nullptr);

			if (id == -1 || driverIndex == -1 || value == nullptr) {
				SERIAL_ERROR_MSG("I (id), D (driver), and V (value) must be specified.");

				return;
			}

			auto* driver = drivers.get(driverIndex);

			if (!driver) {
				SERIAL_ERROR_MSG("Driver doesn't exist.");

				return;
			}

			auto& driverParameter = getRecord<DriverParameter, DriverParameterTable>(driverParameters, index, created);

			driverParameter.setId(id);
			driverParameter.setDriverIndex(driverIndex);
			driverParameter.setValue(value);

			Controller::getInstance().save();

			break;
		}

		case 62: { // delete tool
			const uint16_t t = parser.longval('T', 0);

			auto& tools = toolManager.getTools();

			if (t < 1 || t > tools.length()) {
				SERIAL_ERROR_MSG("T parameter is invalid.");

				return;
			}

			const uint32_t index = t - 1;

			auto* tool = tools.get(index);

			if (tool) {
				tools.remove(*tool);

				Controller::getInstance().save();
			}

			break;
		}

		case 63: { // delete pocket
			const uint16_t p = parser.longval('P', 0);

			auto& pockets = toolManager.getPockets();

			if (p < 1 || p > pockets.length()) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			const uint32_t index = p - 1;

			auto* pocket = pockets.get(index);

			if (pocket) {
				pockets.remove(*pocket);

				Controller::getInstance().save();
			}

			break;
		}

		case 64: { // delete driver
			const uint16_t d = parser.longval('P', 0);

			auto& drivers = toolManager.getDrivers();

			if (d < 1 || d > drivers.length()) {
				SERIAL_ERROR_MSG("D parameter is invalid.");

				return;
			}

			const uint16_t index = d - 1;

			auto* driver = drivers.get(index);

			if (driver) {
				drivers.remove(*driver);

				Controller::getInstance().save();
			}

			break;
		}

		case 65: { // delete driver parameter
			const uint16_t p = parser.longval('P', 0);

			auto& driverParameters = toolManager.getDriverParameters();

			if (p < 1 || p > driverParameters.length()) {
				SERIAL_ERROR_MSG("P parameter is invalid.");

				return;
			}

			const uint16_t index = p - 1;

			auto* driverParameter = driverParameters.get(index);

			if (driverParameter) {
				driverParameters.remove(*driverParameter);

				Controller::getInstance().save();
			}

			break;
		}

		default: {
			SERIAL_ERROR_MSG("Invalid L parameter.");

			break;
		}
	}
}

void GcodeSuite::G11() {
	const uint32_t l = parser.longval('L', 0),
								 p = parser.longval('P', 0);

	switch (l) {
		case 101: { // get wcs
			auto& table = MotionModule::getInstance().getWorkCoordinateSystems();

			handleTableGet(table, p);

			break;
		}

		case 102: { // get tools
			auto& table = ToolsModule::getInstance().getTools();

			handleTableGet(table, p);

			break;
		}

		case 103: { // get pockets
			auto& table = ToolsModule::getInstance().getPockets();

			handleTableGet(table, p);

			break;
		}

		case 104: { // get drivers
			auto& table = ToolsModule::getInstance().getDrivers();

			handleTableGet(table, p);

			break;
		}

		case 105: { // get driver parameters
			auto& table = ToolsModule::getInstance().getDriverParameters();

			handleTableGet(table, p);

			break;
		}

		default: {
			SERIAL_ERROR_MSG("Invalid L parameter.");
		}
	}
}

void GcodeSuite::set_work_offsets(const uint8_t p) {
	auto& motionModule = MotionModule::getInstance();
	auto& coordinateSystems = motionModule.getWorkCoordinateSystems();
	auto& oldSystem = motionModule.getActiveCoordinateSystem();

	uint16_t index = p - 1;

	const boolean systemChange = oldSystem.getIndex() != index;

	if (p < 1 || p > coordinateSystems.length()) {
		SERIAL_ERROR_MSG("P parameter is invalid.");

		return;
	}

	if (systemChange) {
#	ifdef DEBUG_CNC_G10
		SERIAL_ECHO("Active system was ");
		SERIAL_ECHOLN(oldSystem.index());
#	endif

		auto* newSystem = coordinateSystems.get(index);

		if (!newSystem) {
			SERIAL_ERROR_MSG("Invalid work coordinate system.");

			return;
		}

		motionModule.setActiveCoordinateSystem(*newSystem);
	}

	auto& activeSystem = motionModule.getActiveCoordinateSystem();

	auto homeOffset = motionModule.getHomeOffset();
	auto toolOffset = motionModule.getToolOffset();
	auto& offset = activeSystem.getOffset();


	if (parser.seen('X')) {
		offset.x(parser.value_axis_units(Axis::X()) - homeOffset.x() - toolOffset.x() - current_position.x());
	}

	if (parser.seen('Y')) {
		offset.y(parser.value_axis_units(Axis::Y()) - homeOffset.y() - toolOffset.y() - current_position.y());
	}

	if (parser.seen('Z')) {
		offset.z(parser.value_axis_units(Axis::Z()) - homeOffset.z() - toolOffset.z() - current_position.z());
	}

	Controller::getInstance().save();

	motionModule.setActiveCoordinateSystem(activeSystem);

	if (systemChange) {
#	ifdef DEBUG_CNC_G10
		SERIAL_ECHO("Setting system to ");
		SERIAL_ECHOLN(oldSystem.index());
#	endif

		motionModule.setActiveCoordinateSystem(oldSystem);

#	ifdef DEBUG_CNC_G10
		SERIAL_ECHO("Active system is ");
		SERIAL_ECHOLN(oldSystem.index());
		SERIAL_ECHOLN("Current position in current coordinate system is: ");
#	endif

		report_current_position(); // REPORT CURRENT POSITION
	}

	handleTableGet(coordinateSystems, p);
}
#endif // CNC_COORDINATE_SYSTEMS
