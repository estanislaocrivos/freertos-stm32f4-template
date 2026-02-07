/**
 * @file main.c
 * @brief Main application entry point
 */

/* ========================================================================== */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"

/* ========================================================================== */

#define STACK_SIZE configMINIMAL_STACK_SIZE

/* ========================================================================== */

static void prvMainTask(void* pvParameters);

/* ========================================================================== */

int main(void)
{
    /* Create main task */
    xTaskCreate(prvMainTask, "Main", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,
                NULL);

    /* Start scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    for (;;)
    {
    }
}

/* ========================================================================== */

static void prvMainTask(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* TODO: Add your application code here */

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ========================================================================== */
