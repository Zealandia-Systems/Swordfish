/*
 * DriverParameterDefinition.h
 *
 * Created: 18/10/2021 1:40:30 pm
 *  Author: smohekey
 */

#pragma once

#include <swordfish/types.h>

namespace swordfish::tools {
	enum class DriverParameterType {
		Integer,
		String,
	};

	struct DriverParameterDefinition {
		const u16 id;
		const char* name;
		const DriverParameterType type;
	};
}