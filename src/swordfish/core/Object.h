/*
 * Object.h
 *
 * Created: 17/08/2021 9:51:19 am
 *  Author: Scott Mohekey
 */

#pragma once

#include <string_view>

#include <swordfish/io/InputStream.h>
#include <swordfish/io/OutputStream.h>
#include <swordfish/io/Writer.h>

#include <swordfish/utils/List.h>
#include <swordfish/utils/NotCopyable.h>
#include <swordfish/utils/NotMovable.h>

namespace swordfish::data {
	class ITable;
	class Record;
} // namespace swordfish::data

namespace swordfish::core {
	class Field;
	class Pack;
	class String;

	class Object : public utils::Node, public utils::NotCopyable, public utils::NotMovable {
		friend class ObjectFieldBase;

		friend class Pack;

		template<typename T>
		friend class ObjectList;

	private:
		Object* _parent;
		void writeJsonProperty(io::Writer& out, Pack& pack, Field& field, const char*& separator);

	protected:
		Object(Object* parent) :
				_parent(parent) {
		}

		virtual Pack& getPack() = 0;

	public:
		virtual ~Object() = default;

		virtual void read(io::InputStream& stream);
		virtual void write(io::OutputStream& stream);
		virtual void writeJson(io::Writer& out);

		virtual Object* getParent() {
			return _parent;
		}
		virtual Object* getChild(std::string_view name);
		virtual Object* select(std::string_view selector);

		virtual void setValueFromJson(std::string_view name, std::string_view value);

		virtual data::ITable* asTable() {
			return nullptr;
		}
		virtual data::Record* asRecord() {
			return nullptr;
		}
		virtual String* asString() {
			return nullptr;
		}
	};

	namespace _ {
		template<typename T, std::enable_if_t<std::is_constructible<T, Object*>::value, bool> = true>
		T* create(Object* parent) {
			return new T(parent);
		}

		template<typename T, std::enable_if_t<std::is_constructible<T>::value, bool> = true>
		T* create([[maybe_unused]] Object* parent) {
			return new T();
		}
	} // namespace _
} // namespace swordfish::core