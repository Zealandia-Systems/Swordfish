/*
 * FormatException.h
 *
 * Created: 9/11/2021 1:00:27 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/Exception.h>

namespace swordfish::core {
	class FormatException : public Exception {
	private:
		const char* _message;
		
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
		
	public:
		FormatException(const char* message) : _message(message) {
			
		}
		
		virtual ~FormatException() { }
	};	
};