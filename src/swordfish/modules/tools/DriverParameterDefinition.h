/*
 * DriverParameterDefinition.h
 *
 * Created: 18/10/2021 1:40:30 pm
 *  Author: smohekey
 */ 

#pragma once

namespace swordfish::tools {
	enum class DriverParameterType {
		Integer,
		String,	
	};
	
	struct DriverParameterDefinition {
		const uint16_t id;
		const char* name;
		const DriverParameterType type;
	};
}