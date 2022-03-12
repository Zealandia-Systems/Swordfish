/*
 * Tools.h
 *
 * Created: 7/09/2021 7:59:27 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/data/Table.h>

#include "Tool.h"

namespace swordfish::tools {
	class ToolTable : public data::Table<Tool> {
	public:
		ToolTable(core::Object* parent);
		
		virtual const char* getName() override { return "tool"; }
		
		virtual void remove(Tool& tool) override;
	};
}