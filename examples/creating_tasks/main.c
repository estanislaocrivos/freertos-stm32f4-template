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

#define mainDELAY_LOOP_COUNT 1000000UL

void vGenericTask(void* pvParameters)
{
    const char*            pcTaskName = (char*)(pvParameters);
    volatile unsigned long u1         = 0;

    for (;;)
    {
        vPrintString(pcTaskName);
        for (u1 = 0; u1 < mainDELAY_LOOP_COUNT; u1++)
        {
            /* Crude delay implementation */
        }
    }
}

/* ========================================================================== */

int main(void)
{
    log_init();
    led_init();

    const char* vTask1Description = "Example Task 1";
    const char* vTask1Parameter   = "Task 1 running";

    if (xTaskCreate(
            vGenericTask,
            vTask1Description,
            configMINIMAL_STACK_SIZE,
            (void*)vTask1Parameter,
            1,
            NULL)
        == pdTRUE)
    {
        log_info("Task 1 created successfully");
    }

    const char* vTask2Description = "Example Task 2";
    const char* vTask2Parameter   = "Task 2 running";

    if (xTaskCreate(
            vGenericTask,
            vTask2Description,
            configMINIMAL_STACK_SIZE,
            (void*)vTask2Parameter,
            1,
            NULL)
        == pdTRUE)
    {
        log_info("Task 2 created successfully");
    }

    led_on();

    /* Start the scheduler.
     * This function NEVER returns if everything is OK.
     * From here on, FreeRTOS takes control of the CPU. */
    vTaskStartScheduler();

    log_err("Could not initialize scheduler");

    /* The application should never reach here */
    return 0;
}

/* ========================================================================== */
