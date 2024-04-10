/*
 * MotionManager.cpp
 *
 * Created: 27/08/2021 4:47:51 pm
 *  Author: smohekey
 */

#include <cmath>

#include "MotionModule.h"

#include <marlin/module/endstops.h>
#include <marlin/module/motion.h>
#include <marlin/module/planner.h>

extern float32_t rapidrate_mm_s;

#include <swordfish/debug.h>
#include <swordfish/math.h>
#include <swordfish/modules/estop/EStopModule.h>

namespace swordfish::motion {
	using namespace Eigen;

	using namespace swordfish::core;
	using namespace swordfish::estop;
	using namespace swordfish::math;
	using namespace swordfish::utils;

	MotionModule* MotionModule::__instance = nullptr;

	core::ValueField<int16_t> MotionModule::__activeCoordinateSystemField = { "wcs", 0, 0 };
	core::ObjectField<CoordinateSystemTable> MotionModule::__workCoordinateSystemsField = { "workCoordinateSystems", 0 };
	core::ObjectField<Limits> MotionModule::__limitsField = { "limits", 1 };
	core::ObjectField<core::LinearVector3> MotionModule::__homeOffsetField = { "homeOffset", 2 };
	core::ObjectField<core::LinearVector3> MotionModule::__workOffsetField = { "workOffset", 3 };
	core::ObjectField<core::LinearVector3> MotionModule::__toolOffsetField = { "toolOffset", 4 };

	core::Schema MotionModule::__schema = {
		utils::typeName<MotionModule>(),
		&(Module::__schema),
		{ __activeCoordinateSystemField },
		{ __workCoordinateSystemsField,
		                    __limitsField,
		                    __homeOffsetField,
		                    __workOffsetField,
		                    __toolOffsetField }
	};

	MotionModule::MotionModule(Object* parent) :
			Module(parent),
			_pack(__schema, *this, &(Module::_pack)),
			_workCoordinateSystems(__workCoordinateSystemsField.get(_pack)),
			_limits(__limitsField.get(_pack)),
			_machineCoordinateSystem(this),
			_activeCoordinateSystem(nullptr),
			_toolChangeCoordinateSystem(nullptr),
			_toolCoordinateSystem(nullptr) {

		_machineCoordinateSystem.setIndex(-1);
	}

	Pack& MotionModule::getPack() {
		return _pack;
	}

	void MotionModule::read(io::InputStream& stream) {
		Module::read(stream);

		auto activeCoordinateSystem = __activeCoordinateSystemField.get(_pack);

		auto* coordinateSystem = getWorkCoordinateSystems().get(activeCoordinateSystem);

		if (coordinateSystem) {
			setActiveCoordinateSystem(*coordinateSystem);
		}

		updateOffset();
	}

	void MotionModule::init() {
		auto& coordinateSystems = getWorkCoordinateSystems();
		auto activeCoordinateSystem = __activeCoordinateSystemField.get(_pack);

		for (auto i = 0; i < 60; i++) {
			auto& coordinateSystem = coordinateSystems.emplaceBack();

			coordinateSystem.setIndex(i);

			if (i == activeCoordinateSystem) {
				setActiveCoordinateSystem(coordinateSystem);
			} else if (i == 58) {
				_toolChangeCoordinateSystem = &coordinateSystem;
			} else if (i == 59) {
				_toolCoordinateSystem = &coordinateSystem;
			}
		}

		updateOffset();
	}

	static inline const char* absoluteOrRelative(AxisSelector axis, Flags<AxisSelector>& relativeAxis) {
		return relativeAxis & axis ? "relative" : "absolute";
	}

	void MotionModule::move(Movement movement) {
		Vector6f32 currentNative = current_position;
		Vector6f32 currentLogical = toLogical(currentNative); //_logicalTransform * currentNative;
		Vector6f32 targetLogical {
			isnan(movement.x) ? currentLogical.x() : (movement.relative_axes & AxisSelector::X ? currentLogical.x() + movement.x : movement.x),
			isnan(movement.y) ? currentLogical.y() : (movement.relative_axes & AxisSelector::Y ? currentLogical.y() + movement.y : movement.y),
			isnan(movement.z) ? currentLogical.z() : (movement.relative_axes & AxisSelector::Z ? currentLogical.z() + movement.z : movement.z),
			0,
			0,
			0
		};

		Vector6f32 targetNative = toNative(targetLogical); //_nativeTransform * targetLogical;

		debug()("===============================================");
		debug()(
				"requested -> x(", absoluteOrRelative(AxisSelector::X, movement.relative_axes), "): ", movement.x,
				", y(", absoluteOrRelative(AxisSelector::Y, movement.relative_axes), "): ", movement.y,
				", z(", absoluteOrRelative(AxisSelector::Z, movement.relative_axes), "): ", movement.z,
				", feed_rate: ", movement.feed_rate.has_value() ? movement.feed_rate.value().value() : feedrate_mm_s.value());
		debug()("current native -> x: ", currentNative.x(), ", y: ", currentNative.y(), ", z: ", currentNative.z());
		debug()("current logical -> x: ", currentLogical.x(), ", y: ", currentLogical.y(), ", z: ", currentLogical.z());
		debug()("target logical -> x: ", targetLogical.x(), ", y: ", targetLogical.y(), ", z: ", targetLogical.z());
		debug()("target native -> x: ", targetNative.x(), ", y: ", targetNative.y(), ", z: ", targetNative.z());

		_limits.throwIfOutside({ targetNative.x(), targetNative.y(), targetNative.z() });

		auto feed_rate = movement.feed_rate.has_value() ? movement.feed_rate.value() : feedrate_mm_s;

		if (feed_rate.type() == FeedRateType::MillimetersPerSecond) {
			feed_rate = MMS_SCALED(feed_rate);
		}

		planner.buffer_line(
				targetNative,
				feed_rate,
				active_extruder,
				0.0,
				isnan(movement.accel_mm_s2) ? 0.0 : movement.accel_mm_s2);

		current_position = targetNative;
	}

