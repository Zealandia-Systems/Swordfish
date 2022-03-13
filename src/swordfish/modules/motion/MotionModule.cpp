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
#include <swordfish/modules/estop/EStopModule.h>

namespace swordfish::motion {
	using namespace Eigen;
	
	using namespace swordfish::core;
	using namespace swordfish::estop;
	using namespace swordfish::utils;
	
	MotionModule* MotionModule::__instance = nullptr;
	
	core::ValueField<int16_t> MotionModule::__activeCoordinateSystemField = { "wcs", 0, 0 };
	core::ObjectField<CoordinateSystemTable> MotionModule::__workCoordinateSystemsField = { "workCoordinateSystems", 0 };
	core::ObjectField<Limits> MotionModule::__limitsField = { "limits", 1 };
	core::ObjectField<core::Vector3> MotionModule::__homeOffsetField = { "homeOffset", 2 };
	core::ObjectField<core::Vector3> MotionModule::__workOffsetField = { "workOffset", 3 };
	core::ObjectField<core::Vector3> MotionModule::__toolOffsetField = { "toolOffset", 4 };
	
	core::Schema MotionModule::__schema = {
		utils::typeName<MotionModule>(),
		&(Module::__schema), {
			__activeCoordinateSystemField
			}, {
			__workCoordinateSystemsField,
			__limitsField,
			__homeOffsetField,
			__workOffsetField,
			__toolOffsetField
		}
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
		
		if(coordinateSystem) {
			setActiveCoordinateSystem(*coordinateSystem);
		}
		
		updateOffset();
	}
	
	void MotionModule::init() {
		auto& coordinateSystems = getWorkCoordinateSystems();
		auto activeCoordinateSystem = __activeCoordinateSystemField.get(_pack);
		
		for(auto i = 0; i < 60; i++) {
			auto& coordinateSystem = coordinateSystems.emplaceBack();	
			
			coordinateSystem.setIndex(i);
			
			if(i == activeCoordinateSystem) {
				setActiveCoordinateSystem(coordinateSystem);
			} else if(i == 58) {
				_toolChangeCoordinateSystem = &coordinateSystem;
			} else if(i == 59) {
				_toolCoordinateSystem = &coordinateSystem;
			}
		}
		
		updateOffset();
	}

	static inline const char* absoluteOrRelative(AxisSelector axis, Flags<AxisSelector>& relativeAxis) {
		return relativeAxis & axis ? "relative" : "absolute";
	}
	
	void MotionModule::move(Movement movement) {
		/*EStopModule::getInstance().throwIfTriggered();
		
		debug()("current -> x: ", current_position.x, ", y: ", current_position.y, ", z: ", current_position.z, ", feedRate: ", feedrate_mm_s);
		debug()("requested -> x: ", movement.x, ", y: ", movement.y, ", z: ", movement.z, ", feedRate: ", movement.feedRate, ", relative: ", movement.relative);
		
		Vector3f dest = {
			isnan(movement.x) ? current_position.x : (movement.relative ? current_position.x + movement.x : movement.x - _offset(X)),
			isnan(movement.y) ? current_position.y : (movement.relative ? current_position.y + movement.y : movement.y - _offset(Y)),
			isnan(movement.z) ? current_position.z : (movement.relative ? current_position.z + movement.z : movement.z - _offset(Z))
		};
		
		_limits.throwIfOutside(dest);
		
		destination.x = dest(X);
		destination.y = dest(Y);
		destination.z = dest(Z);
		
		feedRate_t old_feedrate = feedrate_mm_s;
		
		try {
			feedrate_mm_s = isnan(movement.feedRate) ? feedrate_mm_s : movement.feedRate;
		
			debug()("actual -> x: ", destination.x, ", y: ", destination.y, ", z: ", destination.z, ", f: ", feedrate_mm_s);
		
			prepare_line_to_destination();
			planner.synchronize();
		} catch {
			feedrate_mm_s = old_feedrate;
			
			throw;
		}*/
		
		EStopModule::getInstance().throwIfTriggered();

		Vector4f currentNative { current_position.x, current_position.y, current_position.z, 1 };
		Vector4f currentLogical = _logicalTransform * currentNative;
		Vector4f targetLogical {
			isnan(movement.x) ? currentLogical(X) : (movement.relativeAxes & AxisSelector::X ? currentLogical(X) + movement.x : movement.x),
			isnan(movement.y) ? currentLogical(Y) : (movement.relativeAxes & AxisSelector::Y ? currentLogical(Y) + movement.y : movement.y),
			isnan(movement.z) ? currentLogical(Z) : (movement.relativeAxes & AxisSelector::Z ? currentLogical(Z) + movement.z : movement.z),
			1
		};

		Vector4f targetNative = _nativeTransform * targetLogical;

		debug()("===============================================");
		debug()(
			"requested -> x(", absoluteOrRelative(AxisSelector::X, movement.relativeAxes), "): ", movement.x,
			", y(", absoluteOrRelative(AxisSelector::Y, movement.relativeAxes), "): ", movement.y,
			", z(", absoluteOrRelative(AxisSelector::Z, movement.relativeAxes), "): ", movement.z,
			", feedRate: ", movement.feedRate
		);
		debug()("current native -> x: ", currentNative(X), ", y: ", currentNative(Y), ", z: ", currentNative(Z));
		debug()("current logical -> x: ", currentLogical(X), ", y: ", currentLogical(Y), ", z: ", currentLogical(Z));
		debug()("target logical -> x: ", targetLogical(X), ", y: ", targetLogical(Y), ", z: ", targetLogical(Z));
		debug()("target native -> x: ", targetNative(X), ", y: ", targetNative(Y), ", z: ", targetNative(Z));

		_limits.throwIfOutside({ targetNative(X), targetNative(Y), targetNative(Z) });

		xyz_pos_t target = {
			targetNative(X),
			targetNative(Y),
			targetNative(Z)
		};

		planner.buffer_line(
			target,
			MMS_SCALED(isnan(movement.feedRate) ? feedrate_mm_s : movement.feedRate),
			active_extruder,
			0.0,
			isnan(movement.accel_mm_s2) ? 0.0 : movement.accel_mm_s2
		);

		current_position = target;
	}
	
