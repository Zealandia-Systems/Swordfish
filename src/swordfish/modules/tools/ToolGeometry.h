/*
 * ToolGeometry.h
 *
 * Created: 8/09/2021 10:39:01 am
 *  Author: smohekey
 */

#pragma once

#include <swordfish/types.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>

namespace swordfish::tools {
	class ToolGeometry : public core::Object {
	private:
		static core::LinearValueField<float32_t> __diameterField;
		static core::LinearValueField<float32_t> __lengthField;

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		virtual core::Pack& getPack() override {
			return _pack;
		}

	public:
		ToolGeometry(core::Object* parent) :
				core::Object(parent), _pack(__schema, *this) {
		}

		inline float32_t diameter() {
			return __diameterField.get(_pack);
		}

		inline void diameter(float32_t value) {
			__diameterField.set(_pack, value);
		}

		inline float32_t length() {
			return __lengthField.get(_pack);
		}

		inline void length(float32_t value) {
			__lengthField.set(_pack, value);
		}
	};
} // namespace swordfish::tools