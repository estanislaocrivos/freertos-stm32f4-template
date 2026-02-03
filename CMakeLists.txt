cmake_minimum_required(VERSION 3.20)

# Nombre del proyecto y lenguajes
project(freertos-stm32f4 LANGUAGES C ASM)

# Estándar de C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Chip target (cambiar según tu board)
set(MCU_FAMILY STM32F4xx)
set(MCU_MODEL  STM32F401xE)

#-----------------------------------------------------------
# Paths
#-----------------------------------------------------------

set(CMSIS_PATH      ${CMAKE_SOURCE_DIR}/drivers/CMSIS_5/CMSIS/Core/Include)
set(DEVICE_PATH     ${CMAKE_SOURCE_DIR}/drivers/cmsis_device_f4)
set(FREERTOS_PATH   ${CMAKE_SOURCE_DIR}/freertos)
set(STARTUP_PATH    ${CMAKE_SOURCE_DIR}/startup)

#-----------------------------------------------------------
# Fuentes de FreeRTOS
#-----------------------------------------------------------

set(FREERTOS_SOURCES
    ${FREERTOS_PATH}/tasks.c
    ${FREERTOS_PATH}/queue.c
    ${FREERTOS_PATH}/list.c
    ${FREERTOS_PATH}/timers.c
    ${FREERTOS_PATH}/event_groups.c
    ${FREERTOS_PATH}/stream_buffer.c

    # Port para ARM Cortex-M4 con FPU
    ${FREERTOS_PATH}/portable/GCC/ARM_CM4F/port.c

    # Heap implementation (heap_4 es el más usado - permite free())
    ${FREERTOS_PATH}/portable/MemMang/heap_4.c
)

#-----------------------------------------------------------
# Fuentes de la aplicación
#-----------------------------------------------------------

set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
)

#-----------------------------------------------------------
# Startup y system
#-----------------------------------------------------------

set(STARTUP_SOURCES
    ${DEVICE_PATH}/Source/Templates/gcc/startup_stm32f401xe.s
)

#-----------------------------------------------------------
# Linker script
#-----------------------------------------------------------

set(LINKER_SCRIPT ${STARTUP_PATH}/stm32f401xe.ld)

#-----------------------------------------------------------
# Include directories
#-----------------------------------------------------------

set(INCLUDE_DIRS
    # Tu código
    ${CMAKE_SOURCE_DIR}/src

    # CMSIS Core (core_cm4.h, etc)
    ${CMSIS_PATH}

    # Device headers (stm32f4xx.h, stm32f401xe.h)
    ${DEVICE_PATH}/Include

    # FreeRTOS
    ${FREERTOS_PATH}/include
    ${FREERTOS_PATH}/portable/GCC/ARM_CM4F
)

#-----------------------------------------------------------
# Ejecutable
#-----------------------------------------------------------

add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${FREERTOS_SOURCES}
    ${STARTUP_SOURCES}
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INCLUDE_DIRS})

# Define para seleccionar el chip correcto en los headers de ST
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    ${MCU_MODEL}
    USE_FULL_ASSERT  # Habilita asserts de ST (opcional)
)

#-----------------------------------------------------------
# Flags de compilación
#-----------------------------------------------------------

# -fdata-sections -ffunction-sections: pone cada función/variable en su
# propia sección, permitiendo que el linker elimine código no usado

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

#-----------------------------------------------------------
# Flags de linker
#-----------------------------------------------------------

target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections              # Eliminar secciones no usadas
    -Wl,--print-memory-usage       # Mostrar uso de memoria al compilar
    -Wl,-Map=${PROJECT_NAME}.map   # Generar mapa de memoria
    --specs=nano.specs             # Usar newlib-nano (más pequeña)
    --specs=nosys.specs            # No usar syscalls (bare metal)
    -lc                            # Link con libc
    -lm                            # Link con libm (math)
    -lnosys                        # Stubs para syscalls
)

#-----------------------------------------------------------
# Post-build: generar .bin y .hex, mostrar tamaño
#-----------------------------------------------------------

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    # Generar archivo .hex (para algunos programadores)
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex

    # Generar archivo .bin (para ST-Link, OpenOCD, etc)
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin

    # Mostrar tamaño de las secciones
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf

    COMMENT "Generando ${PROJECT_NAME}.bin y ${PROJECT_NAME}.hex"
)

#-----------------------------------------------------------
# Target para flashear (opcional, requiere st-flash instalado)
#-----------------------------------------------------------

add_custom_target(flash
    COMMAND st-flash --connect-under-reset write ${PROJECT_NAME}.bin 0x08000000
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flasheando ${PROJECT_NAME}.bin al STM32..."
)

#-----------------------------------------------------------
# Targets para debugging
#-----------------------------------------------------------

# Iniciar OpenOCD (correr en una terminal separada)
add_custom_target(openocd
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    COMMENT "Iniciando OpenOCD... (Ctrl+C para terminar)"
)

# Iniciar GDB y conectar a OpenOCD (requiere OpenOCD corriendo)
# Obtener el directorio del compilador para encontrar GDB
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
    COMMENT "Iniciando GDB..."
)

# Flashear usando OpenOCD (alternativa a st-flash)
add_custom_target(flash-openocd
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
        -c "program ${PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flasheando con OpenOCD..."
)
