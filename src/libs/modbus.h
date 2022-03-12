#ifndef MODBUS_H
#define MODBUS_H

#include <cstdint>

namespace modbus {
	void init(uint32_t baudRate = 9600);
	uint16_t read_parameters(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result);
	uint16_t read_holding_registers(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result);
	uint16_t read_input_registers(uint8_t slave, uint16_t address, uint16_t count, uint16_t* result);
	void write_parameter(uint8_t slave, uint16_t address, uint16_t data);
	void write_holding_register(uint8_t slave, uint16_t address, uint16_t data);
	void diagnostic(uint8_t slave, uint16_t test_sub_code, uint16_t data);
	void write_multiple_parameters(uint8_t slave, uint16_t address, uint16_t n_coils, uint16_t* data);
	void write_multiple_holding_registers(uint8_t slave, uint16_t address, uint8_t regCount, uint16_t* data);
	void read_write_multiple_registers(uint8_t slave, uint16_t read_addr, uint16_t count, uint16_t write_addr, uint16_t* data);
}

#endif