/*
 * RTTSerial.cpp
 *
 * Created: 26/02/2021 3:40:40 pm
 *  Author: smohekey
 */ 

#include "RTTSerial.h"
#include "SEGGER_RTT.h"

size_t RTTSerial::write(uint8_t c) {
	return SEGGER_RTT_PutChar(0, c);
}

int RTTSerial::available() {
	return SEGGER_RTT_HasData(0);
}

int RTTSerial::read() {
	char buffer;
	
	return 1 == SEGGER_RTT_Read(0, &buffer, 1);
}

int RTTSerial::peek() {
	return -1;
}

/*int RTTSerial::printf(const char* format, ...) {
	va_list args;
	va_start (args, format);
	
	auto r = SEGGER_RTT_vprintf(format, args);
	
	va_end (args);
	
	return r;
}*/

RTTSerial RTTSerial1;