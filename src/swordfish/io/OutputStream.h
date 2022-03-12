/*
 * OutputStream.h
 *
 * Created: 8/08/2021 6:45:24 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

namespace swordfish::io {
	class OutputStream {
	public:
		virtual ~OutputStream() noexcept(false) { }
	
		virtual size_t write(const void* buffer, size_t length) = 0;
	};
}