/*
 * ListPack.h
 *
 * Created: 7/09/2021 11:45:26 am
 *  Author: smohekey
 */ 

#pragma once

#include <functional>

#include "Pack.h"

#include <swordfish/utils/List.h>

namespace swordfish::core {
	template<typename T>
	class ListPack : public Pack {
	private:
		std::function<T*(Object*)> _creator;
			
	public:
		ListPack(const Schema& schema, Object& object, std::function<T*(Object*)> creator, Pack* parent = nullptr) : Pack(schema, object, parent), _creator(creator) {
			
		}
		
		virtual ~ListPack() {
			
		}
		
		virtual void readChildren(io::InputStream& stream, uint16_t childCount) override {
			auto* child = _children.first();
			
			for(auto i = 0u; i < childCount; i++) {
				if(!child || static_cast<utils::Node*>(child) == &_children) {
					child = _creator(&_object);
					
					_children.append(child);
				}
				
				child->read(stream);
				
				child = static_cast<Object*>(child->_next);
			}
		}
	};
}