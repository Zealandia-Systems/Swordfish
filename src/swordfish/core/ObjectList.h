/*
 * ObjectArray.h
 *
 * Created: 17/08/2021 6:19:38 pm
 *  Author: smohekey
 */ 

#pragma once

#include <type_traits>

#include <swordfish/utils/TypeInfo.h>

#include "Object.h"
#include "Schema.h"
#include "ListPack.h"

namespace swordfish::core {
	template<typename T>
	class ObjectList : public Object {
	protected:
		static Schema __schema;
			
		ListPack<T> _pack;
		
		virtual Pack& getPack() override {
			return _pack;
		}
		
		static T* create(Object* parent) {
			return _::create<T>(parent);
		}
		
	public:
		ObjectList(Object* parent) : Object(parent), _pack(__schema, *this, create) {
			
		}
		
		uint32_t length() const {
			return _pack._children.length();
		}
				
		virtual T& emplaceBack() {
			auto* child = create(this);
			
			_pack._children.append(child);
			
			return *child;
		}
		
		virtual void popBack() {
			auto* child = _pack._children.last();
			
			if(child) {
				_pack._children.remove(child);
				
				delete child;
			}
		}
		
		virtual void remove(T& child) {
			_pack._children.remove(&child);
			
			delete &child;
		}
		
		virtual void writeJson(io::Writer& out) override {
			out << '[';
					
			auto separator = "";
					
			for(auto& child : _pack._children) {
				out << separator;
						
				child.writeJson(out);
						
				separator = ",";
			}
					
			out << ']';
		}
	
		utils::ListIterator<T> begin() { return utils::ListIterator<T>(_pack._children._next); }
		utils::ListIterator<T> end() { return utils::ListIterator<T>(&_pack._children); }
	};
	
	template<typename T>
	Schema ObjectList<T>::__schema = {
		utils::typeName<ObjectList<T>>(),
		nullptr, {

		}, {
			
		}
	};
}