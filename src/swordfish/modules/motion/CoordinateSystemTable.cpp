/*
 * CoordinateSystems.cpp
 *
 * Created: 9/09/2021 12:17:40 pm
 *  Author: smohekey
 */

#include "CoordinateSystemTable.h"
#include "CoordinateSystem.h"
#include "MotionModule.h"

namespace swordfish::motion {
	CoordinateSystemTable::CoordinateSystemTable(Object* parent) :
			data::Table<CoordinateSystem, CoordinateSystemTable>(parent) {
	}
} // namespace swordfish::motion