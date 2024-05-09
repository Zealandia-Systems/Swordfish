/*
 * M2000.cpp
 *
 * Created: 5/10/2021 11:39:05 am
 *  Author: smohekey
 */

#include <charconv>
#include <iterator>

#include "../../inc/MarlinConfigPre.h"
#include "../gcode.h"

#include <swordfish/debug.h>
#include <swordfish/core/Console.h>
#include <swordfish/data/Table.h>
#include <swordfish/json/Reader.h>
#include <swordfish/Controller.h>
#include <swordfish/modules/gcode/CommandException.h>

using namespace swordfish;
using namespace swordfish::core;
using namespace swordfish::data;
using namespace swordfish::io;
using namespace swordfish::motion;
using namespace swordfish::tools;

enum class Operation {
	Create = 0,
	Read = 1,
	Update = 2,
	Delete = 3
};

static Object* select(Object* root, Object* context, std::string_view selector, bool create = false) {
	std::string_view::size_type npos = std::string_view::npos;

	if (selector[0] == '/') {
		context = root;

		selector = selector.substr(1);
	}

	while (true) {
		if (selector.size() == 0) {
			return context;
		}

		auto i = selector.find_first_of('/');

		auto name = selector.substr(0, i);

		debug()(name);

		auto* child = context->getChild(name);

		debug()(child);

		if (child && create && i == npos) {
			throw CommandException { "A record with that index already exists." };
		}

		if (!child && create) {
			auto table = context->asTable();

			if (table) {
				uint16_t index;

				auto [ptr, ec] = std::from_chars(name.data(), name.data() + name.size(), index);

				if (ec != std::errc()) {
					throw FormatException { "index in selector must be an integer." };
				}

				auto& record = table->createChild();

				record.setIndex(index);

				child = record.asRecord();
			}
		}

		if (!child) {
			return nullptr;
		}

		if (i == npos) {
			selector = selector.substr(selector.size());
		} else {
			selector = selector.substr(i + 1);
		}

		context = child;
	}

	return nullptr;
}

class JsonApplicator : public swordfish::json::Reader<Object> {
private:
	Object* _context;
	std::string_view _objectProperty;
	int32_t _tableIndex;

	Object* getChild(Object* parent) {
		if (_tableIndex >= 0) {
			auto* table = parent->asTable();

			if (!table) {
				throw CommandException { "Expected a table." };
			}

			return table->getChild(_tableIndex);
		}

		if (_objectProperty.size() > 0) {
			auto* child = parent->getChild(_objectProperty);

			if (!child) {
				throw CommandException { "Child is null." };
			}

			return child;
		}

		throw CommandException { "Expected a child." };
	}

protected:
	virtual Object* onStartObject(Object* parent) override {
		debug()();

		if (parent == nullptr) {
			return _context;
		}

		return getChild(parent);
	}

	virtual void onPropertyName(Object* parent, std::string_view name) {
		debug()(name);

		_objectProperty = name;
		_tableIndex = -1;
	}

	virtual void onEndObject(Object* parent, Object* object) override {
		debug()();
	}

	virtual Object* onStartArray(Object* parent) override {
		debug()();

		auto* child = getChild(parent);
		auto* table = child->asTable();

		if (!table) {
			throw CommandException { "Expected a table." };
		}

		return child;
	}

	virtual void onArrayIndex(Object* table, int32_t index) override {
		debug()("index: ", index);

		_tableIndex = index;
		_objectProperty = { nullptr, 0 };
	}

	virtual void onEndArray(Object* parent, Object* table) override {
		debug()();
	}

	virtual void onValue(Object* parent, std::string_view value) override {
		debug()();

		if (_tableIndex >= 0) {

		} else {
			parent->setValueFromJson(_objectProperty, value);
		}
	}

public:
	JsonApplicator(Object* context) :
			_context(context) {
	}
};

void GcodeSuite::M2000(std::function<void(std::function<void(Writer&)>)> writeResult) {
	using namespace std::literals;

	auto op = (Operation) parser.intval('O', (int16_t) Operation::Read);

	auto& controller = Controller::getInstance();

	debug()(parser.selector_string);

	auto* object = select(&controller, &controller, parser.selector_string, op == Operation::Create);

	if (!object) {
		throw CommandException { "Record not found." };
	}

	switch (op) {
		case Operation::Create:
		case Operation::Update: {
			if (!(parser.parameter_string.size() > 0)) {
				throw CommandException { "JSON value must be specified with > for Create or Update operations." };
			}

			JsonApplicator applicator { object };

			applicator.read(parser.parameter_string);

			controller.save();

			writeResult(nullptr);

			break;
		}

		case Operation::Delete: {
			auto* record = object->asRecord();

			if (!record) {
				throw CommandException { "Only records can be deleted." };
			}

			auto* parent = object->getParent();
			auto* table = parent->asTable();

			if (!table) {
				throw CommandException { "Only records can be deleted." };
			}

			table->remove(*record);

			controller.save();

			writeResult(nullptr);

			break;
		}

		case Operation::Read: {
			writeResult([&](Writer& out) {
				auto* table = object->asTable();

				if (table) {
					table->writeJson(out, { .page = -1, .pageLength = -1 });
				} else {
					object->writeJson(out);
				}
			});

			break;
		}
	}

	// in case any offset related records were changed.

	auto& motionModule = MotionModule::getInstance();

	motionModule.setActiveCoordinateSystem(motionModule.getActiveCoordinateSystem());
}