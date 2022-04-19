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
	class ToolTable : public data::Table<Tool, ToolTable> {
		friend class data::Table<Tool, ToolTable>;

	public:
		ToolTable(core::Object* parent);

		virtual const char* getName() override {
			return "tool";
		}

	protected:
		void removeInternal(Tool& tool);
	};
} // namespace swordfish::tools