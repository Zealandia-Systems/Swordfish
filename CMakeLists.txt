cmake_minimum_required(VERSION 3.22.0)

message(STATUS "CMake version: ${CMAKE_VERSION}")

if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(FATAL_ERROR "In-source builds are not supported. Please use separate folder for build.")
endif()

enable_language(C)
enable_language(CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Git REQUIRED)
find_package(Python3 REQUIRED)

#add_custom_target(version
#	${CMAKE_COMMAND}
#		-D SRC=${CMAKE_SOURCE_DIR}/Version.h.in
#		-D DST=${CMAKE_BINARY_DIR}/Version.h
#		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
#		-P ${CMAKE_SOURCE_DIR}/cmake/GenerateVersionHeader.cmake
#)

if(GIT_EXECUTABLE)
	# Generate a git-describe version string from Git repository tags
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
		RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(NOT GIT_DESCRIBE_ERROR_CODE)
		set(SWORDFISH_VERSION_SHORT ${GIT_DESCRIBE_VERSION})
	endif()

	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
		RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(NOT GIT_DESCRIBE_ERROR_CODE)
		set(SWORDFISH_VERSION_LONG ${GIT_DESCRIBE_VERSION})
	endif()
endif()

# Final fallback: Just use a bogus version string that is semantically older
# than anything else and spit out a warning to the developer.
if(NOT DEFINED SWORDFISH_VERSION_SHORT)
	set(SWORDFISH_VERSION_SHORT v0.0.0)
  	message(WARNING "Failed to determine SWORDFISH_VERSION_SHORT from Git tags. Using default version \"${SWORDFISH_VERSION_SHORT}\".")
endif()

if(NOT DEFINED SWORDFISH_VERSION_LONG)
  	set(SWORDFISH_VERSION_LONG v0.0.0-unknown)
  	message(WARNING "Failed to determine SWORDFISH_VERSION_LONG from Git tags. Using default version \"${SWORDFISH_VERSION_LONG}\".")
endif()

configure_file(${CMAKE_SOURCE_DIR}/Version.h.in ${CMAKE_BINARY_DIR}/Version.h @ONLY)

message(STATUS "Short Version: ${SWORDFISH_VERSION_SHORT}")
message(STATUS "Long Version: ${SWORDFISH_VERSION_LONG}")

project(Swordfish VERSION ${SWORDFISH_VERSION_SHORT})

add_executable(${PROJECT_NAME}.elf "")

add_subdirectory(external)
add_subdirectory(src)

target_compile_definitions(${PROJECT_NAME}.elf
	PRIVATE
		__SAMD51__
		__SAMD51P20A__
		__FPU_PRESENT
		ARM_MATH_CM4
		ARDUINO=180
		ADAFRUIT_GRAND_CENTRAL_M4
		ARDUINO_GRAND_CENTRAL_M4
		ENABLE_CACHE
		VARIANT_QSPI_BAUD_DEFAULT=50000000
		USBCON
		USB_VID=0x239A
		USB_PID=0x0031
		F_CPU=120000000
		USB_PRODUCT="Swordfish CNC"
		USB_MANUFACTURER="Zealandia Systems Ltd"
		ARDUINO_ARCH_SAMD
		ARDUINOSTL_M_H
		MACHINE_TYPE=${SWORDFISH_MACHINE_TYPE}
		MACHINE_NAME=${SWORDFISH_MACHINE_NAME}
)

target_include_directories(${PROJECT_NAME}.elf
	PRIVATE
		src
		${CMAKE_BINARY_DIR}
)

add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)

add_custom_target(uf2 DEPENDS ${PROJECT_NAME}.bin)
add_custom_command(TARGET uf2
	USES_TERMINAL
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/uf2conv.py -c -b 0x4000 -o "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${SWORDFISH_VERSION}-${SWORDFISH_MACHINE_NAME}.uf2" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin")

add_custom_target(flash DEPENDS ${PROJECT_NAME}.bin)
add_custom_command(TARGET flash
    USES_TERMINAL
    COMMAND JLinkExe -CommanderScript ../flash.jlink)
