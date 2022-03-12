/*
 * DuplicateIndexException.h
 *
 * Created: 22/11/2021 2:44:02 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>
#include <swordfish/Exception.h>

namespace swordfish::core {
	class DuplicateIndexException : public Exception {
	private:
		int16_t _index;
	
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
	
	public:
		DuplicateIndexException(int16_t index) : _index(index) {
			
		}
		
		virtual ~DuplicateIndexException() { }
	};
}