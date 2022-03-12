/*
 * M222.cpp
 *
 * Created: 6/05/2021 10:07:32 am
 *  Author: smohekey
 */

#include "../../inc/MarlinConfig.h"
#include "../gcode.h"

#if HAS_FAST_MOVES

extern int16_t rapidrate_percentage;
/**
 * M222: Set rapid rate percentage (M221 S95)
 *
 * Parameters
 *   S<percent> : Set the rapid rate percentage factor
 *
 * Report the current rapid rate percentage factor if no parameter is specified
 */
void GcodeSuite::M222() {
	static int16_t backup_rapidrate_percentage = 100;
	if (parser.seen('B'))
		backup_rapidrate_percentage = rapidrate_percentage;
	if (parser.seen('R'))
		rapidrate_percentage = backup_rapidrate_percentage;

	if (parser.seenval('S'))
		rapidrate_percentage = parser.value_int();

	if (!parser.seen_any()) {
		SERIAL_ECHOPAIR("RR:", rapidrate_percentage);
		SERIAL_CHAR('%');
		SERIAL_EOL();
	}
}

#endif
