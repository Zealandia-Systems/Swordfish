/*
 * Console.cpp
 *
 * Created: 18/08/2021 12:05:49 pm
 *  Author: smohekey
 */ 

#include "Console.h"

namespace swordfish::core {
	io::ConsoleOutputStream Console::__outStream;
	
	io::Writer Console::__out = { __outStream };
}