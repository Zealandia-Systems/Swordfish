/*
 * DriverParameter.h
 *
 * Created: 18/10/2021 11:37:26 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/core/Object.h>
#include <swordfish/core/Schema.h>
#include <swordfish/core/String.h>

#include <swordfish/data/Record.h>

namespace swordfish::tools {
	class DriverParameter : public data::Record {
	private:
		static core::ValueField<int16_t> __indexField;
		static core::ValueField<int16_t> __driverIndexField;
		static core::ValueField<uint16_t> __idField;
			
		static core::ObjectField<core::String> __valueField;
	
	protected:
		static core::Schema __schema;
			
		core::Pack _pack;
			
		virtual core::Pack& getPack() override {
			return _pack;
		}
	
	public:
		DriverParameter(core::Object* parent) : data::Record(parent), _pack(__schema, *this) {
			
		}
		
		virtual int16_t getIndex() override {
			return __indexField.get(_pack);
		}
		
		virtual void setIndex(int16_t index) override {
			__indexField.set(_pack, index);
		}
		
		int16_t getDriverIndex() {
			return __driverIndexField.get(_pack);
		}
		
		void setDriverIndex(int16_t driverIndex) {
			__driverIndexField.set(_pack, driverIndex);
		}
		
		uint16_t getId() {
			return __idField.get(_pack);
		}
		
		void setId(uint16_t id) {
			__idField.set(_pack, id);
		}
		
		core::String& getValue() {
			return __valueField.get(_pack);
		}
		
		void setValue(const char* value) {
			getValue().value(value);
		}
		
		void setValue(std::string_view value) {
			getValue().value(value);
		}
	};
}