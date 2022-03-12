/*
 * Console.h
 *
 * Created: 18/08/2021 12:04:34 pm
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/io/ConsoleOutputStream.h>
#include <swordfish/io/Writer.h>

namespace swordfish::core {
	class Console {
	private:
		static io::ConsoleOutputStream __outStream;
		static io::Writer __out;
		
	public:
		inline static io::ConsoleOutputStream& outStream() {
			return __outStream;
		}
		
		inline static io::Writer& out() {
			return __out;
		}
	};
}