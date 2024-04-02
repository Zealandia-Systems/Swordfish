/*
 * Pack.cpp
 *
 * Created: 30/08/2021 8:16:32 pm
 *  Author: smohekey
 */

#undef DEBUG
#include <swordfish/debug.h>
#include <swordfish/utils/TypeInfo.h>

#include "FormatException.h"
#include "InvalidOperationException.h"
#include "Pack.h"
#include "Object.h"
#include "Schema.h"

namespace swordfish::core {
	class UnknownObject : public Object {
	private:
		inline static Schema __schema = {
			utils::typeName<UnknownObject>(),
			nullptr,
			{

			},
			{

			}
		};

		Pack _pack;

	protected:
		virtual Pack& getPack() override {
			return _pack;
		}

	public:
		UnknownObject(Object* parent) :
				Object(parent), _pack(__schema, *this) {
		}
	};

	Pack::Pack(const Schema& schema, Object& object, Pack* parent) :
			_schema(schema), _object(object), _parent(parent) {
		for (auto fieldWrapper : _schema.valueFields()) {
			auto& field = fieldWrapper.get();

			field.init(*this);
		}

		for (auto fieldWrapper : _schema.objectFields()) {
			auto* child = fieldWrapper.get().create(&_object);

			_children.append(child);
		}
	}

	Pack::~Pack() {
		for (auto& child : _children) {
			delete &child;
		}
	}

	uint32_t Pack::length() {
		uint32_t length = _values.size();
		uint16_t childCount = _children.length();

		if (_parent) {
			childCount++;
		}

		debug()("measuring object of type: ", _schema.name());
		debug()("length: ", length);
		debug()("childCount: ", childCount);

		if (_parent) {
			length += _parent->length();
		}

		for (auto& child : _children) {
			length += child.getPack().length();
		}

		return length;
	}

	void Pack::read(io::InputStream& stream) {
		debug()("reading object of type: ", _schema.name());

		uint16_t valuesLength;
		uint16_t childCount;

		stream.read(&valuesLength, sizeof(valuesLength));
		stream.read(&childCount, sizeof(childCount));

		debug()("valuesLength: ", valuesLength);
		debug()("childCount: ", childCount);

		if (valuesLength > 1024) {
			throw FormatException { "Values length exceeds 1024." };
		}

		if (childCount > 1024) {
			throw FormatException { "Child count exeeds 1024." };
		}

		if (_values.size() < valuesLength) {
			_values.resize(valuesLength);
		}

		if (valuesLength > 0) {
			stream.read(_values.data(), valuesLength);
		}

		if (_parent) {
			childCount--;

			_parent->read(stream);
		}

		readChildren(stream, childCount);

		debug()("finished reading object of type: ", _schema.name());
	}

	void Pack::readChildren(io::InputStream& stream, uint16_t childCount) {
		auto objectFields = _schema.objectFields();
		auto fieldIt = objectFields.begin();
		auto childIt = _children.begin();

		for (auto i = 0u; i < childCount; i++, childIt++) {
			if (childIt == _children.end()) {

				auto child = fieldIt == objectFields.end() ? new UnknownObject(&_object) : (*fieldIt).get().create(&_object);

				child->read(stream);

				_children.append(child);
			} else {
				auto& child = *childIt;

				child.read(stream);
			}

			if (fieldIt != objectFields.end()) {
				fieldIt++;
			}
		}
	}

	void Pack::write(io::OutputStream& stream) {
		debug()("writing object of type: ", _schema.name());

		uint16_t valuesLength = _values.size();
		uint16_t childCount = _children.length();

		debug()("valuesLength: ", valuesLength);
		debug()("childCount: ", childCount);

		if (_parent) {
			childCount++;
		}

		stream.write(&valuesLength, sizeof(valuesLength));
		stream.write(&childCount, sizeof(childCount));

		if (valuesLength > 0) {
			stream.write(_values.data(), _values.size());
		}

		if (_parent) {
			_parent->write(stream);
		}

		for (auto& child : _children) {
			child.write(stream);
		}

		debug()("finished writing object of type: ", _schema.name());
	}

	Object& Pack::getChild(uint16_t index) {
		auto childIt = _children.begin();

		for (auto i = 0u; i <= index; i++, childIt++) {
			if (childIt == _children.end()) {
				auto child = new UnknownObject(&_object);

				_children.append(child);

				if (i == index) {
					return *child;
				}
			} else if (i == index) {
				return *childIt;
			}
		}

		throw InvalidOperationException { "Child not found." };
	}
} // namespace swordfish::core
