/*
 * CoordinateSystems.h
 *
 * Created: 9/09/2021 12:15:35 pm
 *  Author: smohekey
 */

#pragma once

#include <swordfish/data/Table.h>

#include "CoordinateSystem.h"

namespace swordfish::motion {
	class CoordinateSystemTable : public data::Table<CoordinateSystem, CoordinateSystemTable> {
	public:
		CoordinateSystemTable(Object* object);

		virtual const char* getName() override {
			return "wcs";
		};
	};
} // namespace swordfish::motion