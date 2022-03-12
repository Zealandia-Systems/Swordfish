/*
 * Exception.cpp
 *
 * Created: 30/09/2021 1:27:27 pm
 *  Author: smohekey
 */ 

#include <swordfish/io/Writer.h>

#include "Exception.h"

namespace swordfish {
	using namespace swordfish::io;
	
	void Exception::writeJson(Writer& writer) const {
		writer << "{\"type\":\"";
		
		writeType(writer);
		
		writer << "\",\"message\":\"";
		
		writeMessage(writer);
		
		writer << "\"}";
	}
}