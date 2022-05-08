/*
 * Driver.cpp
 *
 * Created: 19/10/2021 10:30:13 am
 *  Author: smohekey
 */

#include "Driver.h"

#include "ToolsModule.h"

#include "drivers/ChangzouH100DriverImpl.h"
#include "drivers/CT100DriverImpl.h"
#include "drivers/FulingDZBDriverImpl.h"
#include "drivers/PWMLaserDriverImpl.h"

namespace swordfish::tools {
	core::ValueField<int16_t> Driver::__indexField = { "index", 0, 0 };
	core::ValueField<uint16_t> Driver::__typeField = { "type", 2, 0 };

	IDriver* createImplementation();

	core::Schema Driver::__schema = {
		utils::typeName<Driver>(),
		nullptr,
		{__indexField,
		  __typeField },
		{

      }
	};

	swordfish::tools::IDriver* Driver::createImplementation() {
		IDriver* implementation = nullptr;

		switch (getType()) {
			case 0: {
				implementation = new drivers::ChangzouH100DriverImpl();

				break;
			}

			case 1: {
				implementation = new drivers::FulingDZBDriverImpl();

				break;
			}

			case 2: {
				implementation = new drivers::PWMLaserDriverImpl();

				break;
			}

			case 3: {
				implementation = new drivers::CT100DriverImpl();

				break;
			}
		}

		if (implementation) {
			implementation->init(getIndex(), ToolsModule::getInstance().getDriverParameters());
		}

		return implementation;
	}
} // namespace swordfish::tools