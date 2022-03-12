/*
 * M82.cpp
 *
 * Created: 29/07/2021 12:28:39 pm
 *  Author: smohekey
 */ 

#include <swordfish/Controller.h>

#include "../../inc/MarlinConfig.h"

#include "../gcode.h"

#if ENABLED(HAS_ATC)

using namespace swordfish::tools;

void GcodeSuite::M82() {
	ToolsModule::getInstance().unlock();
}


void GcodeSuite::M86() {
	ToolsModule::getInstance().lock();
}

#endif