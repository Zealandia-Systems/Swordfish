/*
 * Pockets.h
 *
 * Created: 7/09/2021 8:02:36 am
 *  Author: smohekey
 */

#pragma once

#include <swordfish/data/Table.h>

#include "Pocket.h"

namespace swordfish::tools {
	class PocketTable : public data::Table<Pocket, PocketTable> {
	public:
		PocketTable(core::Object* parent);

		virtual const char* getName() override {
			return "pocket";
		}
	};
} // namespace swordfish::tools