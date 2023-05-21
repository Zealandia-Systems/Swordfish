/*
 * Pocket.cpp
 *
 * Created: 23/11/2021 2:37:19 pm
 *  Author: smohekey
 */

#include <swordfish/core/DuplicateIndexException.h>

#include "Pocket.h"

#include "ToolsModule.h"

namespace swordfish::tools {
	core::ValidatedValueField<int16_t> Pocket::__indexField = { "index", 0, 0, validateIndex };
	core::ValueField<int16_t> Pocket::__toolIndexField = { "tool", 16, -1 };
	core::ValueField<bool> Pocket::__enabledField = { "enabled", 32, true };
	core::ValueField<bool> Pocket::__readOnlyField = { "readOnly", 33, false };
	core::LinearValueField<float32_t> Pocket::__depthField = { "depth", 96, 150.0 };

	core::ObjectField<core::LinearVector3> Pocket::__offsetField = { "offset", 0 };

	core::Schema Pocket::__schema = {
		utils::typeName<Pocket>(),
		nullptr,
		{ __indexField,
		  __toolIndexField,
		  __enabledField,
		  __readOnlyField,
		  __depthField },
		{ __offsetField }
	};

	void Pocket::validateIndex(int16_t oldValue, int16_t newValue) {
		auto& toolsModule = ToolsModule::getInstance();
		auto& pockets = toolsModule.getPockets();

		if (oldValue == newValue) {
			return;
		}

		for (auto& pocket : pockets) {
			debug()("index: ", pocket.getIndex());

			if (pocket.getIndex() == newValue) {
				throw core::DuplicateIndexException { newValue };
			}
		}
	}
} // namespace swordfish::tools