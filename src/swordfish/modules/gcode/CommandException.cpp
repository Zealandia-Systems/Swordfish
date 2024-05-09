/*
 * CommandException.cpp
 *
 * Created: 10/11/2021 12:25:06 pm
 *  Author: smohekey
 */

#include <swordfish/io/Writer.h>

#include "CommandException.h"

namespace swordfish {
	void CommandException::writeType(io::Writer& writer) const {
		writer << "CommandException";
	}

	void CommandException::writeMessage(io::Writer& writer) const {
		writer << _message;
	}
}
