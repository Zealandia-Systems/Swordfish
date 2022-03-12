/*
 * StringOutputStream.cpp
 *
 * Created: 17/12/2021 3:58:02 pm
 *  Author: smohekey
 */ 

#include "StringOutputStream.h"

namespace swordfish::io {
	StringOutputStream::StringOutputStream(char* buffer, size_t length) : _buffer(buffer), _length(length), _index(0) {
		
	}
	
	size_t StringOutputStream::write(const void* buffer, size_t length) {
		auto i = 0u;
		
		while(_index < _length && i < length) {
			_buffer[_index++] = ((const char*)buffer)[i++];
		}
		
		return i;
	}
}