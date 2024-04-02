/*
 * EStopException.cpp
 *
 * Created: 30/09/2021 2:47:36 pm
 *  Author: smohekey
 */

#include <swordfish/io/Writer.h>

#include "EStopException.h"

namespace swordfish::estop {

	void EStopException::writeType(io::Writer& writer) const {
		writer << "EStopException";
	}

	void EStopException::writeMessage(io::Writer& writer) const {
		writer << "Emergency Stop";
	}
}