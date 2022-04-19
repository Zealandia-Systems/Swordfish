/*
 * DriverParameterTable.h
 *
 * Created: 18/10/2021 11:47:31 am
 *  Author: smohekey
 */

#pragma once

#include <swordfish/data/Table.h>

#include "DriverParameter.h"

namespace swordfish::tools {
	class DriverParameterTable : public data::Table<DriverParameter, DriverParameterTable> {
	public:
		DriverParameterTable(core::Object* parent);

		virtual const char* getName() override {
			return "driverParameter";
		}
	};
} // namespace swordfish::tools