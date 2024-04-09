/*
 * G43.cpp
 *
 * Created: 6/05/2021 9:41:10 am
 *  Author: smohekey
 */

#include <Eigen/Core>

#include "../gcode.h"
#include "../../module/motion.h"

#include <swordfish/Controller.h>

#if HAS_TOOL_OFFSET

using namespace swordfish;
using namespace swordfish::motion;
using namespace swordfish::tools;
using namespace Eigen;

void GcodeSuite::G43() {
	auto& toolManager = ToolsModule::getInstance();
	auto& motionManager = MotionModule::getInstance();
	auto& tools = toolManager.getTools();

	Vector3f offset { 0, 0, 0 };

	if (parser.seenval('H')) {
		int16_t h = parser.intval('H');

		if (h < 1 || h > MAX_TOOL_OFFSETS) {
			SERIAL_ERROR_MSG("H parameter invalid");

			return;
		}

		auto* tool = tools.get(h - 1);

		if (!tool) {
			SERIAL_ERROR_MSG("H parameter invalid");

			return;
		}

		offset[Axis::Z()] = -tool->getGeometry().length();
	}

	for (auto i : linear_axes) {
		if (parser.seenval(i.to_char())) {
			offset[i] += parser.value_axis_units(i);
		}
	}

	motionManager.setToolOffset(offset);
}

void GcodeSuite::G49() {
	MotionModule::getInstance().setToolOffset({ 0, 0, 0 });
}

#endif
