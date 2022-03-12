/*
 * UpBuffer.h
 *
 * Created: 23/08/2021 12:05:58 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

namespace swordfish::trace::rtt {
	class UpBuffer {
		const char* _name;
		uint8_t* _buffer;
		uint32_t _length;
		uint32_t _writeOffset;
		volatile uint32_t _readOffset;         // Position of next item to be read by host. Must be volatile since it may be modified by host.
		uint32_t _flags;
	};
}