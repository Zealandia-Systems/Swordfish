/*
* Object.h
*
* Created: 8/08/2021 4:51:03 pm
*  Author: smohekey
*/

#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <string_view>
#include <type_traits>
#include <functional>

#include <swordfish/types.h>
#include <swordfish/utils/NotCopyable.h>
#include <swordfish/utils/NotMovable.h>

#include <swordfish/io/InputStream.h>
#include <swordfish/io/OutputStream.h>
#include <swordfish/io/Writer.h>

#include <swordfish/utils/List.h>

namespace swordfish::core {
	class Schema;
	class Object;
	class String;
	
	class Pack : public utils::NotCopyable, public utils::NotMovable {
		friend class Object;
		
		friend class String;
		
		template<typename T>
		friend class ObjectList;
		
		template<typename T, typename... Ts>
		friend class TypedObjectList;
	
		template<typename T>
		friend class Table;
		
		template<typename T>
		friend class ValueField;
		
		friend class ObjectFieldBase;
		
		template<typename T>
		friend class ObjectField;

	protected:
		const Schema& _schema;
		Object& _object;
		Pack* _parent;
		std::vector<uint8_t> _values;
		utils::List<Object> _children;
		
		virtual void readChildren(io::InputStream& stream, uint16_t childCount);
		
	public:
		Pack(const Schema& schema, Object& object, Pack* parent = nullptr);
		virtual ~Pack();
		
		Pack* getParent() {
			return _parent;
		}
		
		Object& getObject() {
			return _object;
		}
		
		const Schema& schema() {
			return _schema;
		}
		
		Object& getChild(uint16_t index);
		
		virtual uint32_t length();
		virtual void read(io::InputStream& stream);
		
		virtual void write(io::OutputStream& stream);
	};

}