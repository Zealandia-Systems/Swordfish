/*
 * Pockets.cpp
 *
 * Created: 7/09/2021 8:27:09 am
 *  Author: smohekey
 */

#include "PocketTable.h"
#include "Pocket.h"

#include "ToolTable.h"
#include "Tool.h"

#include "ToolsModule.h"

namespace swordfish::tools {
	using namespace swordfish::core;

	PocketTable::PocketTable(Object* parent) :
			data::Table<Pocket, PocketTable>(parent) {
	}
} // namespace swordfish::tools