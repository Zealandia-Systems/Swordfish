target_sources(${PROJECT_NAME}.elf
	PRIVATE
		MarlinCore.cpp
		MarlinCore.h
)

add_subdirectory(core)
add_subdirectory(feature)
add_subdirectory(gcode)
add_subdirectory(HAL)
add_subdirectory(inc)
add_subdirectory(lcd)
add_subdirectory(libs)
add_subdirectory(module)
add_subdirectory(pins)
add_subdirectory(sd)
add_subdirectory(swordfish)
