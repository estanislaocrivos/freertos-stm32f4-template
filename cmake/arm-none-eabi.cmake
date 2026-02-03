# Toolchain file para cross-compilar a ARM Cortex-M
# CMake lo usa para saber qué compilador usar

set(CMAKE_SYSTEM_NAME Generic)        # "Generic" = bare metal (sin OS de host)
set(CMAKE_SYSTEM_PROCESSOR arm)       # Arquitectura target

# Path al toolchain de ARM (ajustar si está en otro lugar)
set(ARM_TOOLCHAIN_PATH "/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi")
set(ARM_TOOLCHAIN_BIN "${ARM_TOOLCHAIN_PATH}/bin")

# Especificar el compilador con path completo
set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")

# Herramientas adicionales
set(CMAKE_OBJCOPY "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-objdump")
set(CMAKE_SIZE "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-size")

# Flags comunes para Cortex-M4 con FPU
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# -mcpu=cortex-m4   -> genera código para Cortex-M4
# -mthumb           -> usa instrucciones Thumb (16/32 bit, más compactas)
# -mfpu=fpv4-sp-d16 -> usa la FPU de precisión simple
# -mfloat-abi=hard  -> pasa floats en registros FPU (más rápido)

set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}")

# Evitar que CMake intente linkear un ejecutable de prueba
# (fallaría porque no tenemos libc ni startup code configurado aún)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
