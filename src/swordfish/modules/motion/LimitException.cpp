/*
 * LimitException.cpp
 *
 * Created: 30/09/2021 3:03:31 pm
 *  Author: smohekey
 */

#include <algorithm>

#include <swordfish/io/Writer.h>

#include "LimitException.h"

namespace swordfish::motion {
	void LimitException::writeJson(io::Writer& writer) const {
		writer << "{\"type\":\"LimitException\",\"message\":\"Limits exceeded\"";

		if(_target.x() != std::clamp(_target.x(), _min.x(), _max.x())) {
			writer << ",\"x\":{\"target\":" << _target.x() << ",\"min\":" << _min.x() << ",\"max\":" << _max.x() << '}';
		}

		if(_target.y() != std::clamp(_target.y(), _min.y(), _max.y())) {
			writer << ",\"y\":{\"target\":" << _target.y() << ",\"min\":" << _min.y() << ",\"max\":" << _max.y() << '}';
		}

		if(_target.z() != std::clamp(_target.z(), _min.z(), _max.z())) {
			writer << ",\"z\":{\"target\":" << _target.z() << ",\"min\":" << _min.z() << ",\"max\":" << _max.z() << '}';
		}

		writer << '}';
	}
}