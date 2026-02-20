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

Each task is a small program on its own. It has an entry point and an infinite loop, and should not exit, which means, should not contain a return statement. If a task is no longer required after it has been run, should be deleted. An example of a task definition is shown below:

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

A single task definition/declaration can be used to create multiple tasks, because the `pvParameters` pointer could be type cast to any chosen type which makes it easy and flexible to pass data to tasks.

### Creating a Task

For a task to be created, you may call the [`xTaskCreate`](https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/01-xTaskCreate) function. This function's declaration is shown below:

```c
 BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                         const char * const pcName,
                         const configSTACK_DEPTH_TYPE uxStackDepth,
                         void *pvParameters,
                         UBaseType_t uxPriority,
                         TaskHandle_t *pxCreatedTask
                       );
```

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

After a tasks does its job, it should block itself so other tasks can enter the ready state to be executed. To achieve this condition, a task could wait for a timeout or for a resource. the [vTaskDelay](https://www.freertos.org/Documentation/02-Kernel/04-API-references/02-Task-control/01-vTaskDelay) API function can be used for this purpose. This function takes the number of ticks for which the task will block. You may use the `pdMS_TO_TICKS(delay_ms)` macro for converting milliseconds to ticks:

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

### Message Queues

Message queues are objects that allow the communication and synchronization between tasks. A queue consists in a FIFO messaging buffer which can be written and read by multiple tasks. A task can be in the blocked state waiting for data to be written in a particular queue, or can be waiting to write in a particular queue. A block timeout can be specified during which the task will wait blocked until any of the aforementioned events happen. You may create a message queue by calling the [xQueueCreate](https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate) API function:

```c
 QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength,
                             UBaseType_t uxItemSize );
```

#### Example

```c
void vTask1(void* pvParameters)
{
    xQueueHandle* xQueue = (xQueueHandle*)pvParameters;
    uint8_t value_to_send = 1;

    for (;;)
    {
        portBASE_TYPE xStatus = xQueueSendToBack(xQueue, &value_to_send, 0);
        if (xStatus != pdPASS)
        {
            vPrintString("Could not send data to the queue from Task 1");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vTask2(void* pvParameters)
{
    xQueueHandle* xQueue = (xQueueHandle*)pvParameters;
    uint8_t received_value = 0;

    for (;;)
    {
        portBASE_TYPE xStatus = xQueueReceive(xQueue, &received_value, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            vPrintString("Received data on Task 2 from Task 1");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    /* Create the queue and check its handler */
    xQueueHandle xQueue = xQueueCreate(10, sizeof(uint8_t));
    if (xQueue == NULL)
    {
        vPrintString("Could not create the queue");
        return -1;
    }

    if (xTaskCreate(vTask1, "Task 1", 128, (void*)(xQueue), 1, NULL) == pdTRUE)
    {
        /* Task created successfully */
    }
    if (xTaskCreate(vTask2, "Task 2", 128, (void*)(xQueue), 2, NULL) == pdTRUE)
    {
        /* Task created successfully */
    }

    vTaskStartScheduler();

    /* The app. should never reach this point */
    return 0;
}
```

### Binary Semaphores

A [semaphore](https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/00-Semaphores) is a synchronization object which can be used to unblock a task each time a particular interrupt occurs, effectively synchronizing the task with the interrupt. A Binary Semaphore can be used to unblock a task each time a particular interrupt occurs, effectively synchronizing the task with the interrupt. A binary semaphore can be thought as a queue of length 1 (it is actually implemented as one), which can be full or empty (only two states). By calling `xSemaphoreTake`, a task enters the blocked state until the semaphore is freed from within an interrupt or another task. You may create a binary semaphore by calling the [xSemaphoreCreateBinary](https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/01-xSemaphoreCreateBinary) API function, described below:

```c
SemaphoreHandle_t xSemaphoreCreateBinary( void );
```
