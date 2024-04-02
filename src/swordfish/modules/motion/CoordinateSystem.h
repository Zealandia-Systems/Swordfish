/*
 * WorkSystem.h
 *
 * Created: 20/08/2021 5:29:53 pm
 *  Author: smohekey
 */

#pragma once

#include <swordfish/core/Object.h>
#include <swordfish/core/Pack.h>
#include <swordfish/core/Vector3.h>

#include <swordfish/data/Record.h>

namespace swordfish::motion {
	class MotionModule;

	class CoordinateSystem : public data::Record {
	private:
		static core::ValueField<int16_t> __indexField;

		static core::ObjectField<core::LinearVector3> __offsetField;
		static core::ObjectField<core::Vector3> __rotationField;

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		core::Pack& getPack() override {
			return _pack;
		}

	public:
		CoordinateSystem(Object* parent);

		virtual int16_t getIndex() override {
			return __indexField.get(_pack);
		}

		virtual void setIndex(int16_t value) override {
			__indexField.set(_pack, value);
		}

		core::LinearVector3& getOffset() {
			return __offsetField.get(_pack);
		}

		// void offset(math::Vector3f value);

		core::Vector3& getRotation() {
			return __rotationField.get(_pack);
		}
	};
} // namespace swordfish::motion
