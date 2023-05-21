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

#include "../../inc/MarlinConfigPre.h"

#if HAS_SOFTWARE_ENDSTOPS

#	include <Eigen/Core>

#	include "../gcode.h"
#	include "../../module/motion.h"

#	include <swordfish/core/Vector3.h>
#	include <swordfish/modules/motion/MotionModule.h>

using namespace Eigen;
using namespace swordfish;
using namespace swordfish::motion;

static void setAxis(core::LinearVector3& vector, uint8_t axis, float32_t value) {
	switch (axis) {
		case 0: {
			vector.x(value);

			break;
		}

		case 1: {
			vector.y(value);

			break;
		}

		case 2: {
			vector.z(value);

			break;
		}
	}
}

static void logLimits() {
	auto& motionModule = MotionModule::getInstance();
	auto& limits = motionModule.getLimits();

	Vector3f min = limits.getMin();
	Vector3f max = limits.getMax();

	// motionModule.toLogical(min);
	// motionModule.toLogical(max);

	SERIAL_ECHO_START();
	SERIAL_ECHOPGM(STR_SOFT_ENDSTOPS);

	serialprint_onoff(limits.areEnabled());
	print_xyz(min, PSTR(STR_SOFT_MIN), PSTR(" "));
	print_xyz(max, PSTR(STR_SOFT_MAX));
}
/**
 * M211: Enable, Disable, and/or Report software endstops
 *
 * Usage: M211 S1 to enable, M211 S0 to disable, M211 alone for report
 */
void GcodeSuite::M211() {
	auto& motionModule = MotionModule::getInstance();
	auto& limits = motionModule.getLimits();

	if (parser.seen('S')) {
		limits.setEnabled(parser.value_bool());
	}

	logLimits();
}

void GcodeSuite::M212() {
	auto& motionManager = MotionModule::getInstance();
	auto& limits = motionManager.getLimits();
	auto& minObj = limits.getMin();
	auto writeConfig = false;

	for (auto i = 0; i < 3; i++) {
		if (parser.seen(XYZ_CHAR(i))) {
			writeConfig = true;

			setAxis(minObj, i, parser.linear_value_to_mm(parser.value_linear_units()));
		}
	}

	if (writeConfig) {
		Controller::getInstance().save();
	}

	logLimits();
}

void GcodeSuite::M213() {
	auto& motionManager = MotionModule::getInstance();
	auto& limits = motionManager.getLimits();
	auto& maxObj = limits.getMax();
	auto writeConfig = false;

	for (auto i = 0; i < 3; i++) {
		if (parser.seen(XYZ_CHAR(i))) {
			writeConfig = true;

			setAxis(maxObj, i, parser.linear_value_to_mm(parser.value_linear_units()));
		}
	}

	if (writeConfig) {
		Controller::getInstance().save();
	}

	logLimits();
}

#endif
