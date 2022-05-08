/*
 * Tool.cpp
 *
 * Created: 22/11/2021 2:55:12 pm
 *  Author: smohekey
 */

#include <swordfish/core/DuplicateIndexException.h>
#include <swordfish/core/InvalidOperationException.h>

#include "Tool.h"
#include "ToolsModule.h"

namespace swordfish::tools {
	core::ValidatedValueField<int16_t> Tool::__indexField = { "index", 0, 0, validateIndex };
	core::ValueField<bool> Tool::__fixedField = { "fixed", 32, false };
	core::ValueField<bool> Tool::__needsProbeField = { "needsProbe", 33, true };
	core::ValueField<uint8_t> Tool::__driverIndexField = { "driver", 40, 0 };
	core::ObjectField<ToolGeometry> Tool::__geometryField = { "geometry", 0 };
	core::ObjectField<core::String> Tool::__descriptionField = { "description", 1 };
	core::ObjectField<core::Vector3> Tool::__offsetField = { "offset", 2 };
	core::TransientField<Tool, int16_t> Tool::__pocketField = { "pocket", getPocketIndex, setPocketIndex };

	core::Schema Tool::__schema = {
		utils::typeName<Tool>(),
		nullptr,
		{   __indexField,
		  __fixedField,
		  __needsProbeField,
		  __driverIndexField },
		{__geometryField,
		  __descriptionField,
		  __offsetField },
		{  __pocketField }
	};

	void Tool::validateIndex(int16_t oldValue, int16_t newValue) {
		auto& toolsModule = ToolsModule::getInstance();
		auto& tools = toolsModule.getTools();

		if (oldValue == newValue) {
			return;
		}

		for (auto& tool : tools) {
			debug()("index: ", tool.getIndex());

			if (tool.getIndex() == newValue) {
				throw core::DuplicateIndexException { newValue };
			}
		}
	}

	int16_t Tool::getPocketIndex(Tool& tool) {
		auto& toolsModule = ToolsModule::getInstance();
		auto& pockets = toolsModule.getPockets();

		for (auto& pocket : pockets) {
			if (pocket.getToolIndex() == tool.getIndex()) {
				return pocket.getIndex();
			}
		}

		return -1;
	}

	void Tool::setPocketIndex(Tool& tool, int16_t pocketIndex) {
		auto& toolsModule = ToolsModule::getInstance();
		auto& pockets = toolsModule.getPockets();

		if (pocketIndex == -1) {
			for (auto& pocket : pockets) {
				if (pocket.getToolIndex() == tool.getIndex()) {
					pocket.setToolIndex(-1);

					return;
				}
			}
		} else {
			// remove tool from existing pocket first

			auto toolIndex = tool.getIndex();

			for (auto& pocket : pockets) {
				if (pocket.getToolIndex() == toolIndex) {
					pocket.setToolIndex(-1);

					break;
				}
			}

			// find the requested pocket and set the tool
			for (auto& pocket : pockets) {
				if (pocket.getIndex() == pocketIndex) {
					auto currentToolIndex = pocket.getToolIndex();

					if (currentToolIndex != -1 && currentToolIndex != toolIndex) {
						throw core::InvalidOperationException { "Pocket isn't empty." };
					}

					pocket.setToolIndex(toolIndex);

					return;
				}
			}

			throw core::InvalidOperationException { "Pocket not found." };
		}
	}
} // namespace swordfish::tools