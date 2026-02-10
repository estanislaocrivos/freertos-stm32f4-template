cmake_minimum_required(VERSION 3.20)

# Project name and languages
project(freertos-stm32f4 LANGUAGES C ASM)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Target chip (change according to your board)
set(MCU_FAMILY STM32F4xx)
set(MCU_MODEL  STM32F401xE)
# TODO: consider adding support for other STM32F4 models (F405, F407, etc)

# Example selection:
# Usage: cmake -DEXAMPLE=02_queues ..

set(EXAMPLE "00_basic_blink" CACHE STRING "Example to build")

# Validate example exists
if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/main.c")
    set(APP_SOURCES "${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/main.c")
    message(STATUS "Building example: ${EXAMPLE}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/src/main.c")
    set(APP_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")
    message(STATUS "Building: src/main.c")
else()
    message(FATAL_ERROR "No main.c found! Use -DEXAMPLE=<name> or create src/main.c")
endif()

# Paths
set(CMSIS_PATH      ${CMAKE_SOURCE_DIR}/drivers/CMSIS_5/CMSIS/Core/Include)
set(DEVICE_PATH     ${CMAKE_SOURCE_DIR}/drivers/cmsis_device_f4)
set(FREERTOS_PATH   ${CMAKE_SOURCE_DIR}/freertos)
set(STARTUP_PATH    ${CMAKE_SOURCE_DIR}/startup)

# FreeRTOS sources
set(FREERTOS_SOURCES
    ${FREERTOS_PATH}/tasks.c
    ${FREERTOS_PATH}/queue.c
    ${FREERTOS_PATH}/list.c
    ${FREERTOS_PATH}/timers.c
    ${FREERTOS_PATH}/event_groups.c
    ${FREERTOS_PATH}/stream_buffer.c

    # Port for ARM Cortex-M4 with FPU
    ${FREERTOS_PATH}/portable/GCC/ARM_CM4F/port.c

    # Heap implementation (heap_4 is most common - allows free())
    ${FREERTOS_PATH}/portable/MemMang/heap_4.c
)

# Startup and system
set(STARTUP_SOURCES
    ${DEVICE_PATH}/Source/Templates/gcc/startup_stm32f401xe.s
)

# Linker script
set(LINKER_SCRIPT ${STARTUP_PATH}/stm32f401xe.ld)

# Include directories
set(INCLUDE_DIRS
    # Your code
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include

    # CMSIS Core (core_cm4.h, etc)
    ${CMSIS_PATH}

    # Device headers (stm32f4xx.h, stm32f401xe.h)
    ${DEVICE_PATH}/Include

    # FreeRTOS
    ${FREERTOS_PATH}/include
    ${FREERTOS_PATH}/portable/GCC/ARM_CM4F
)

# Executable
add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${FREERTOS_SOURCES}
    ${STARTUP_SOURCES}
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INCLUDE_DIRS})

# Define to select the correct chip in ST headers
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    ${MCU_MODEL}
    USE_FULL_ASSERT  # Enable ST asserts (optional)
)

# Compiler flags
# -fdata-sections -ffunction-sections: puts each function/variable in its
# own section, allowing the linker to remove unused code

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Wall
    -Wextra
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-gdwarf-2>
    $<$<CONFIG:Release>:-O2>
)

# Linker flags
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections              # Remove unused sections
    -Wl,--print-memory-usage       # Show memory usage when compiling
    -Wl,-Map=${PROJECT_NAME}.map   # Generate memory map
    --specs=nano.specs             # Use newlib-nano (smaller)
    --specs=nosys.specs            # Don't use syscalls (bare metal)
    -lc                            # Link with libc
    -lm                            # Link with libm (math)
    -lnosys                        # Stubs for syscalls
)

# Post-build: generate .bin and .hex, show size
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    # Generate .hex file (for some programmers)
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex

    # Generate .bin file (for ST-Link, OpenOCD, etc)
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin

    # Show section sizes
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf

    COMMENT "Generating ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"
)

# Flash target (requires st-flash installed)
add_custom_target(flash
    COMMAND st-flash --connect-under-reset write ${PROJECT_NAME}.bin 0x08000000
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing ${PROJECT_NAME}.bin to STM32..."
)

# Debugging targets
# Start OpenOCD (run in a separate terminal)
add_custom_target(openocd
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    COMMENT "Starting OpenOCD... (Ctrl+C to stop)"
)

# Start GDB and connect to OpenOCD (requires OpenOCD running)
# Get compiler directory to find GDB
get_filename_component(TOOLCHAIN_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)
set(CMAKE_GDB "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gdb")

add_custom_target(debug
    COMMAND ${CMAKE_GDB}
        ${PROJECT_NAME}.elf
        -ex "target extended-remote localhost:3333"
        -ex "monitor reset halt"
        -ex "load"
        -ex "break main"
        -ex "continue"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Starting GDB..."
)

# Flash using OpenOCD (alternative to st-flash)
add_custom_target(flash-openocd
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
        -c "program ${PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing with OpenOCD..."
)
