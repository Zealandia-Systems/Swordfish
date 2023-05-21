/*
 * Object.cpp
 *
 * Created: 17/08/2021 1:57:30 pm
 *  Author: smohekey
 */

#include <cstring>

#include <swordfish/debug.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include "Console.h"
#include "Object.h"
#include "Pack.h"
#include "Schema.h"
#include "String.h"

namespace swordfish::core {
	void Object::read(io::InputStream& stream) {
		auto& pack = getPack();

		pack.read(stream);
	}

	void Object::write(io::OutputStream& stream) {
		auto& pack = getPack();

		pack.write(stream);
	}

	void Object::writeJsonProperty(io::Writer& out, Pack& pack, Field& field, const char*& separator) {
		out << separator;

		separator = ",";

		out << '"' << field.name() << "\":";

		field.writeJson(out, *this, pack);
	}

	void Object::writeJson(io::Writer& out) {
		out << '{';

		auto separator = "";

		auto* pack = &getPack();
		const auto* schema = &pack->schema();

		while (schema) {
			for (auto& field : schema->valueFields()) {
				writeJsonProperty(out, *pack, field.get(), separator);
			}

			for (auto& field : schema->transientFields()) {
				writeJsonProperty(out, *pack, field.get(), separator);
			}

			for (auto& field : schema->objectFields()) {
				writeJsonProperty(out, *pack, field.get(), separator);
			}

			schema = schema->parent();
			pack = pack->getParent();
		}

		out << '}';
	}

	Object* Object::getChild(std::string_view name) {
		auto* pack = &getPack();
		const auto* schema = &pack->schema();

		while (schema) {
			for (auto& field : schema->objectFields()) {
				if (strncmp(field.get().name(), name.data(), name.size()) == 0) {

					return &(field.get().get(*pack));
				}
			}

			schema = schema->parent();
			pack = pack->getParent();
		}

		return nullptr;
	}

	Object* Object::select(std::string_view selector) {
		debug()(selector);

		auto i = selector.find_first_of("/[");
		auto name = selector.substr(0, i);

		auto* child = getChild(name);

		if (child) {
			if (i == std::string_view::npos) {
				return child;
			} else {
				return child->select(selector.substr(i + 1));
			}
		}

		return nullptr;
	}

	void Object::setValueFromJson(std::string_view name, std::string_view value) {
		debug()("setting ", name, " to ", value);

		auto* pack = &getPack();
		const auto* schema = &pack->schema();

		while (schema) {
			for (auto& field : schema->valueFields()) {
				if (strncmp(field.get().name(), name.data(), name.size()) == 0) {
					field.get().readJson(*pack, value);

					return;
				}
			}

			// attempt to find a string object

			for (auto& field : schema->objectFields()) {
				if (strncmp(field.get().name(), name.data(), name.size()) == 0) {
					auto& child = field.get().get(*pack);

					auto* string = child.asString();

					if (string) {
						string->value(value);

						return;
					}
				}
			}

			// find a transient/synthetic field

			for (auto& field : schema->transientFields()) {
				if (strncmp(field.get().name(), name.data(), name.size()) == 0) {
					field.get().set(*pack, value);

					return;
				}
			}

			schema = schema->parent();
			pack = pack->getParent();
		}
	}
} // namespace swordfish::core