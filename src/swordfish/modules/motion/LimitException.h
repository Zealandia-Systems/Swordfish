/*
 * LimitException.h
 *
 * Created: 30/09/2021 11:01:54 am
 *  Author: smohekey
 */

#pragma once

#include <Eigen/Core>

#include <swordfish/Exception.h>

namespace swordfish::motion {
	class LimitException : public Exception {
	private:
		const Eigen::Vector3f _target;
		const Eigen::Vector3f _min;
		const Eigen::Vector3f _max;

	protected:
		void virtual writeType([[maybe_unused]] io::Writer& writer) const override { }
		void virtual writeMessage([[maybe_unused]] io::Writer& writer) const override { }

	public:
		LimitException(const Eigen::Vector3f& target, const Eigen::Vector3f& min, const Eigen::Vector3f& max) : _target(target), _min(min), _max(max) {

		}

		virtual ~LimitException() { }

		const Eigen::Vector3f& getTarget() {
			return _target;
		}

		virtual void writeJson(io::Writer& writer) const override;
	};
}
