/*
 * InvalidOperationException.cpp
 *
 * Created: 23/11/2021 3:00:32 pm
 *  Author: smohekey
 */ 

#include <swordfish/io/Writer.h>

#include "InvalidOperationException.h"

namespace swordfish::core {
	void InvalidOperationException::writeType(io::Writer& writer) const {
		writer << "InvalidOperationException";
	}

	void InvalidOperationException::writeMessage(io::Writer& writer) const {
		writer << _message;
	}
}