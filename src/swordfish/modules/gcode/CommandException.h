/*
 * CommandException.h
 *
 * Created: 10/11/2021 12:23:08 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/Exception.h>

namespace swordfish::gcode {
	class CommandException : public Exception {
	private:
		const char* _message;
		
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
	
	public:
		CommandException(const char* message) : _message(message) {
			
		}
		
		virtual ~CommandException() { }
	};
}