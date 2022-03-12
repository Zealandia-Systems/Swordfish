/*
 * Limits.h
 *
 * Created: 16/09/2021 2:06:39 pm
 *  Author: smohekey
 */

#pragma once

#include <Eigen/Core>

#include <swordfish/core/Pack.h>
#include <swordfish/core/Boundary.h>
#include <swordfish/utils/TypeInfo.h>

namespace swordfish::motion {
	class Limits : public core::Boundary<core::Vector3> {
	private:
		bool _enabled;

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		virtual core::Pack& getPack() override {
			return _pack;
		}

	public:
		Limits(core::Object* parent) : core::Boundary<core::Vector3>(parent), _enabled(true), _pack(__schema, *this, &(core::Boundary<core::Vector3>::_pack)) {

		}

		bool areEnabled() const {
			return _enabled;
		}

		void setEnabled(bool enabled) {
			_enabled = enabled;
		}

		void clamp(Eigen::Vector3f& vector);
		void throwIfOutside(const Eigen::Vector3f& vector);
	};
}