	void MotionModule::synchronize() {
		planner.synchronize();
	}

	void MotionModule::setActiveCoordinateSystem(CoordinateSystem& coordinateSystem) {
		__activeCoordinateSystemField.set(_pack, coordinateSystem.getIndex());

		_activeCoordinateSystem = &coordinateSystem;

		debug()("wcs: ", coordinateSystem.getIndex());

		if (&_machineCoordinateSystem == &coordinateSystem) {
			_offset = coordinateSystem.getOffset();

			updateTransforms();
		} else {
			setWorkOffset(coordinateSystem.getOffset());
		}
	}

	void MotionModule::updateTransforms() {
		Matrix4f translate {
			{1.0, 0.0, 0.0, -_offset.x()},
			{0.0, 1.0, 0.0, -_offset.y()},
			{0.0, 0.0, 1.0, -_offset.z()},
			{0.0, 0.0, 0.0,         1.0}
		};

		// float32_t shearFactor = std::tan(0.5 * (M_PI/180));
		float32_t shearFactor = 0;

		debug()("shearFactor: ", shearFactor);

		Matrix4f shear {
			{				1.0, 0.0, 0.0, 0.0},
			{shearFactor, 1.0, 0.0, 0.0},
			{				0.0, 0.0, 1.0, 0.0},
			{				0.0, 0.0, 0.0, 1.0}
		};

		_nativeTransform = translate * shear;
		_logicalTransform = _nativeTransform.inverse();
	}

	void MotionModule::updateOffset() {
		Vector3f homeOffset = getHomeOffset();
		Vector3f workOffset = getWorkOffset();
		Vector3f toolOffset = getToolOffset();

		_offset = homeOffset + workOffset + toolOffset;

		debug()("home x: ", homeOffset.x(), ", y: ", homeOffset.y(), ", z: ", homeOffset.z());
		debug()("work x: ", workOffset.x(), ", y: ", workOffset.y(), ", z: ", workOffset.z());
		debug()("tool x: ", toolOffset.x(), ", y: ", toolOffset.y(), ", z: ", toolOffset.z());
		debug()("offset x: ", _offset.x(), ", y: ", _offset.y(), ", z: ", _offset.z());

		updateTransforms();
	}

	swordfish::motion::CoordinateSystem& MotionModule::getActiveCoordinateSystem() {
		if (!_activeCoordinateSystem) {
			setActiveCoordinateSystem(_machineCoordinateSystem);
		}

		return *_activeCoordinateSystem;
	}

	void MotionModule::setHomeOffset(Vector3f homeOffset_) {
		auto& homeOffset = __homeOffsetField.get(_pack);

		homeOffset.x(homeOffset_.x());
		homeOffset.y(homeOffset_.y());
		homeOffset.z(homeOffset_.z());

		updateOffset();
	}

	void MotionModule::setWorkOffset(Vector3f workOffset_) {
		auto& workOffset = __workOffsetField.get(_pack);

		workOffset.x(workOffset_.x());
		workOffset.y(workOffset_.y());
		workOffset.z(workOffset_.z());

		updateOffset();
	}

	void MotionModule::setToolOffset(Vector3f toolOffset_) {
		auto& toolOffset = __toolOffsetField.get(_pack);

		toolOffset.x(toolOffset_.x());
		toolOffset.y(toolOffset_.y());
		toolOffset.z(toolOffset_.z());

		updateOffset();
	}

	swordfish::motion::MotionModule& MotionModule::getInstance(core::Object* parent /*= nullptr*/) {
		return *(__instance ?: __instance = new MotionModule(parent));
	}

	void MotionModule::rapidMove(Movement movement) {
		move({ .x = movement.x,
		       .y = movement.y,
		       .z = movement.z,
		       .feed_rate = movement.feed_rate.has_value() ? movement.feed_rate.value() : FeedRate::MillimetersPerSecond(rapidrate_mm_s),
		       .relative_axes = movement.relative_axes,
		       .accel_mm_s2 = isnan(movement.accel_mm_s2) ? planner.settings.travel_acceleration : movement.accel_mm_s2 });
	}

	void MotionModule::feedMove(Movement movement) {
		move({ .x = movement.x,
		       .y = movement.y,
		       .z = movement.z,
		       .feed_rate = movement.feed_rate.has_value() ? movement.feed_rate.value() : feedrate_mm_s,
		       .relative_axes = movement.relative_axes,
		       .accel_mm_s2 = isnan(movement.accel_mm_s2) ? planner.settings.acceleration : movement.accel_mm_s2 });
	}
} // namespace swordfish::motion