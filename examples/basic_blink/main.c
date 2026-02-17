/*
 * FreeRTOS STM32F4 Template
 *
 * Basic example with two tasks that blink the LED.
 *
 * Nucleo-F401RE onboard LED: PA5
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "led.h"
#include "task.h"

/* ========================================================================== */

void vTask1(void* pvParameters)
{
    (void)pvParameters; /* Avoid unused parameter warning */

    for (;;) /* Tasks must never return */
    {
        led_toggle();

        /* Block the task for 500ms.
         * During this time, other tasks can run.
         * pdMS_TO_TICKS converts milliseconds to RTOS ticks. */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ========================================================================== */

/* In this application there is only one task. This task runs and then it blocks
 * itself for 500 milliseconds, then it runs again and so on, because it is the
 * only task on the app. */

int main(void)
{
    led_init();

    xTaskCreate(vTask1, "LED_Blink", 128, NULL, 1, NULL);

    vTaskStartScheduler();

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
