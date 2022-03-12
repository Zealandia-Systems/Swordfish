/*
 * GPIO.h
 *
 * Created: 30/09/2021 8:52:21 am
 *  Author: smohekey
 */ 


#pragma once

#include <swordfish/Module.h>

#include <swordfish/core/Schema.h>

#include <swordfish/utils/TypeInfo.h>

#include "PinTable.h"

namespace swordfish::io {
	class InputStream;
	class OutputStream;
}

namespace swordfish::gpio {
	class GPIOModule : public Module {
	private:
		static core::ObjectField<PinTable> __pinTableField;
	
		static GPIOModule* __instance;
		
		GPIOModule(core::Object* parent);
		
	protected:
		static core::Schema __schema;
	
		core::Pack _pack;

		core::Pack& getPack() override {
			return _pack;
		}
		
	public:
		virtual ~GPIOModule() { }
		
		virtual const char* name() override { return "GPIO"; }
		virtual void init() override;
		virtual void idle() override;
		
		PinTable& getPinTable() {
			return __pinTableField.get(_pack);
		}
				
		static GPIOModule& getInstance(core::Object* parent);
	};
}