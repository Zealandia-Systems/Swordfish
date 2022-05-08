#include <charconv>

#include <SERCOM.h>

#include "modbus.h"

#include "../inc/MarlinConfig.h"
#include "../core/serial.h"
#include "../HAL/HAL.h"
#include "../HAL/SAMD51/RS485.h"

#include <swordfish/debug.h>

using namespace swordfish;

static RS485 __rs485(&sercom5, 15, 14, SERCOM_RX_PAD_1, UART_TX_RTS_PAD_0_2, 8);

extern "C" {
	void SERCOM5_0_Handler() {
		__rs485.IrqHandler();
	}

	void SERCOM5_1_Handler() {
		__rs485.IrqHandler();
	}

	void SERCOM5_2_Handler() {
		__rs485.IrqHandler();
	}

	void SERCOM5_3_Handler() {
		__rs485.IrqHandler();
	}
};

namespace modbus {

	void init(uint32_t baudRate) {
		__rs485.begin(baudRate);
	}

	static const uint16_t __crc_table[] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

	static uint16_t _crc16(uint16_t crc, uint8_t data) {
		uint8_t tmp = data ^ crc;

		crc = crc >> 8;

		crc ^= __crc_table[tmp];

		return crc;
	}

	static uint16_t crc16(uint8_t* data, uint16_t len) {
		// uint8_t tmp;
		uint16_t crc = 0xFFFF;

		while (len--) {
			crc = _crc16(crc, *data++);
		}

		return crc;
	}

	static void log(const char* message, uint8_t* buffer, uint8_t length) {
		static const char lookup[] = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F'
		};

		char formatBuffer[length * 2 + 1];
		char* p = formatBuffer;

		memset(formatBuffer, 0, length * 2 + 1);

		for (auto i = 0; i < length; i++) {
			auto high = buffer[i] >> 4;
			auto low = buffer[i] & 0xF;

			*p++ = lookup[high];
			*p++ = lookup[low];
			// std::to_chars(p, p + 2, buffer[i], 16);
			// snprintf(p, 3, "%02x", buffer[i]);
		}

		debug()(message, (char*) formatBuffer);

