/*
 * Spindle.h
 *
 * Created: 18/10/2021 11:33:03 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/core/Object.h>
#include <swordfish/core/Schema.h>

#include <swordfish/data/Record.h>

#include "IDriver.h"
#include "DriverParameterTable.h"

namespace swordfish::tools {
	class Driver : public data::Record {
	private:
		static core::ValueField<int16_t> __indexField;
		static core::ValueField<uint16_t> __typeField;
		
		IDriver* createImplementation();
		
	protected:
		static core::Schema __schema;
		
		core::Pack _pack;
		
		virtual core::Pack& getPack() override {
			return _pack;
		}
		
		IDriver* _implementation;
				
	public:
		Driver(core::Object* parent) : data::Record(parent), _pack(__schema, *this), _implementation(nullptr) {
			
		}
		
		virtual int16_t getIndex() override {
			return __indexField.get(_pack);
		}
		
		virtual void setIndex(int16_t index) override {
			__indexField.set(_pack, index);
		}
		
		uint16_t getType() {
			return __typeField.get(_pack);
		}
		
		void setType(uint16_t type) {
			auto existing = getType();
			
			if(existing != type) {
				__typeField.set(_pack, type);
				
				_implementation = nullptr;
			}
		}
		
		IDriver* getImplementation() {
			return _implementation ? _implementation : (_implementation = createImplementation());
		}
	};
}