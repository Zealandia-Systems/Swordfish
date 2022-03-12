/*
 * Tools.cpp
 *
 * Created: 7/09/2021 8:25:53 am
 *  Author: smohekey
 */ 

#include "ToolTable.h"
#include "Tool.h"
#include "PocketTable.h"
#include "Pocket.h"
#include "ToolsModule.h"

namespace swordfish::tools {
	using namespace swordfish::core;
	
	ToolTable::ToolTable(Object* parent) : data::Table<Tool>(parent) {
		
	}

	void ToolTable::remove(Tool& tool) {
		auto& toolsModule = ToolsModule::getInstance();
		auto& pockets = toolsModule.getPockets();
		
		for(auto& pocket : pockets) {
			if(pocket.getToolIndex() == tool.getIndex()) {
				pocket.setToolIndex(-1);
			}
		}
		
		ObjectList<Tool>::remove(tool);
	}
}
