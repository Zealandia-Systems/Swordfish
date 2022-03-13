/*
 * Trace.cpp
 *
 * Created: 23/08/2021 1:14:34 pm
 *  Author: smohekey
 */ 
#include <sam.h>
#include <Arduino.h>

#include "debug.h"

#include <marlin/libs/rtt/SEGGER_RTT.h>

#include <swordfish/io/OutputStream.h>

namespace swordfish {
	class TraceStream : public io::OutputStream {
	public:
		size_t write(const void* buffer, size_t length) override {
			if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
				SEGGER_RTT_Write(0, buffer, length);
			} else {
				//Serial.write((const uint8_t*)buffer, length);
			}
				
			return length;
		}
	} stream;
		
	io::Writer out(stream);
}