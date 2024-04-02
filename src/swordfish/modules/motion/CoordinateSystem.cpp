/*
 * WorkSystem.cpp
 *
 * Created: 8/09/2021 2:34:53 pm
 *  Author: smohekey
 */

#include <swordfish/core/Object.h>
#include <swordfish/debug.h>

#include "MotionModule.h"
#include "CoordinateSystem.h"

namespace swordfish::motion {
	core::ValueField<int16_t> CoordinateSystem::__indexField = { "index", 0, 0 };

	core::ObjectField<core::LinearVector3> CoordinateSystem::__offsetField = { "offset", 0 };
	core::ObjectField<core::Vector3> CoordinateSystem::__rotationField = { "rotation", 1 };

	core::Schema CoordinateSystem::__schema = {
		utils::typeName<CoordinateSystem>(),
		nullptr,
		{ __indexField },
		{ __offsetField,
		  __rotationField }
	};

	CoordinateSystem::CoordinateSystem(core::Object* parent) :
			data::Record(parent), _pack(__schema, *this) {
	}

	/*void CoordinateSystem::offset(math::Vector3f value) {
	  debug()("x: ", value.x, "y: ", value.y, "z: ", value.z);

	  offset().set(value);

	  auto& activeSystem = _motionManager.getActiveCoordinateSystem();

	  if(&activeSystem == this) {
	    _motionManager.setActiveCoordinateSystem(*this);
	  }
	}*/
} // namespace swordfish::motion