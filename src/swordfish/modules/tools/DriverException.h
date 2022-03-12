/*
 * DriverException.h
 *
 * Created: 21/10/2021 1:35:20 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/Exception.h>

namespace swordfish::io {
	class Writer;
}
	
namespace swordfish::tools {
	class DriverException : public Exception {
	private:
		const char* _message;
	
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
		
	public:
		DriverException(const char* message) : _message(message) {
			
		}	
		
		virtual ~DriverException() {
			
		}
	};
}