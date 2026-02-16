/*
 * FreeRTOS STM32F4 Template
 *
 * Basic example on how to create tasks
 * USART2 (VCP via ST-Link): PA2 (TX), PA3 (RX)
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "led.h"
#include "logging.h"
#include "projdefs.h"
#include "task.h"

#include <stdint.h>
#include <string.h>

/* ========================================================================== */

void vPrintString(const char* str)
{
    /* Thread-safe print using FreeRTOS critical section */
    taskENTER_CRITICAL();
    log_info(str);
    taskEXIT_CRITICAL();
}

/* ========================================================================== */

void vTask1(void* pvParameters)
{
    const char* pcTaskName = (char*)(pvParameters);

    for (;;) /* Tasks must never return */
    {
        vPrintString(pcTaskName);

        /* Block the task for 500ms.
         * During this time, other tasks can run.
         * pdMS_TO_TICKS converts milliseconds to RTOS ticks. */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTask2(void* pvParameters)
{
    const char* pcTaskName = (char*)(pvParameters);

    for (;;) /* Tasks must never return */
    {
        vPrintString(pcTaskName);

        /* Block the task for 500ms.
         * During this time, other tasks can run.
         * pdMS_TO_TICKS converts milliseconds to RTOS ticks. */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ========================================================================== */

/* In this application, Task 2 will start running given that it is the one with
 * the highest priority ready to be run (in the ready state). Task 2 will print
 * a string through the logging utility. After printing the message it will
 * block itself for 1000 ms. Then, Task 1 will be executed and will print a
 * string through the logging utility, and it will block itself for 500 ms.
 * After this time, Task 2 becomes ready for execution again, and although Task
 * 1 would be ready too, Task 2 will be executed due to being the one with the
 * highest priority */

int main(void)
{
    log_init();
    led_init();

    const char* vTask1Description = "Task 1 running";
    const char* vTask2Description = "Task 2 running";

    xTaskCreate(vTask1, "Task 1", 128, (void*)vTask1Description, 1, NULL);

    xTaskCreate(vTask2, "Task 2", 128, (void*)vTask2Description, 2, NULL);

    led_on();
    vTaskStartScheduler();

    log_err("Could not initialize scheduler");

    for (;;)
    {
        /* Very fast error blink */
        led_toggle();
        for (volatile int i = 0; i < 100000; i++)
        {
        }
    }

    return 0;
}

/* ========================================================================== */
