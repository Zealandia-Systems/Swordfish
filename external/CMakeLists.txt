cmake_minimum_required(VERSION 3.22.0)

# Adafruit_SPIFlash

target_sources(${PROJECT_NAME}.elf
	PRIVATE
		Adafruit_SPIFlash/src/qspi/Adafruit_FlashTransport_QSPI_SAMD.cpp
		Adafruit_SPIFlash/src/qspi/Adafruit_FlashTransport_QSPI.h
		Adafruit_SPIFlash/src/Adafruit_FlashCache.cpp
		Adafruit_SPIFlash/src/Adafruit_FlashCache.h
		Adafruit_SPIFlash/src/Adafruit_FlashTransport.h
		Adafruit_SPIFlash/src/Adafruit_SPIFlash.cpp
		Adafruit_SPIFlash/src/Adafruit_SPIFlash.h
		Adafruit_SPIFlash/src/Adafruit_SPIFlashBase.cpp
		Adafruit_SPIFlash/src/Adafruit_SPIFlashBase.h
		Adafruit_SPIFlash/src/flash_devices.h	
)

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		Adafruit_SPIFlash/src
		Adafruit_SPIFlash/src/qspi
)

# ArduinoCore-samd

target_sources(${PROJECT_NAME}.elf
	PRIVATE
		ArduinoCore-samd/cores/arduino/abi.cpp
		ArduinoCore-samd/cores/arduino/Arduino.h
		ArduinoCore-samd/cores/arduino/avr
		ArduinoCore-samd/cores/arduino/binary.h
		ArduinoCore-samd/cores/arduino/Client.h
		ArduinoCore-samd/cores/arduino/cortex_handlers.c
		ArduinoCore-samd/cores/arduino/delay.c
		ArduinoCore-samd/cores/arduino/delay.h
		ArduinoCore-samd/cores/arduino/HardwareSerial.h
		ArduinoCore-samd/cores/arduino/hooks.c
		ArduinoCore-samd/cores/arduino/IPAddress.cpp
		ArduinoCore-samd/cores/arduino/IPAddress.h
		ArduinoCore-samd/cores/arduino/itoa.c
		ArduinoCore-samd/cores/arduino/itoa.h
		ArduinoCore-samd/cores/arduino/main.cpp
		#ArduinoCore-samd/cores/arduino/math_helper.c
		#ArduinoCore-samd/cores/arduino/math_helper.h
		ArduinoCore-samd/cores/arduino/new.cpp
		ArduinoCore-samd/cores/arduino/Print.cpp
		ArduinoCore-samd/cores/arduino/Print.h
		ArduinoCore-samd/cores/arduino/Printable.h
		ArduinoCore-samd/cores/arduino/pulse.c
		ArduinoCore-samd/cores/arduino/pulse.h
		ArduinoCore-samd/cores/arduino/pulse_asm.S
		ArduinoCore-samd/cores/arduino/Reset.cpp
		ArduinoCore-samd/cores/arduino/Reset.h
		ArduinoCore-samd/cores/arduino/RingBuffer.h
		ArduinoCore-samd/cores/arduino/SERCOM.cpp
		ArduinoCore-samd/cores/arduino/SERCOM.h
		ArduinoCore-samd/cores/arduino/Server.h
		ArduinoCore-samd/cores/arduino/startup.c
		ArduinoCore-samd/cores/arduino/Stream.cpp
		ArduinoCore-samd/cores/arduino/Stream.h
		#ArduinoCore-samd/cores/arduino/Tone.cpp
		#ArduinoCore-samd/cores/arduino/Tone.h
		ArduinoCore-samd/cores/arduino/Uart.cpp
		ArduinoCore-samd/cores/arduino/Uart.h
		ArduinoCore-samd/cores/arduino/Udp.h
		ArduinoCore-samd/cores/arduino/USB
		ArduinoCore-samd/cores/arduino/USB/CDC.cpp
		ArduinoCore-samd/cores/arduino/USB/PluggableUSB.cpp
		ArduinoCore-samd/cores/arduino/USB/PluggableUSB.h
		ArduinoCore-samd/cores/arduino/USB/samd21_host.c
		ArduinoCore-samd/cores/arduino/USB/samd21_host.h
		ArduinoCore-samd/cores/arduino/USB/SAMD21_USBDevice.cpp
		ArduinoCore-samd/cores/arduino/USB/SAMD21_USBDevice.h
		ArduinoCore-samd/cores/arduino/USB/SAMR21_USBDevice.h
		ArduinoCore-samd/cores/arduino/USB/USBAPI.h
		ArduinoCore-samd/cores/arduino/USB/USBCore.cpp
		ArduinoCore-samd/cores/arduino/USB/USBCore.h
		ArduinoCore-samd/cores/arduino/USB/USBDesc.h
		ArduinoCore-samd/cores/arduino/USB/USB_host.h
		ArduinoCore-samd/cores/arduino/WCharacter.h
		ArduinoCore-samd/cores/arduino/WInterrupts.c
		ArduinoCore-samd/cores/arduino/WInterrupts.h
		ArduinoCore-samd/cores/arduino/wiring.c
		ArduinoCore-samd/cores/arduino/wiring.h
		ArduinoCore-samd/cores/arduino/wiring_analog.c
		ArduinoCore-samd/cores/arduino/wiring_analog.h
		ArduinoCore-samd/cores/arduino/wiring_constants.h
		ArduinoCore-samd/cores/arduino/wiring_digital.c
		ArduinoCore-samd/cores/arduino/wiring_digital.h
		ArduinoCore-samd/cores/arduino/wiring_private.c
		ArduinoCore-samd/cores/arduino/wiring_private.h
		ArduinoCore-samd/cores/arduino/wiring_shift.c
		ArduinoCore-samd/cores/arduino/wiring_shift.h
		ArduinoCore-samd/cores/arduino/WMath.cpp
		ArduinoCore-samd/cores/arduino/WMath.h
		ArduinoCore-samd/cores/arduino/WString.cpp
		ArduinoCore-samd/cores/arduino/WString.h
		ArduinoCore-samd/cores/arduino/WVariant.h
		ArduinoCore-samd/libraries/Adafruit_ZeroDMA/Adafruit_ZeroDMA.cpp
		ArduinoCore-samd/libraries/Adafruit_ZeroDMA/Adafruit_ZeroDMA.h
		ArduinoCore-samd/libraries/SPI/SPI.cpp
		ArduinoCore-samd/libraries/SPI/SPI.h
		ArduinoCore-samd/variants/grand_central_m4/pins_arduino.h
		ArduinoCore-samd/variants/grand_central_m4/variant.cpp
		ArduinoCore-samd/variants/grand_central_m4/variant.h
)

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		ArduinoCore-samd/cores/arduino
		ArduinoCore-samd/variants/grand_central_m4
		ArduinoCore-samd/libraries/Adafruit_ZeroDMA
		ArduinoCore-samd/libraries/SPI
)

# ArduinoModule-CMSIS-Atmel

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		ArduinoModule-CMSIS-Atmel/CMSIS-Atmel/CMSIS/Device/ATMEL/
)

# CMSIS_5

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		CMSIS_5/CMSIS/Core/Include
		CMSIS_5/CMSIS/DSP/Include
)

# eigen

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		eigen
)

# SdFat

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		SdFat/src
)