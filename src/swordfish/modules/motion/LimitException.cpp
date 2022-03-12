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
			
		if(_target(X) != std::clamp(_target(X), _min(X), _max(X))) {
			writer << ",\"x\":{\"target\":" << _target(X) << ",\"min\":" << _min(X) << ",\"max\":" << _max(X) << '}';
		}
		
		if(_target(Y) != std::clamp(_target(Y), _min(Y), _max(Y))) {
			writer << ",\"y\":{\"target\":" << _target(Y) << ",\"min\":" << _min(Y) << ",\"max\":" << _max(Y) << '}';
		}
		
		if(_target(Z) != std::clamp(_target(Z), _min(Z), _max(Z))) {
			writer << ",\"z\":{\"target\":" << _target(Z) << ",\"min\":" << _min(Z) << ",\"max\":" << _max(Z) << '}';
		}
		
		writer << '}';
	}
}