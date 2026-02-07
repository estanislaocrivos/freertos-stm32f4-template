# Toolchain file for cross-compiling to ARM Cortex-M
# CMake uses this to know which compiler to use

set(CMAKE_SYSTEM_NAME Generic)        # "Generic" = bare metal (no host OS)
set(CMAKE_SYSTEM_PROCESSOR arm)       # Target architecture

# Common installation paths for ARM GCC toolchain
set(ARM_TOOLCHAIN_HINTS
    # macOS - ARM Developer website installation
    "/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/bin"
    "/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin"
    "/Applications/ArmGNUToolchain/13.3.rel1/arm-none-eabi/bin"
    # macOS - Homebrew
    "/opt/homebrew/bin"
    "/usr/local/bin"
    # Linux common paths
    "/usr/bin"
    "/usr/local/gcc-arm-none-eabi/bin"
    "$ENV{HOME}/.local/bin"
)

# Allow override via environment variable
if(DEFINED ENV{ARM_TOOLCHAIN_PATH})
    list(INSERT ARM_TOOLCHAIN_HINTS 0 "$ENV{ARM_TOOLCHAIN_PATH}/bin")
endif()

# Find ARM GCC compiler
find_program(ARM_GCC
    NAMES arm-none-eabi-gcc
    HINTS ${ARM_TOOLCHAIN_HINTS}
    REQUIRED
)
get_filename_component(ARM_TOOLCHAIN_BIN ${ARM_GCC} DIRECTORY)

message(STATUS "ARM toolchain found: ${ARM_TOOLCHAIN_BIN}")

# Specify compilers
set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")

# Additional tools
set(CMAKE_OBJCOPY "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-objdump")
set(CMAKE_SIZE "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-size")

# Common flags for Cortex-M4 with FPU
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# -mcpu=cortex-m4   -> generate code for Cortex-M4
# -mthumb           -> use Thumb instructions (16/32 bit, more compact)
# -mfpu=fpv4-sp-d16 -> use single-precision FPU
# -mfloat-abi=hard  -> pass floats in FPU registers (faster)

set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}")

# Prevent CMake from trying to link a test executable
# (would fail because we don't have libc or startup code configured yet)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
