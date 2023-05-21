/*
 * Tool.h
 *
 * Created: 8/08/2021 5:07:25 pm
 *  Author: smohekey
 */

#pragma once

#include <string_view>

#include <swordfish/types.h>

#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/String.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Vector3.h>

#include <swordfish/data/Record.h>

#include "Driver.h"
#include "ToolGeometry.h"

namespace swordfish::tools {
	class Tool : public data::Record {
	private:
		static void validateIndex(int16_t oldValue, int16_t newValue);
		static int16_t getPocketIndex(Tool& tool);
		static void setPocketIndex(Tool& tool, int16_t pocketIndex);

		static core::ValidatedValueField<int16_t> __indexField;
		static core::ValueField<bool> __fixedField;
		static core::ValueField<bool> __needsProbeField;
		static core::ValueField<uint8_t> __driverIndexField;
		static core::ObjectField<ToolGeometry> __geometryField;
		static core::ObjectField<core::String> __descriptionField;
		static core::ObjectField<core::LinearVector3> __offsetField;
		static core::TransientField<Tool, int16_t> __pocketField;

	protected:
		core::Pack _pack;

		static core::Schema __schema;

		virtual core::Pack& getPack() override {
			return _pack;
		}

	public:
		Tool(core::Object* parent) :
				data::Record(parent), _pack(__schema, *this) {
		}

		virtual int16_t getIndex() override {
			return __indexField.get(_pack);
		}

		virtual void setIndex(int16_t value) override {
			__indexField.set(_pack, value);
		}

		core::String& getDescription() {
			return __descriptionField.get(_pack);
		}

		void setDescription(const char* value) {
			getDescription().value(value);
		}

		void setDescription(std::string_view value) {
			getDescription().value(value);
		}

		bool isFixed() {
			return __fixedField.get(_pack);
		}

		void setFixed(bool fixed) {
			__fixedField.set(_pack, fixed);
		}

		bool getNeedsProbe() {
			return __needsProbeField.get(_pack);
		}

		void setNeedsProbe(bool value) {
			__needsProbeField.set(_pack, value);
		}

		uint8_t getDriverIndex() {
			return __driverIndexField.get(_pack);
		}

		void setDriverIndex(uint8_t driver) {
			__driverIndexField.set(_pack, driver);
		}

		ToolGeometry& getGeometry() {
			return __geometryField.get(_pack);
		}

		core::LinearVector3& getOffset() {
			return __offsetField.get(_pack);
		}
	};
} // namespace swordfish::tools