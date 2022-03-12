/*
 * Record.h
 *
 * Created: 10/11/2021 11:51:10 am
 *  Author: smohekey
 */ 

#pragma once

#include <swordfish/core/Object.h>

namespace swordfish::data  {
	class Record : public core::Object {
	protected:
		Record(core::Object* parent) : core::Object(parent) { }
			
	public:
		virtual int16_t getIndex() = 0;
		virtual void setIndex(int16_t index) = 0;
		
		virtual Record* asRecord() override { return this; }
	};
}