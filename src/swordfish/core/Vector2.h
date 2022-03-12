/*
 * Vector2.h
 *
 * Created: 8/09/2021 10:37:29 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/types.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>

namespace swordfish::core {
	class Vector2 : public Object {
	private:
		static ValueField<float32_t> __xField;
		static ValueField<float32_t> __yField;
		
	protected:
		static Schema __schema;
		
		Pack _pack;
		
		virtual Pack& getPack() override {
			return _pack;
		}
		
	public:
		Vector2(Object* parent) : Object(parent), _pack(__schema, *this) {
			
		}
		
		inline float32_t x() {
			return __xField.get(_pack);
		}
		
		inline void x(float32_t value) {
			__xField.set(_pack, value);
		}
		
		inline float32_t y() {
			return __yField.get(_pack);
		}
		
		inline void y(float32_t value) {
			__yField.set(_pack, value);
		}
	};
}