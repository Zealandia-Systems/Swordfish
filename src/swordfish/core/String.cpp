/*
 * String.cpp
 *
 * Created: 27/08/2021 4:21:35 pm
 *  Author: smohekey
 */ 

#include "String.h"

namespace swordfish::core {
	Schema String::__schema = {
		utils::typeName<String>(),
		nullptr, {

		}, {
					
		}
	};
			
	void String::writeJson(io::Writer& out) {
		out << '"';
		
		for(const char c : value()) {
			if(c == '"') {
				out << '\\';
			}
			
			out << c;
		}
		
		out << '"';
	}
}