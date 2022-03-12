/*
 * RS485.h
 *
 * Created: 18/02/2021 6:26:47 pm
 *  Author: smohekey
 */

#ifndef RS485_H_
#define RS485_H_

#include "Arduino.h"
#include "HardwareSerial.h"
#include "watchdog.h"

class RS485 : public HardwareSerial {
public:
	RS485(SERCOM* _s, uint8_t _pinRX, uint8_t _pinTX, SercomRXPad _padRX, SercomUartTXPad _padTX, uint8_t _pinRTS);

	void begin(unsigned long baudRate);
	void begin(unsigned long baudrate, uint16_t config);
	void end();
	int available();
	int availableForWrite();
	int peek();
	int read();
	void flush();
	void clear();
	size_t write(const uint8_t data);
	using Print::write; // pull in write(str) and write(buf, size) from Print

	void IrqHandler();

	operator bool() {
		return true;
	}

protected:
	// protected method to read stream with timeout
	virtual int timedRead() {
		int c;
		_startMillis = millis();
		do {
			c = read();
			if (c >= 0)
				return c;
			HAL_watchdog_refresh();
		} while (millis() - _startMillis < _timeout);
		return -1; // -1 indicates timeout
	}

	// protected method to peek stream with timeout
	virtual int timedPeek() {
		int c;
		_startMillis = millis();
		do {
			c = peek();
			if (c >= 0)
				return c;
			HAL_watchdog_refresh();
		} while (millis() - _startMillis < _timeout);
		return -1; // -1 indicates timeout
	}

private:
	SERCOM* sercom;
	RingBuffer rxBuffer;
	RingBuffer txBuffer;

	uint8_t uc_pinRX;
	uint8_t uc_pinTX;
	SercomRXPad uc_padRX;
	SercomUartTXPad uc_padTX;
	uint8_t uc_pinRTS;

	SercomNumberStopBit extractNbStopBit(uint16_t config);
	SercomUartCharSize extractCharSize(uint16_t config);
	SercomParityMode extractParity(uint16_t config);
};

#endif /* RS485_H_ */
