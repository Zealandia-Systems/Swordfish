/*
 * DriverException.cpp
 *
 * Created: 21/10/2021 1:42:19 pm
 *  Author: smohekey
 */ 

#include <swordfish/io/Writer.h>

#include "DriverException.h"

namespace swordfish::tools {
	void DriverException::writeType(io::Writer& writer) const {
		writer << "DriverException";
	}

	void DriverException::writeMessage(io::Writer& writer) const {
		writer << _message;
	}
}