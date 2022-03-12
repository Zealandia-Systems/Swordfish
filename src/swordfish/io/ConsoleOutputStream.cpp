/*
 * ConsoleOutputStream.cpp
 *
 * Created: 15/08/2021 9:32:48 pm
 *  Author: smohekey
 */ 

#include <Arduino.h>

#include "ConsoleOutputStream.h"

namespace swordfish::io {
	size_t ConsoleOutputStream::write(const void* buffer, size_t length) {
		return Serial.write((const uint8_t*)buffer, length);
	}
}