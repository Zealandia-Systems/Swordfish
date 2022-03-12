/*
 * String.h
 *
 * Created: 26/08/2021 4:52:09 pm
 *  Author: smohekey
 */ 

#pragma once

#include <cstring>
#include <string_view>

#include <swordfish/debug.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>
#include <swordfish/utils/TypeInfo.h>

namespace swordfish::core {
	class String : public Object {
	protected:
		static Schema __schema;
		
		Pack _pack;
		
		virtual Pack& getPack() override {
			return _pack;
		}
		
	public:
		String(Object* parent) : Object(parent), _pack(__schema, *this) {
			
		}
		
		virtual String* asString() override { return this; }
			
		std::string_view value() const {
			debug()("string size: ", (uint32_t)_pack._values.size());
			
			return { (const char*)_pack._values.data(), _pack._values.size() };
		}
		
		void value(const char* string) {
			size_t length = string ? strlen(string) : 0;
			
			value(std::string_view { string, length });
		}
		
		void value(const std::string_view value) {
			debug()("string: ", value);
			
			_pack._values.resize(value.length());
			
			for(auto i = 0ul; i < value.length(); i++) {
				_pack._values[i] = value[i];
			}
		}
		
		uint32_t length() const {
			return _pack._values.size();
		}
		
		virtual void writeJson(io::Writer& out) override;
	};
}