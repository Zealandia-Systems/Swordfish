target_sources(${PROJECT_NAME}.elf
	PRIVATE
		HAL.h
		platforms.h
)

add_subdirectory(SAMD51)
add_subdirectory(shared)
