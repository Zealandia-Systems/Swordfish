/*
 * Controller.cpp
 *
 * Created: 8/08/2021 1:31:37 pm
 *  Author: smohekey
 */ 

#include "Controller.h"

#include <functional>

#include <sam.h>

#include <swordfish/core/Console.h>
#include <swordfish/debug.h>

#include <Adafruit_SPIFlashBase.h>

#include "../src/hal/SAMD51/watchdog.h"
#include "../src/MarlinCore.h"

namespace swordfish {
	using namespace core;
	
	static constexpr uint32_t MAGIC = 0xBEEFDEAD;
	
	Controller* Controller::__instance = nullptr;

	core::ObjectField<tools::ToolsModule> Controller::__toolingModuleField = { "tooling", 0, getToolsModule };
	core::ObjectField<motion::MotionModule> Controller::__motionModuleField = { "motion", 1, getMotionModule };
	core::ObjectField<estop::EStopModule> Controller::__estopModuleField = { "estop", 2, getEStopModule };
	core::ObjectField<gpio::GPIOModule> Controller::__gpioModuleField = { "gpio", 3, getGPIOModule };
		
	core::Schema Controller::__schema = {
		utils::typeName<Controller>(),
		nullptr, {
			
			}, {
			__toolingModuleField,
			__motionModuleField,
			__estopModuleField,
			__gpioModuleField
		}
	};

	core::Pack& Controller::getPack() {
		return _pack;
	}
	
	static bool readHeader(io::WrappingInputStream& stream, offset_t offset, uint32_t& length) {
		uint32_t magic;
		uint32_t self;
		uint32_t version;
		
		stream.seek(offset, io::Origin::Start);
		
		stream.read(&magic, 4);
		
		debug()("magic: ", (void*)magic);
		
		if(magic != MAGIC) {
			return false;
		}
		
		stream.read(&self, 4);
		
		debug()("self: ", self);
		
		if(self != offset) {
			return false;
		}
		
		stream.read(&version, 4);
		stream.read(&length,  4);
		
		debug()("version: ", version);
		debug()("length: ", length);
		
		return true;
	}

	static void readObject(io::InputStream& stream, uint32_t depth) {
		HAL_watchdog_refresh();
		
		if(++depth > 10) {
			return;
		}
		
		uint16_t valuesLength;
		uint16_t objectCount;
		
		stream.read(&valuesLength, sizeof(valuesLength));
		stream.read(&objectCount, sizeof(objectCount));
		
		if(valuesLength > 1024 || objectCount > 1024) {
			return;
		}
		
		while(valuesLength) {
			uint8_t dummy;
			
			stream.read(&dummy, 1);
			
			valuesLength--;
		}
		
		while(objectCount) {
			readObject(stream, depth);
			
			objectCount--;
		}
	}
	
	void Controller::loadConfig(PersistentStoreInputStream& store, io::WrappingInputStream& stream, offset_t offset) {
		uint32_t magic;
		uint32_t self;
		uint32_t version;
		uint32_t reserved;
		uint16_t crc;
		uint16_t storedCrc;
		
		stream.seek(offset, io::Origin::Start);
		
		store.resetCRC();
		
		stream.read(&magic, 4);
		
		if(magic != MAGIC) {
			throw FormatException { "magic header is incorrect." };
		}
		
		stream.read(&self, 4);
		
		if(self != offset) {
			throw FormatException { "self offset is incorrect." };
		}
		
		stream.read(&version, 4);
		stream.read(&reserved, 4);
		
		Console::out() << "version: " << version << io::nl;
		
		read(stream);
		
		crc = store.getCRC();
		
		stream.read(&storedCrc, 2);
		
		if(storedCrc != crc) {
			Console::out() << "crc: " << crc << " stored crc: " << storedCrc << io::nl;
			
			throw FormatException { "crc is incorrect." };
		}
	}
	
	static offset_t align(offset_t current) {
		void* p = (void*)current;
		size_t space = CONFIG_END - current;
		
		return (offset_t)std::align(SFLASH_SECTOR_SIZE, SFLASH_SECTOR_SIZE, p, space);
	}

	Controller::Controller() :
		core::Object(nullptr),
		_pack(__schema, *this),
		_configVersion(0),
		_configStart(0),
		_configEnd(0),
		_modules({
			&static_cast<Module&>(__toolingModuleField.get(_pack)),
			&static_cast<Module&>(__motionModuleField.get(_pack)),
			&static_cast<Module&>(__estopModuleField.get(_pack)),
			&static_cast<Module&>(__gpioModuleField.get(_pack))
		}) {
			
	}

	void Controller::init() {
		// Setup a 24Mhz clock
		GCLK->GENCTRL[7].reg =
			GCLK_GENCTRL_DIV(2) |
			GCLK_GENCTRL_IDC |
			GCLK_GENCTRL_GENEN |
			GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val);
				
		while (GCLK->SYNCBUSY.bit.GENCTRL7);               // Wait for synchronization
				
		// Check startup - does nothing if bootloader sets MCUSR to 0
		const byte mcu = HAL_get_reset_source();
		if (mcu & RST_POWER_ON) debug()(STR_POWERUP);
		if (mcu & RST_EXTERNAL) debug()(STR_EXTERNAL_RESET);
		if (mcu & RST_BROWN_OUT) debug()(STR_BROWNOUT_RESET);
		if (mcu & RST_WATCHDOG) debug()(STR_WATCHDOG_RESET);
		if (mcu & RST_SOFTWARE) debug()(STR_SOFTWARE_RESET);
		
