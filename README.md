# FreeRTOS STM32F4 Template 🚝

This repository provides a template project for developing applications using FreeRTOS on STM32F4 microcontrollers. It includes a basic setup with FreeRTOS, necessary configurations, and example tasks to get you started quickly.

## Prerequisites

- **32-bit ARM GCC Toolchain** (`arm-none-eabi`). Visit the [ARM Developer website](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) to download and install the toolchain.
- **CMake (version 3.13 or higher)** for building the project. You can download it from the [CMake website](https://cmake.org/download/).
- **OpenOCD** for flashing and debugging. Installation instructions can be found on the [OpenOCD website](http://openocd.org/).
- An **STM32F4 development board** (e.g., STM32F407 Discovery, Nucleo-F446RE, etc.)
- **ST-Link Utility** or equivalent flashing tool for programming the microcontroller (e.g., `st-flash`).

## Building the Project

To build the project, run the following commands in your terminal:

```bash
mkdir build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./cmake/arm-none-eabi.cmake
cmake --build build --config Release
```

Or you can use the `make` commands, inside the `build/` directory:

```bash
make
```

## Flashing the Firmware

To flash the compiled firmware onto your STM32F4 microcontroller, use the following command:

```bash
st-flash --connect-under-reset write build/freertos-stm32f4.bin 0x08000000
```

Or you can use the `make flash` command inside the `build/` directory:

```bash
make flash
```

## Debugging the Application

Open two terminal windows to debug the application using OpenOCD and GDB. On the first terminal, start OpenOCD to interface with the STM32F4 microcontroller. On the second terminal, launch GDB to connect to OpenOCD and control the debugging session.

```bash
# Terminal 1: OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
```

```bash
# Terminal 2: GDB
arm-none-eabi-gdb build/freertos-stm32f4.elf \
    -ex "target extended-remote localhost:3333" \
    -ex "monitor reset halt" \
    -ex "load" \
    -ex "break main" \
    -ex "continue"
```
