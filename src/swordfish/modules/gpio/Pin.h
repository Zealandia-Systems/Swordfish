/*
 * Pin.h
 *
 * Created: 30/09/2021 7:55:03 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/core/Object.h>
#include <swordfish/core/Schema.h>
#include <swordfish/data/Table.h>
#include <swordfish/utils/TypeInfo.h>

namespace swordfish::gpio {
	class Pin : public data::Record {
	private:
		static core::ValueField<int8_t> __portField;
		static core::ValueField<int8_t> __pinField;
		static core::ValueField<bool> __inputField;
		static core::ValueField<bool> __pullUpField;
		static core::ValueField<bool> __pullDownField;
		static core::ValueField<bool> __totemPoleField;
			
	protected:
		static core::Schema __schema;
		
		core::Pack _pack;
		
		core::Pack& getPack() override {
			return _pack;
		}
		
	public:
		Pin(core::Object* parent) : data::Record(parent), _pack(__schema, *this) {
			
		}
		
		virtual ~Pin() { }
		
		int8_t getPort() {
			return __portField.get(_pack);
		}
		
		void setPort(int8_t port) {
			__portField.set(_pack, port);
		}

		int8_t getPin() {
			return __pinField.get(_pack);
		}
		
		void setPin(int8_t pin) {
			__pinField.set(_pack, pin);
		}
		
		virtual int16_t getIndex() override {
			return getPort() * 32 + getPin();		
		}
		
		virtual void setIndex(int16_t index) override {
			setPin(index & 0x1F);
			setPort(index >> 5);
		}
	};
}