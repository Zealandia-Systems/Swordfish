/*
 * DriverParameterTable.cpp
 *
 * Created: 18/10/2021 1:14:51 pm
 *  Author: smohekey
 */

#include "DriverParameterTable.h"
#include "DriverParameter.h"

namespace swordfish::tools {
	DriverParameterTable::DriverParameterTable(core::Object* parent) :
			data::Table<DriverParameter, DriverParameterTable>(parent) {
	}
} // namespace swordfish::tools