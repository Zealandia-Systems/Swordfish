/*
 * LimitException.h
 *
 * Created: 30/09/2021 11:01:54 am
 *  Author: smohekey
 */

#pragma once


#include <swordfish/math.h>
#include <swordfish/Exception.h>

namespace swordfish::motion {
	class LimitException : public Exception {
	private:
		const swordfish::math::Vector3f32 _target;
		const swordfish::math::Vector3f32 _min;
		const swordfish::math::Vector3f32 _max;

	protected:
		void virtual writeType([[maybe_unused]] io::Writer& writer) const override { }
		void virtual writeMessage([[maybe_unused]] io::Writer& writer) const override { }

	public:
		LimitException(const swordfish::math::Vector3f32& target, const swordfish::math::Vector3f32& min, const swordfish::math::Vector3f32& max) : _target(target), _min(min), _max(max) {

		}

		virtual ~LimitException() { }

		const swordfish::math::Vector3f32& getTarget() {
			return _target;
		}

		virtual void writeJson(io::Writer& writer) const override;
	};
}
