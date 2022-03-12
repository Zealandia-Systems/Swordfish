/*
 * Pocket.h
 *
 * Created: 8/08/2021 5:06:09 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>

#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Vector3.h>
#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>

#include <swordfish/data/Record.h>

namespace swordfish::tools {
	class Pocket : public data::Record {		
	private:
		static void validateIndex(int16_t oldValue, int16_t newValue);
	
		static core::ValidatedValueField<int16_t> __indexField;
		static core::ValueField<int16_t> __toolIndexField;
		static core::ValueField<bool> __enabledField;
		static core::ValueField<bool> __readOnlyField;
		static core::ValueField<float32_t> __depthField;
			
		static core::ObjectField<core::Vector3> __offsetField;
	
	protected:
		static core::Schema __schema;
		
		core::Pack _pack;
		
		virtual core::Pack& getPack() override {
			return _pack;
		}
		
	public:
		Pocket(core::Object* parent) : data::Record(parent), _pack(__schema, *this) {
				
		}
		
		bool isEnabled() {
			return __enabledField.get(_pack);
		}
		
		void setEnabled(bool value) {
			__enabledField.set(_pack, value);
		}
		
		bool isReadOnly() {
			return __readOnlyField.get(_pack);
		}
		
		void setReadOnly(bool value) {
			__readOnlyField.set(_pack, value);
		}
		
		float32_t getDepth() {
			return __depthField.get(_pack);
		}
		
		void setDepth(float32_t value) {
			__depthField.set(_pack, value);
		}
		
		virtual int16_t getIndex() override {
			return __indexField.get(_pack);
		}
		
		virtual void setIndex(int16_t value) override {
			__indexField.set(_pack, value);
		}
		
		int16_t getToolIndex() {
			return __toolIndexField.get(_pack);
		}
			
		void setToolIndex(int16_t value) {
			__toolIndexField.set(_pack, value);
		}
			
		core::Vector3& getOffset() {
			return __offsetField.get(_pack);
		}
	};
}