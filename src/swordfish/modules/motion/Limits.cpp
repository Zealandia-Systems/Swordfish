/*
 * Limits.cpp
 *
 * Created: 27/08/2021 4:47:51 pm
 *  Author: smohekey
 */

#include <algorithm>

#include <swordfish/debug.h>

#include <marlin/module/motion.h>

#include "Limits.h"
#include "LimitException.h"

using namespace Eigen;

namespace swordfish::motion {
	core::Schema Limits::__schema = {
		utils::typeName<Limits>(),
		&(core::Boundary<core::LinearVector3>::__schema),
		{

		},
		{

		}
	};

	void Limits::clamp(Vector3f& vector) {
		if (_enabled) {
			Vector3f min = getMin();
			Vector3f max = getMax();

			if (axis_was_homed(Axis::X())) {
				vector.x() = std::clamp(vector.x(), min.x(), max.x());
			}

			if (axis_was_homed(Axis::Y())) {
				vector.y() = std::clamp(vector.y(), min.y(), max.y());
			}

			if (axis_was_homed(Axis::Z())) {
				vector.z() = std::clamp(vector.z(), min.z(), max.z());
			}
		}
	}

	/*void Limits::throwIfOutside(math::Vector3f& vector) {
	  if(_enabled) {
	    math::Vector3f min = getMin();
	    math::Vector3f max = getMax();

	    //debug()("x: ", vector.x, ", min_x: ", min.x, ", max_x: ", max.x);
	    //debug()("y: ", vector.y, ", min_y: ", min.y, ", max_y: ", max.y);
	    //debug()("z: ", vector.z, ", min_z: ", min.z, ", max_z: ", max.z);

	    if(
	    (vector.x != std::clamp(vector.x, min.x, max.x)) ||
	    (vector.y != std::clamp(vector.y, min.y, max.y)) ||
	    (vector.z != std::clamp(vector.z, min.z, max.z))
	    ) {
	      throw LimitException(vector, min, max);
	    }
	  }
	}*/

	void Limits::throwIfOutside(const Vector3f& vector) {
		if (_enabled) {
			Vector3f min = getMin();
			Vector3f max = getMax();

			// debug()("x: ", vector.x, ", min_x: ", min.x, ", max_x: ", max.x);
			// debug()("y: ", vector.y, ", min_y: ", min.y, ", max_y: ", max.y);
			// debug()("z: ", vector.z, ", min_z: ", min.z, ", max_z: ", max.z);

			if (
					(vector.x() != std::clamp(vector.x(), min.x(), max.x())) ||
					(vector.y() != std::clamp(vector.y(), min.y(), max.y())) ||
					(vector.z() != std::clamp(vector.z(), min.z(), max.z()))) {
				throw LimitException(vector, min, max);
			}
		}
	}
} // namespace swordfish::motion
