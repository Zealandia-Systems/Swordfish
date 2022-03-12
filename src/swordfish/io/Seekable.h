/*
 * Seekable.h
 *
 * Created: 11/08/2021 9:23:12 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

#include "InputStream.h"
#include "OutputStream.h"

namespace swordfish::io {
	enum class Origin {
		Start,
		Current,
		End
	};
	
	class Seekable {
	public:
		virtual ~Seekable() noexcept(false) { }
		
		virtual offset_t seek(const offset_t offset, const Origin origin) = 0;
	};
	
	class SeekableInputStream : public InputStream, public Seekable {
		
	};
	
	class SeekableOutputStream : public OutputStream, public Seekable {
		
	};
}