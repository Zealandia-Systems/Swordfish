/*
 * FormatException.cpp
 *
 * Created: 9/11/2021 1:53:31 pm
 *  Author: smohekey
 */ 

#include "FormatException.h"

#include <swordfish/io/Writer.h>

namespace swordfish::core {
	void FormatException::writeType(io::Writer& writer) const {
		writer << "FormatException";	
	}

	void FormatException::writeMessage(io::Writer& writer) const {
		writer << _message;
	}
}