/*
 * Boundary.h
 *
 * Created: 16/09/2021 1:09:35 pm
 *  Author: smohekey
 */ 

#pragma once

#include "Object.h"
#include "Pack.h"
#include "Schema.h"

namespace swordfish::core {
	template<typename TVector>
	class Boundary : public Object {
	private:
		static ObjectField<TVector> __minField;
		static ObjectField<TVector> __maxField;
		
	protected:
		static Schema __schema;
		
		Pack _pack;
		
		virtual Pack& getPack() override {
			return _pack;
		}
	
	public:
		Boundary(Object* parent) : Object(parent), _pack(__schema, *this) {
			
		}
		
		TVector& getMin() {
			return __minField.get(_pack);
		}
		
		TVector& getMax() {
			return __maxField.get(_pack);
		}
	};
	
	template<typename TVector>
	ObjectField<TVector> Boundary<TVector>::__minField = { "min", 0 };
		
	template<typename TVector>
	ObjectField<TVector> Boundary<TVector>::__maxField = { "max", 1 };
		
	template<typename TVector>
	Schema Boundary<TVector>::__schema = {
		utils::typeName<Boundary>(),
		nullptr, {
			
		}, {
			__minField,
			__maxField
		}
	};
}