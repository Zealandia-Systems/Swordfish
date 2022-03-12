/*
 * Module.h
 *
 * Created: 10/08/2021 2:48:12 pm
 *  Author: smohekey
 */

#pragma once

#include <swordfish/utils/NotCopyable.h>
#include <swordfish/utils/NotMovable.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>

namespace swordfish {
	class Module : public core::Object {
	private:
		static core::ValueField<bool> __enabledField;

	protected:
		static core::Schema __schema;
		
		core::Pack _pack;
	
		Module(core::Object* parent) : core::Object(parent), _pack(__schema, *this) {
			
		}
		
		virtual core::Pack& getPack() override;
		
	public:
		virtual ~Module() {
		
		}
	
		bool enabled() {
			return __enabledField.get(_pack);
		}
		
		void enabled(bool enabled) {
			__enabledField.set(_pack, enabled);
		}
		
		virtual const char* name() = 0;
		
		virtual void init() = 0;
		virtual void idle() { }
	};
}