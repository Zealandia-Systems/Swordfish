/*
 * NotHomedException.cpp
 *
 *  Author: smohekey
 */

#include <swordfish/io/Writer.h>

#include "NotHomedException.h"

namespace swordfish::motion {

	void NotHomedException::writeType(io::Writer& writer) const {
		writer << "NotHomedException";
	}

	void NotHomedException::writeMessage(io::Writer& writer) const {
		writer << "Not Homed";
	}
}