	void MotionModule::synchronize() {
		planner.synchronize();
	}

	void MotionModule::setActiveCoordinateSystem(CoordinateSystem& coordinateSystem) {
		__activeCoordinateSystemField.set(_pack, coordinateSystem.getIndex());
		
		_activeCoordinateSystem = &coordinateSystem;
	
		debug()("wcs: ", coordinateSystem.getIndex());
		
		if(&_machineCoordinateSystem == &coordinateSystem) {
			_offset = coordinateSystem.getOffset();
			
			updateTransforms();
		} else {
			setWorkOffset(coordinateSystem.getOffset());
		}
	}

	void MotionModule::updateTransforms() {
		Matrix4f translate {
			{ 1.0, 0.0, 0.0, -_offset(X) },
			{ 0.0, 1.0, 0.0, -_offset(Y) },
			{ 0.0, 0.0, 1.0, -_offset(Z) },
			{ 0.0, 0.0, 0.0, 1.0 }
		};
		
		//float32_t shearFactor = std::tan(0.5 * (M_PI/180));
		float32_t shearFactor = 0;
		
		debug()("shearFactor: ", shearFactor);
		
		Matrix4f shear {
			{ 1.0, 0.0, 0.0, 0.0 },
			{	shearFactor, 1.0, 0.0, 0.0 },
			{ 0.0, 0.0, 1.0, 0.0 },
			{ 0.0, 0.0, 0.0, 1.0 }
		};
		
		_nativeTransform = translate * shear;
		_logicalTransform = _nativeTransform.inverse();
	}
	
	void MotionModule::updateOffset() {
		Vector3f homeOffset = getHomeOffset();
		Vector3f workOffset = getWorkOffset();
		Vector3f toolOffset = getToolOffset();
		
		_offset = homeOffset + workOffset + toolOffset;
		
		debug()("home x: ", homeOffset(X), ", y: ", homeOffset(Y), ", z: ", homeOffset(Z));
		debug()("work x: ", workOffset(X), ", y: ", workOffset(Y), ", z: ", workOffset(Z));
		debug()("tool x: ", toolOffset(X), ", y: ", toolOffset(Y), ", z: ", toolOffset(Z));
		
		debug()("offset x: ", _offset(X), ", y: ", _offset(Y), ", z: ", _offset(Z));
		
		updateTransforms();
	}

	swordfish::motion::CoordinateSystem& MotionModule::getActiveCoordinateSystem() {
		if(!_activeCoordinateSystem) {
			setActiveCoordinateSystem(_machineCoordinateSystem);
		}
	
		return *_activeCoordinateSystem;
	}
	
	void MotionModule::setHomeOffset(Vector3f homeOffset_) {
		auto& homeOffset = __homeOffsetField.get(_pack);
		
		homeOffset.x(homeOffset_(X));
		homeOffset.y(homeOffset_(Y));
		homeOffset.z(homeOffset_(Z));
		
		updateOffset();
	}

	void MotionModule::setWorkOffset(Vector3f workOffset_) {
		auto& workOffset = __workOffsetField.get(_pack);
	
		workOffset.x(workOffset_(X));
		workOffset.y(workOffset_(Y));
		workOffset.z(workOffset_(Z));
	
		updateOffset();
	}

	void MotionModule::setToolOffset(Vector3f toolOffset_) {
		auto& toolOffset = __toolOffsetField.get(_pack);
		
		toolOffset.x(toolOffset_(X));
		toolOffset.y(toolOffset_(Y));
		toolOffset.z(toolOffset_(Z));
	
		updateOffset();
	}

	swordfish::motion::MotionModule& MotionModule::getInstance(core::Object* parent /*= nullptr*/) {
		return *(__instance ?: __instance = new MotionModule(parent));
	}

	void MotionModule::rapidMove(Movement movement) {
		move({
			.x = movement.x,
			.y = movement.y,
			.z = movement.z,
			.feedRate = isnan(movement.feedRate) ? rapidrate_mm_s : movement.feedRate,
			.relativeAxes = movement.relativeAxes,
			.accel_mm_s2 = isnan(movement.accel_mm_s2) ? planner.settings.travel_acceleration : movement.accel_mm_s2
		});
	}

	void MotionModule::feedMove(Movement movement) {
		move({
			.x = movement.x,
			.y = movement.y,
			.z = movement.z,
			.feedRate = isnan(movement.feedRate) ? feedrate_mm_s : movement.feedRate,
			.relativeAxes = movement.relativeAxes,
			.accel_mm_s2 = isnan(movement.accel_mm_s2) ? planner.settings.acceleration : movement.accel_mm_s2
		});
	}
}