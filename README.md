# FreeRTOS STM32F4 Template 🚝

This repository provides a template project for developing applications using FreeRTOS on STM32F4 microcontrollers. It includes a basic setup with FreeRTOS, necessary configurations, and example tasks to get you started quickly.

## Prerequisites

<!-- TODO: check if the prerequisites are correct and update if necessary -->

- **32-bit ARM GCC Toolchain** (`arm-none-eabi`). Visit the [ARM Developer website](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) to download and install the toolchain.
- **CMake (version 3.13 or higher)** for building the project. You can download it from the [CMake website](https://cmake.org/download/).
- **OpenOCD** for flashing and debugging. Installation instructions can be found on the [OpenOCD website](http://openocd.org/).
- An **STM32F4 development board** (e.g., STM32F407 Discovery, Nucleo-F446RE, etc.)
- **ST-Link Utility** or equivalent flashing tool for programming the microcontroller (e.g., `st-flash`).

## Building the Project

<!-- TODO: update the build instructions -->

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

<!-- TODO: check if debug process works and update instructions -->

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

## Getting Started with FreeRTOS

FreeRTOS is a lightweight kernel used for task-scheduling on small targets such as microcontrollers. This kernel allows for the applications to be organized as a set of tasks which can be thought as independent threads of execution.

### The Task Function

In FreeRTOS, a task is a function with the following definition:

```bash
void aTaskFunction(void *pvParameters);
```

Each task is a small program on its own. It has an entry point and an infinite loop, and should not exit, which means, should not contain a return statement. If a task is no longer required after it has been ran, should be deleted. An example of a task definition is shown below:

```c
void aTaskFunction(void *pvParameters)
{
    /* This variable is local to the application and is allocated on the stack as any other variables inside a function */
    uint8_t a_variable = 0;

    /* Infinite loop */
    for (;;)
    {
        /* The code which must implement the task functionality should go here */
    }

    /* If the task breaks out of the previous loop (not common in general) then it should call the vTaskDelete function, which will delete the task before it reaching the end of the function */
    vTaskDelete(NULL);
}
```

A single task definition/declaration can be used to create multiple tasks, because the `pvParameters` pointer could be casted to any chosen type which makes it easy and flexible to pass data to tasks.

### Creating a Task

For a task to be created, you may call the `xTaskCreate` function. This function's declaration is shown below:

```c
portBASE_TYPE xTaskCreate(pdTASK_CODE pvTaskCode,
                          const signed char * const pcName,
                          unsigned short usStackDepth,
                          void *pvParameters,
                          unsigned portBASE_TYPE uxpriority,
                          xTaskHandle *pxCreatedTask);
```

- `pvTaskCode`: this is a pointer to the function which implements the task functionality. As stated before, the task functionality must be implemented as a `void vTaskName(void *pvParameters)` kind of function.
- `pcName`: a descriptive name for the task. This name is not used by FreeRTOS in any way, as it acts solely as a debugging aid. Note that this parameter is implemented as a `const` pointer to a `const` char (string stored in flash memory).
- `usStackDepth`: this parameter refers to the stack depth. Every time FreeRTOS swaps in a task for execution, it must reserve storage in the stack for the stack resources. This value specifies the stack depth in terms of words, so for example, in a Cortex M3 microcontroller the word size is 32-bits. In this case, a parameter with value 100 would mean 400 bytes of stack availability for all the function's local variables.
- `pvParameters`: the value assigned to this parameter will be passed to the task when called by the scheduler.
- `uxPriority`: defines the priority at which the task will execute. Priorities start from 0 being the lowest one, to `configMAX_PRIORITIES - 1`. You can define the number of priorities on the FreeRTOS configuration, although it is always preferrable to use the lowest number of priorities required in order to avoid wasting RAM memory.
- `pxCreatedTask`: this parameter can be used to pass out a handle to the task being created. This handle can be used to reference the task in API calls which, for example, could change the priority of the task. Can be set to NULL for most cases.
- `portBASE_TYPE return value`: the task will return `pdTRUE` if the task has been created successfully or `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY` if the task could not be created due to lack of memory resources.

#### Example

```c
/* Task implementation */
void vTask1(void *pvParameters)
{
    const char *pcTaskName = "Task 1 is running";
    volatile unsigned long u1 = 0;

    for (;;)
    {
        vPrintString(pcTaskName);
        for (u1 = 0; u1 < mainDELAY_LOOP_COUNT; u1++)
        {
            /* Crude delay implementation */
        }
    }
}

/* Main app. entry point. Task initialization */
int main(void)
{
    const char* vTask1Description = "This task does nothing but print a string";
    if (xTaskCreate(
            vTask1,
            vTask1Description,
            configMINIMAL_STACK_SIZE,
            (void*)vTask2Parameter,
            1,
            NULL)
        == pdTRUE)
    {
        /* Task created successfully */
    }

    vTaskStartScheduler();

    /* The app. should never reach this point */
    return 0;
}
```

### Tasks Delays

After a tasks does its job, it should block itself so other tasks can enter the ready state to be executed. For this purpose, the `vTaskDelay` API function can be used for this purpose. This function takes the number of ticks for which the task will block. You may use the `pdMS_TO_TICKS(delay_ms)` macro for converting milliseconds to ticks:

```c
/* Task implementation */
void vTask1(void *pvParameters)
{
    const char *pcTaskName = "Task 1 is running";

    for (;;)
    {
        vPrintString(pcTaskName);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### Queues

Message queues are objects that allow the communication between tasks in a FIFO way. A queue can be written and read by multiple tasks. A task can be in the blocked state waiting for data to be written in a particular queue, or can be waiting to write in a particular queue. A block timeout can be specified during which the task will wait blocked until any of the aforementioned events happen.

```c
xQueueHandle xQueueCreate()
```
