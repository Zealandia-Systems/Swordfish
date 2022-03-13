target_sources(${PROJECT_NAME}.elf
	PRIVATE
		crc16.cpp
		crc16.h
		modbus.cpp
		modbus.h
		stopwatch.cpp
		stopwatch.h
)

add_subdirectory(rtt)
