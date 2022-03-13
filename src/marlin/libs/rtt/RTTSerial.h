/*
 * RTTSerial.h
 *
 * Created: 26/02/2021 3:22:52 pm
 *  Author: smohekey
 */ 


#ifndef RTTSERIAL_H_
#define RTTSERIAL_H_

#include "Stream.h"

class RTTSerial : public Stream {
	public:
	  virtual void begin(unsigned long) {}
	  virtual void begin(unsigned long, uint16_t) {}
	  virtual void end() {}
				
		virtual size_t write(uint8_t c);
		virtual int available();
		virtual int read();
		virtual int peek();
		
		//int printf(const char* format, ...);
		
		operator bool() { return true; }
};


extern RTTSerial RTTSerial1;

#endif /* RTTSERIAL_H_ */