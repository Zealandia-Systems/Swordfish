/*
 * WrappingInputStream.h
 *
 * Created: 11/08/2021 9:10:23 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

#include "InputStream.h"
#include "Seekable.h"

namespace swordfish::io {
	class WrappingInputStream : public SeekableInputStream {
	protected:
		SeekableInputStream& _inner;
		offset_t _start;
		offset_t _end;
		offset_t _offset;
		
	public:
		WrappingInputStream(SeekableInputStream& inner, offset_t start, offset_t end, offset_t offset) : _inner(inner), _start(start), _end(end), _offset(start + offset) {
			_inner.seek(_offset, Origin::Start);
		}
		
		uint8_t readByte() {
			if(++_offset == _end) {
				_offset = _start;
				
				_inner.seek(_offset, Origin::Start);
			}
			
			uint8_t value;
			
			_inner.read(&value, 1);
			
			return value;
		}
		
		offset_t seek(offset_t offset, Origin origin) override {
			switch(origin) {
				case Origin::Start: {
					offset += _start;
					
					break;
				}
				
				case Origin::Current: {
					offset = _offset + offset;
					
					break;
				}
				
				case Origin::End: {
					offset += _end;
					
					break;
				}
			}
			
			if(offset < _start) {
				offset = _start;
			}
			
			if(offset >= _end) {
				offset = _end - 1;
			}
			
			_offset = _inner.seek(offset, Origin::Start);
			
			return _offset - _start;
		}
		
		size_t read(void* buffer, size_t length) override {
			for(auto i = 0u; i < length; i++) {
				((uint8_t*)buffer)[i] = readByte();
			}
			
			return length;
		}
	};
}