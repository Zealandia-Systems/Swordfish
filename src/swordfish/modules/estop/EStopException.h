/*
 * EmergencyStopException.h
 *
 * Created: 16/09/2021 8:15:58 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/Exception.h>

namespace swordfish::estop {
	class EStopException : public Exception {
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;
		
	public:
		virtual ~EStopException() { }
	};
}