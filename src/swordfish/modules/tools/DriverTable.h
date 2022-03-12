/*
 * DriverTable.h
 *
 * Created: 18/10/2021 11:54:10 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/data/Table.h>

#include "Driver.h"

namespace swordfish::tools {
	class DriverTable : public data::Table<Driver> {
	public:
		DriverTable(core::Object* parent);
		virtual ~DriverTable();
		
		virtual const char* getName() override { return "driver"; }
	};
}