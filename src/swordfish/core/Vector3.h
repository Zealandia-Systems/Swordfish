/*
 * Offset.h
 *
 * Created: 8/08/2021 5:03:56 pm
 *  Author: smohekey
 */

#pragma once

#include <Eigen/Core>

#include <swordfish/types.h>
#include <swordfish/debug.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>

namespace swordfish::core {
	class Vector3 : public Object {
	private:
		static ValueField<float32_t> __xField;
		static ValueField<float32_t> __yField;
		static ValueField<float32_t> __zField;

	protected:
		static Schema __schema;

		Pack _pack;

		virtual Pack& getPack() override {
			return _pack;
		}

	public:
		Vector3(Object* parent) : Object(parent), _pack(__schema, *this) {

		}

		inline float32_t x() {
			return __xField.get(_pack);
		}

		inline void x(float32_t value) {
			__xField.set(_pack, value);
		}

		inline float32_t y() {
			return __yField.get(_pack);
		}

		inline void y(float32_t value) {
			__yField.set(_pack, value);
		}

		inline float32_t z() {
			return __zField.get(_pack);
		}

		inline void z(float32_t value) {
			__zField.set(_pack, value);
		}

		void set(Eigen::Vector3f& value) {
			x(value(X));
			y(value(Y));
			z(value(Z));
		}

		inline operator Eigen::Vector3f() {
			return { x(), y(), z() };
		}
	};
}
