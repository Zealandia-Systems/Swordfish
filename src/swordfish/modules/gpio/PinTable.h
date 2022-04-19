/*
 * Pins.h
 *
 * Created: 30/09/2021 9:07:35 am
 *  Author: smohekey
 */

#pragma once

#include <swordfish/data/Table.h>

#include "Pin.h"

namespace swordfish::gpio {
	class PinTable : public data::Table<Pin, PinTable> {
	public:
		PinTable(Object* parent);

		virtual const char* getName() override {
			return "pin";
		}
		// virtual void remove(Pin& pin) override;
	};
} // namespace swordfish::gpio