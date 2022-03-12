/*
 * DriverTable.cpp
 *
 * Created: 18/10/2021 1:00:44 pm
 *  Author: smohekey
 */ 

#include "DriverTable.h"
#include "Driver.h"

namespace swordfish::tools {
	DriverTable::DriverTable(core::Object* parent) : data::Table<Driver>(parent) {
		
	}

	DriverTable::~DriverTable() {

	}
}