		if (DEBUGGING(SPINDLE)) {
			SERIAL_ECHO(message);
			SERIAL_ECHO(formatBuffer);
		}
	}

	static void logln(const char* message, uint8_t* buffer, uint8_t length) {
		log(message, buffer, length);

		if (DEBUGGING(SPINDLE)) {
			SERIAL_ECHO('\n');
		}
	}

	uint16_t read_parameters(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result) {
		const uint8_t length = 8;
		uint8_t i = 0;
		uint8_t buffer[length + count * 2];
		uint16_t crc;

		buffer[i++] = slave; // Slave address
		buffer[i++] = 0x01; // Function code
		buffer[i++] = (address >> 8); // parameter address MSB
		buffer[i++] = address & 0xFF; // parameter address LSB
		buffer[i++] = (count >> 8); // number of coils to read MSB
		buffer[i++] = count & 0xFF; // number of coils to read LSB

		crc = crc16(buffer, length - 2);

		buffer[i++] = crc; // CRC LSB
		buffer[i++] = (crc >> length); // CRC MSB

		logln("modbus::read_parameters request: ", buffer, length);

		__rs485.write(buffer, length);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto bytesRead = __rs485.readBytes(buffer, 3);

		if (bytesRead != 3 || buffer[2] == 0) {
			// read crc
			bytesRead += __rs485.readBytes(buffer + bytesRead, 2);

			logln("modbus::read_parameters response: ", buffer, bytesRead);

			safe_delay(4);

			return 0;
		}

		auto resultOffset = bytesRead;

		bytesRead += __rs485.readBytes(buffer + bytesRead, buffer[2]);

		uint8_t* p = (uint8_t*) result;

		for (auto i = 0; i < count; i++) {
			*(p + 1) = buffer[resultOffset++];
			*p = buffer[resultOffset++];

			p += 2;
		}

		// read crc
		bytesRead += __rs485.readBytes(buffer + bytesRead, 2);

		logln("modbus::read_parameters response: ", buffer, bytesRead);

		safe_delay(4);

		return bytesRead / 2;
	}

	uint16_t read_holding_registers(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result) {
		const uint8_t length = 8;
		uint8_t i = 0;
		uint8_t buffer[length + count * 2];
		uint16_t crc;

		buffer[i++] = slave; // Slave address
		buffer[i++] = 0x03; // Function code
		buffer[i++] = (address >> 8); // parameter address MSB
		buffer[i++] = address & 0xFF; // parameter address LSB
		buffer[i++] = (count >> 8); // number of coils to read MSB
		buffer[i++] = count & 0xFF; // number of coils to read LSB

		crc = crc16(buffer, length - 2);

		buffer[i++] = crc; // CRC LSB
		buffer[i++] = (crc >> length); // CRC MSB

		logln("modbus::read_holding_registers request: ", buffer, length);

		__rs485.write(buffer, length);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto bytesRead = __rs485.readBytes(buffer, 3);

		if (bytesRead != 3 || buffer[2] == 0) {
			// read crc
			bytesRead += __rs485.readBytes(buffer + bytesRead, 2);

			logln("modbus::read_holding_registers response: ", buffer, bytesRead);

			safe_delay(4);

			return 0;
		}

		auto resultOffset = bytesRead;

		auto resultBytes = __rs485.readBytes(buffer + bytesRead, buffer[2]);

		bytesRead += resultBytes;

		uint8_t* p = (uint8_t*) result;

		for (auto i = 0; i < count; i++) {
			*(p + 1) = buffer[resultOffset++];
			*p = buffer[resultOffset++];

			p += 2;
		}

		// read crc
		bytesRead += __rs485.readBytes(buffer + bytesRead, 2);

		logln("modbus::read_holding_registers response: ", buffer, bytesRead);

		safe_delay(4);

		return resultBytes / 2;
	}

	uint16_t read_input_registers(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result) {
		const uint8_t length = 8;
		uint8_t i = 0;
		uint8_t buffer[length];
		uint16_t crc;

		buffer[i++] = slave;
		buffer[i++] = 0x04;
		buffer[i++] = (address >> 8);
		buffer[i++] = address & 0xFF;
		buffer[i++] = (count >> 8);
		buffer[i++] = count & 0xFF;

		crc = crc16(buffer, length - 2);

		buffer[i++] = crc;
		buffer[i++] = (crc >> length);

		logln("modbus::read_input_registers request: ", buffer, length);

		__rs485.write(buffer, length);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto bytesRead = __rs485.readBytes(buffer, 3);

		log("modbus::read_input_registers response: ", buffer, bytesRead);

		if (bytesRead != 3 || buffer[2] == 0) {
			// read crc
			__rs485.readBytes(buffer, 2);

			logln("", nullptr, 0);

			safe_delay(4);

			return 0;
		}

		bytesRead = __rs485.readBytes((uint8_t*) result, buffer[2]);

		for (auto i = 0; i < count; i++) {
			auto temp = result[i];

			result[i] = temp << 8 | temp >> 8;
		}

		// read crc
		__rs485.readBytes(buffer, 2);

		logln("", (uint8_t*) result, bytesRead);

		safe_delay(4);

		return bytesRead / 2;
	}

	void write_parameter(uint8_t slave, uint16_t address, uint16_t data) {
		uint8_t i = 0;
		const uint8_t length = 8;
		uint8_t buffer[8];
		uint16_t crc;

		buffer[i++] = slave; // Slave address
		buffer[i++] = 0x05; // Function code
		buffer[i++] = (address >> 8); // Coil address MSB
		buffer[i++] = address & 0xFF; // Coil address LSB
		buffer[i++] = (data >> 8); // Data MSB
		buffer[i++] = data & 0xFF; // Data LSB

		crc = crc16(buffer, 6);

		buffer[i++] = crc; // CRC LSB
		buffer[i++] = (crc >> 8); // CRC MSB

		logln("modbus::write_parameter request: ", buffer, length);

		__rs485.write(buffer, length);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto read = __rs485.readBytes(buffer, length);

		logln("modbus::write_parameter response: ", buffer, read);

		safe_delay(4);
	}

	void write_holding_register(uint8_t slave, uint16_t address, uint16_t data) {
		uint8_t i = 0;
		const uint8_t length = 8;
		uint8_t buffer[8];
		uint16_t crc;

		buffer[i++] = slave; // Slave address
		buffer[i++] = 0x06; // Function code
		buffer[i++] = (address >> 8); // Register address MSB
		buffer[i++] = address; // Register address LSB
		buffer[i++] = (data >> 8); // Data MSB
		buffer[i++] = data & 0xFF; // Data LSB

		crc = crc16(buffer, 6);

		buffer[i++] = crc; // CRC LSB
		buffer[i++] = (crc >> 8); // CRC MSB

		logln("modbus::write_holding_register request: ", buffer, length);

		__rs485.write(buffer, 8);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto read = __rs485.readBytes(buffer, 8);

		logln("modbus::write_holding_register response: ", buffer, read);

		safe_delay(4);
	}

	void diagnostic(uint8_t slave, uint16_t test_sub_code, uint16_t data) {
		// TODO: implement this
	}

	void write_multiple_parameters(uint8_t slave, uint16_t address, uint16_t n_coils, uint16_t* data) {
		// TODO: implement this
	}

	void write_multiple_holding_registers(uint8_t slave, uint16_t address, uint8_t count, uint16_t* data) {
		uint8_t i = 0;
		uint8_t length = 9 + count * 2;

		uint8_t buffer[length];

		buffer[i++] = slave;
		buffer[i++] = 0x10;
		buffer[i++] = address >> 8;
		buffer[i++] = address;
		buffer[i++] = count >> 8;
		buffer[i++] = count;
		buffer[i++] = count * 2;

		for (auto j = 0; j < count; j++) {
			uint16_t value = *data++;

			buffer[i++] = value >> 8;
			buffer[i++] = value;
		}

		uint16_t crc = crc16(buffer, i);

		buffer[i++] = crc;
		buffer[i++] = crc >> 8;

		logln("modbus::write_multiple_holding_registers request: ", buffer, length);

		__rs485.write(buffer, length);
		__rs485.flush();
		__rs485.clear();

		safe_delay(4);

		auto read = __rs485.readBytes(buffer, length);

		logln("modbus::write_multiple_holding_registers response: ", buffer, read);

		safe_delay(4);
	}

	void read_write_multiple_registers(uint8_t slave, uint16_t read_addr, uint16_t count, uint16_t write_addr, uint16_t* data) {
		// TODO: implement this
	}
} // namespace modbus
