/*
 * SerialOutputStream.h
 *
 * Created: 15/08/2021 9:06:40 pm
 *  Author: smohekey
 */ 

#pragma once

#include "OutputStream.h"

namespace swordfish::io {
	class ConsoleOutputStream : public OutputStream {
	public:
		size_t write(const void* buffer, size_t length) override;
	};
}