		for(auto* module : _modules) {
			debug()("Initializing module ", module->name());
			
			module->init();
		}
	}

	void Controller::idle() {
		for(auto* module : _modules) {
			module->idle();
		}
	}
	
	bool Controller::findNewestConfig(PersistentStoreInputStream& store, io::WrappingInputStream& stream, offset_t& offset) {
		offset = 0;
		offset_t lastOffset = 0;
		
		while(offset < CONFIG_END - CONFIG_START) {
			debug()("Checking offset: ", (uint32_t)offset);
			
			uint32_t length;
			
			HAL_watchdog_refresh();
			
			if(readHeader(stream, offset, length)) {
				lastOffset = offset;
			} else {
				break;
			}
			
			offset = align(offset + length);
			//offset += SFLASH_SECTOR_SIZE;
		}
		
		// now work backwards finding a valid config
		
		offset = lastOffset;
		
		while(offset >= 0) {
			try {
				loadConfig(store, stream, offset);
			
				_configEnd = stream.seek(0, io::Origin::Current);
			
				return true;
			} catch(Exception& e) {
				
			}
			
			offset -= SFLASH_SECTOR_SIZE;
		}
		
		return false;
	}
	
	void Controller::load() {
		//reset();
		
		PersistentStoreInputStream store;
		
		io::WrappingInputStream stream(store, CONFIG_START, CONFIG_END, 0);
		
		offset_t offset;
		
		if(!findNewestConfig(store, stream, offset)) {
			Console::out() << "No valid config found." << io::nl;
			
			save();
		} else {
			Console::out() << "Found config " << _configVersion << " at " << _configStart + CONFIG_START << " with length " << _configEnd - _configStart << io::nl;
		}
	}
	
	static bool validateCRC(io::WrappingInputStream& stream, offset_t offset, uint16_t crc) {
		uint16_t storedCRC;
		
		stream.seek(offset, io::Origin::Start);
		
		stream.read(&storedCRC, sizeof(uint16_t));
		
		debug()("storedCRC: ", storedCRC, " CRC: ", crc);
		
		return storedCRC == crc;
	}

	void Controller::save() {
		//debug()(_configEnd);
		offset_t offset = _configStart;
		uint16_t crc = 0;
		
		PersistentStoreInputStream inputStore;
		PersistentStoreOutputStream outputStore;
		io::WrappingInputStream input(inputStore, CONFIG_START, CONFIG_END, 0);
		io::WrappingOutputStream output(outputStore, CONFIG_START, CONFIG_END, 0);
		
		_configVersion++;
		
		do {
			HAL_watchdog_refresh();
			
			debug()("attempting to write at ", (uint32_t)offset);
			
			outputStore.resetCRC();
			
			output.seek(offset, io::Origin::Start);
			
			uint32_t buffer = MAGIC;
			output.write(&buffer, 4);
			
			buffer = output.seek(0, io::Origin::Current);
			
			buffer -= 4;
			output.write(&buffer, 4);
			
			buffer = _configVersion;
			output.write(&buffer, 4);
			
			debug()("version: ", buffer);
			
			buffer = _pack.length();
			output.write(&buffer, 4);
			
			debug()("length: ", buffer);
			
			write(output);
			
			crc = outputStore.getCRC();
			
			debug()("crc: ", crc);
			
			output.write(&crc, 2);
			
			offset += SFLASH_SECTOR_SIZE;
		} while((offset < CONFIG_END - CONFIG_START) && !validateCRC(input, output.seek(-2, io::Origin::Current), crc));
	}
	
	void Controller::reset() {
		Adafruit_FlashTransport_QSPI transport = { };
		Adafruit_SPIFlashBase flash = { &transport };
		
		flash.begin(nullptr);
		
		_configStart = _configEnd = _configVersion = 0;
		
		uint32_t offset = CONFIG_START;
		
		debug()("Resetting eeprom.");
		
		while(offset < CONFIG_END) {
			auto block = offset / SFLASH_BLOCK_SIZE;
			
			if(block > 0) {
				debug()("Erasing address: ", offset, " block: ", block);
				
				flash.eraseBlock(block);
				
				offset += SFLASH_BLOCK_SIZE;
			} else {
				auto sector = offset / SFLASH_SECTOR_SIZE;
				
				debug()("Erasing address: ", offset, " sector: ", sector);
				
				flash.eraseSector(sector);
				
				offset += SFLASH_SECTOR_SIZE;
			}
			
			HAL_watchdog_refresh();
		}
	}

	swordfish::Controller& Controller::getInstance() {
		return *(__instance ?: __instance = new Controller());
	}

	swordfish::tools::ToolsModule* Controller::getToolsModule([[maybe_unused]] Object* parent) {
		return &tools::ToolsModule::getInstance(parent);
	}

	swordfish::motion::MotionModule* Controller::getMotionModule([[maybe_unused]] Object* parent) {
		return &motion::MotionModule::getInstance(parent);
	}

	swordfish::estop::EStopModule* Controller::getEStopModule([[maybe_unused]] Object* parent) {
		return &estop::EStopModule::getInstance(parent);
	}

	swordfish::gpio::GPIOModule* Controller::getGPIOModule([[maybe_unused]] Object* parent) {
		return &gpio::GPIOModule::getInstance(parent);
	}

}