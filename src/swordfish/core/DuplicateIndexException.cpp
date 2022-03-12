/*
 * DuplicateIndexException.cpp
 *
 * Created: 22/11/2021 2:45:45 pm
 *  Author: smohekey
 */ 

#include <swordfish/io/Writer.h>

#include "DuplicateIndexException.h"

namespace swordfish::core {
	void DuplicateIndexException::writeType(io::Writer& writer) const {
		writer << "DuplicateIndexException";
	}

	void DuplicateIndexException::writeMessage(io::Writer& writer) const {
		writer << "A record with an index of " << _index + 1 << " already exists.";
	}
}