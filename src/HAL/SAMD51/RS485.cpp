/*
 * RS485.cpp
 *
 * Created: 18/02/2021 6:31:26 pm
 *  Author: smohekey
 */

#include "RS485.h"
#include "Arduino.h"
#include "wiring_private.h"

#define NO_RTS_PIN       255
#define RTS_RX_THRESHOLD 10

RS485::RS485(SERCOM* _s, uint8_t _pinRX, uint8_t _pinTX, SercomRXPad _padRX, SercomUartTXPad _padTX, uint8_t _pinRTS) {
	_timeout = 100;
	sercom = _s;
	uc_pinRX = _pinRX;
	uc_pinTX = _pinTX;
	uc_padRX = _padRX;
	uc_padTX = _padTX;
	uc_pinRTS = _pinRTS;
}

void RS485::begin(unsigned long baudrate) {
	begin(baudrate, SERIAL_8N1);
}

void RS485::begin(unsigned long baudrate, uint16_t config) {
	pinPeripheral(uc_pinRX, g_APinDescription[uc_pinRX].ulPinType);
	pinPeripheral(uc_pinTX, g_APinDescription[uc_pinTX].ulPinType);

	PORT->Group[g_APinDescription[8].ulPort].PINCFG[g_APinDescription[8].ulPin].bit.PMUXEN = 1; // Enable the TE output on D8
	PORT->Group[g_APinDescription[8].ulPort].PMUX[g_APinDescription[8].ulPin >> 1].reg =
			PORT->Group[g_APinDescription[8].ulPort].PMUX[g_APinDescription[8].ulPin >> 1].reg |
			PORT_PMUX_PMUXE(2);

	sercom->initUART(UART_INT_CLOCK, SAMPLE_RATE_x16, baudrate);
	sercom->initFrame(extractCharSize(config), LSB_FIRST, extractParity(config), extractNbStopBit(config));
	sercom->initPads(uc_padTX, uc_padRX);

	// SERCOM5->USART.CTRLA.bit.TXPO = 0x3;          // Set TXPO to 3 to activate the TE output
	SERCOM5->USART.CTRLC.bit.GTIME = 0x4; // Set the RS485 guard time

	sercom->enableUART();
}

void RS485::end() {
	sercom->resetUART();
	rxBuffer.clear();
	txBuffer.clear();
}

void RS485::flush() {
	while (txBuffer.available())
		; // wait until TX buffer is empty

	sercom->flushUART();
}

void RS485::clear() {
	rxBuffer.clear();
	txBuffer.clear();
}

void RS485::IrqHandler() {
	if (sercom->isFrameErrorUART()) {
		// frame error, next byte is invalid so read and discard it
		sercom->readDataUART();

		sercom->clearFrameErrorUART();
	}

	if (sercom->availableDataUART()) {
		rxBuffer.store_char(sercom->readDataUART());
	}

	if (sercom->isDataRegisterEmptyUART()) {
		if (txBuffer.available()) {
			uint8_t data = txBuffer.read_char();

			sercom->writeDataUART(data);
		} else {
			sercom->disableDataRegisterEmptyInterruptUART();
		}
	}

	if (sercom->isUARTError()) {
		sercom->acknowledgeUARTError();
		// TODO: if (sercom->isBufferOverflowErrorUART()) ....
		// TODO: if (sercom->isParityErrorUART()) ....
		sercom->clearStatusUART();
	}
}

int RS485::available() {
	return rxBuffer.available();
}

int RS485::availableForWrite() {
	return txBuffer.availableForStore();
}

int RS485::peek() {
	return rxBuffer.peek();
}

int RS485::read() {
	return rxBuffer.read_char();
}

size_t RS485::write(const uint8_t data) {
	if (sercom->isDataRegisterEmptyUART() && txBuffer.available() == 0) {
		sercom->writeDataUART(data);
	} else {
		// spin lock until a spot opens up in the buffer
		while (txBuffer.isFull()) {
			uint8_t interruptsEnabled = ((__get_PRIMASK() & 0x1) == 0);

			if (interruptsEnabled) {
				uint32_t exceptionNumber = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);

				if (exceptionNumber == 0 ||
				    NVIC_GetPriority((IRQn_Type) (exceptionNumber - 16)) > SERCOM_NVIC_PRIORITY) {
					// no exception or called from an ISR with lower priority,
					// wait for free buffer spot via IRQ
					continue;
				}
			}

			// interrupts are disabled or called from ISR with higher or equal priority than the SERCOM IRQ
			// manually call the UART IRQ handler when the data register is empty
			if (sercom->isDataRegisterEmptyUART()) {
				IrqHandler();
			}
		}

		txBuffer.store_char(data);

		sercom->enableDataRegisterEmptyInterruptUART();
	}

	return 1;
}

SercomNumberStopBit RS485::extractNbStopBit(uint16_t config) {
	switch (config & HARDSER_STOP_BIT_MASK) {
		case HARDSER_STOP_BIT_1:
		default:
			return SERCOM_STOP_BIT_1;

		case HARDSER_STOP_BIT_2:
			return SERCOM_STOP_BITS_2;
	}
}

SercomUartCharSize RS485::extractCharSize(uint16_t config) {
	switch (config & HARDSER_DATA_MASK) {
		case HARDSER_DATA_5:
			return UART_CHAR_SIZE_5_BITS;

		case HARDSER_DATA_6:
			return UART_CHAR_SIZE_6_BITS;

		case HARDSER_DATA_7:
			return UART_CHAR_SIZE_7_BITS;

		case HARDSER_DATA_8:
		default:
			return UART_CHAR_SIZE_8_BITS;
	}
}

SercomParityMode RS485::extractParity(uint16_t config) {
	switch (config & HARDSER_PARITY_MASK) {
		case HARDSER_PARITY_NONE:
		default:
			return SERCOM_NO_PARITY;

		case HARDSER_PARITY_EVEN:
			return SERCOM_EVEN_PARITY;

		case HARDSER_PARITY_ODD:
			return SERCOM_ODD_PARITY;
	}
}
