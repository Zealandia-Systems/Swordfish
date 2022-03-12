/*
 * InvalidOperationException.h
 *
 * Created: 23/11/2021 2:59:29 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>
#include <swordfish/Exception.h>

namespace swordfish::core {
	class InvalidOperationException : public Exception {
	private:
		const char* _message;
		
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
		
	public:
		InvalidOperationException(const char* message) : _message(message) {
			
		}
		
		virtual ~InvalidOperationException() { }
	};
}