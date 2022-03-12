/*
 * InputStream.h
 *
 * Created: 8/08/2021 6:34:29 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

namespace swordfish::io {
	class InputStream {
	public:
		virtual ~InputStream() noexcept(false) { }
		
		virtual size_t read(void* buffer, size_t length) = 0;
	};
}