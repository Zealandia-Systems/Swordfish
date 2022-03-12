/*
 * StringOutputStream.h
 *
 * Created: 17/12/2021 3:56:31 pm
 *  Author: smohekey
 */ 

#pragma once

#include "OutputStream.h"

namespace swordfish::io {
	class StringOutputStream : public OutputStream {
	private:
		char* _buffer;
		size_t _length;
		size_t _index;
		
	public:
		StringOutputStream(char* buffer, size_t length);
		size_t write(const void* buffer, size_t length) override;
	};
}