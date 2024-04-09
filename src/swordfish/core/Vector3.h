/*
 * Offset.h
 *
 * Created: 8/08/2021 5:03:56 pm
 *  Author: smohekey
 */

#pragma once

#include <Eigen/Core>

#include <swordfish/math.h>
#include <swordfish/types.h>
#include <swordfish/debug.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Schema.h>

namespace swordfish::core {
	template<bool const IS_LINEAR>
	class Vector3Base : public Object {
	private:
		using Field = typename std::conditional<IS_LINEAR, LinearValueField<float32_t>, ValueField<float32_t>>::type;

		static Field __xField;
		static Field __yField;
		static Field __zField;

	protected:
		static Schema __schema;

		Pack _pack;

		virtual Pack& getPack() override {
			return _pack;
		}

	public:
		Vector3Base(Object* parent) :
				Object(parent), _pack(__schema, *this) {
		}

		inline f32 x() {
			return __xField.get(_pack);
		}

		inline void x(f32 value) {
			__xField.set(_pack, value);
		}

		inline f32 y() {
			return __yField.get(_pack);
		}

		inline void y(f32 value) {
			__yField.set(_pack, value);
		}

		inline f32 z() {
			return __zField.get(_pack);
		}

		inline void z(f32 value) {
			__zField.set(_pack, value);
		}

		void set(swordfish::math::Vector3f32& value) {
			x(value(Axis::X()));
			y(value(Axis::Y()));
			z(value(Axis::Z()));
		}

		inline operator swordfish::math::Vector3f32() {
			return { x(), y(), z() };
		}
	};

	using Vector3 = Vector3Base<false>;
	using LinearVector3 = Vector3Base<true>;
} // namespace swordfish::core
