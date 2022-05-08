# Cross compiling.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(TOOLCHAIN_PREFIX "arm-none-eabi-")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")

find_program(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}objcopy")

# Must be explicit, not set by CMake.
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}size")

set(CMAKE_ASM_FLAGS " -x assembler-with-cpp"
  CACHE STRING "Flags used by the ASM compiler during all build types.")

set(CMAKE_ASM_FLAGS_DEBUG "-Og -g2 -DDEBUG"
  CACHE STRING "Flags used by the ASM compiler during DEBUG builds.")

set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os"
  CACHE STRING "Flags used by the ASM compiler during MINSIZEREL builds.")

set(CMAKE_ASM_FLAGS_RELEASE "-Os"
  CACHE STRING "Flags used by the ASM compiler during RELEASE builds.")

set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-Os -g2"
  CACHE STRING "Flags used by the ASM compiler during RELWITHDEBINFO builds.")

set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -Wall -Werror"
  CACHE STRING "Flags used by the C compiler during all build types.")

set(CMAKE_C_FLAGS_DEBUG "-Og -g2 -DDEBUG"
  CACHE STRING "Flags used by the C compiler during DEBUG builds.")

set(CMAKE_C_FLAGS_MINSIZEREL "-Os"
  CACHE STRING "Flags used by the C compiler during MINSIZEREL builds.")

set(CMAKE_C_FLAGS_RELEASE "-Os"
  CACHE STRING "Flags used by the C compiler during RELEASE builds.")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -g2"
  CACHE STRING "Flags used by the C compiler during RELWITHDEBINFO builds.")

set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -fno-rtti -mlong-calls -Wall -Werror -Wno-expansion-to-defined"
  CACHE STRING "Flags used by the CXX compiler during all build types.")

set(CMAKE_CXX_FLAGS_DEBUG "-Og -g2 -DDEBUG"
  CACHE STRING "Flags used by the CXX compiler during DEBUG builds.")

set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os"
  CACHE STRING "Flags used by the CXX compiler during MINSIZEREL builds.")

set(CMAKE_CXX_FLAGS_RELEASE "-Os"
  CACHE STRING "Flags used by the CXX compiler during RELEASE builds.")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -g2"
  CACHE STRING "Flags used by the CXX compiler during RELWITHDEBINFO builds.")

set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/samd51p20a_flash.ld -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -static -specs=nosys.specs -Wl,--start-group -lm  -Wl,--end-group -Wl,-Map=Swordfish.map,--cref -Wl,--gc-sections -Xlinker -print-memory-usage -Xlinker -v"
  CACHE STRING "Flags used by the linker during all build types.")
# -----------------------------------------------------------------------------
