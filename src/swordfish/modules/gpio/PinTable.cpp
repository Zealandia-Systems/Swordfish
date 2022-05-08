/*
 * Pins.cpp
 *
 * Created: 30/09/2021 9:14:57 am
 *  Author: smohekey
 */

#include "PinTable.h"
#include "Pin.h"

namespace swordfish::gpio {
	PinTable::PinTable(Object* parent) :
			data::Table<Pin, PinTable>(parent) {
	}
} // namespace swordfish::gpio