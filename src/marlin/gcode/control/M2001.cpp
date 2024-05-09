#include "../../inc/MarlinConfigPre.h"
#include "../gcode.h"

#include <marlin/libs/modbus.h>
#include <swordfish/modules/gcode/CommandException.h>

using namespace swordfish;

enum class Function : uint8_t {
	None = 0x00,
	ReadCoils = 0x01,
	ReadDiscreteInputs = 0x02,
	ReadHoldingRegisters = 0x03,
	ReadInputRegisters = 0x04,
	WriteCoil = 0x05,
	WriteHoldingRegister = 0x06,
	ReadExceptionStatus = 0x07,
	Diagnostic = 0x08,
	GetComEventCounter = 0x0B,
	GetComEventLog = 0x0C,
	WriteCoils = 0x0F,
	WriteHoldingRegisters = 0x10,
};

static uint8_t debug_flags;

FORCE_INLINE
static void push_debug(uint8_t flags) {
	debug_flags = marlin_debug_flags;

	marlin_debug_flags |= flags;
}

static void pop_debug() {
	marlin_debug_flags = debug_flags;
}

void GcodeSuite::M2001(std::function<void(std::function<void(swordfish::io::Writer&)>)> writeResult) {
	auto slave = parser.byteval('S', 1);
	auto address = parser.ushortval('A', 0);

	if (address == 0) {
		throw CommandException { "Address must be specified/non zero." };
	}

	auto function = (Function) parser.byteval('F', (uint8_t) Function::None);

	switch (function) {
		case Function::None: {
			throw CommandException { "Modbus function not specified." };
		}

		case Function::ReadCoils: {
			auto count = parser.byteval('C', 1);
			uint16_t result[count];

			push_debug(MARLIN_DEBUG_SPINDLE);
			modbus::read_parameters(slave, address, count, result);
			pop_debug();

			for (auto i = 0; i < count; i++) {
				SERIAL_ECHOLNPAIR("value: ", result[i]);
			}

			break;
		}

		case Function::ReadHoldingRegisters: {
			auto count = parser.byteval('C', 1);

			uint16_t result[count];

			push_debug(MARLIN_DEBUG_SPINDLE);
			modbus::read_holding_registers(slave, address, count, result);
			pop_debug();

			for (auto i = 0; i < count; i++) {
				SERIAL_ECHOLNPAIR("value: ", result[i]);
			}

			break;
		}

		case Function::WriteCoil: {
			auto value = parser.ushortval('V', 0);

			push_debug(MARLIN_DEBUG_SPINDLE);
			modbus::write_parameter(slave, address, value);
			pop_debug();

			break;
		}

		case Function::WriteHoldingRegister: {
			auto value = parser.ushortval('V', 0);

			push_debug(MARLIN_DEBUG_SPINDLE);
			modbus::write_holding_register(slave, address, value);
			pop_debug();

			break;
		}

		default: {
			throw CommandException { "Modbus function not supported." };
		}
	}
}