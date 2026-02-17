/*
 * FreeRTOS STM32F4 Template
 *
 * Binary Semaphore Example
 *
 * This example demonstrates task synchronization using a binary semaphore.
 * - vHandlerTask: waits (blocked) for the semaphore, then processes the "event"
 * - vPeriodicTask: simulates an event by giving the semaphore every 3 seconds
 *
 * In real applications, the semaphore would typically be given from an ISR
 * (e.g., button press, UART RX, timer) to wake up a handler task.
 *
 * USART2 (VCP via ST-Link): PA2 (TX), PA3 (RX)
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "led.h"
#include "logging.h"
#include "semphr.h"
#include "task.h"

/* ========================================================================== */

/* Global semaphore handle */
static xSemaphoreHandle xBinarySemaphore = NULL;

/* ========================================================================== */

void vPrintString(const char* str)
{
    taskENTER_CRITICAL();
    log_info(str);
    taskEXIT_CRITICAL();
}

/* ========================================================================== */

/*
 * Handler task: high priority, waits blocked for the semaphore.
 * When the semaphore is given, it "handles" the event and goes back to waiting.
 */
void vHandlerTask(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* Block indefinitely waiting for the semaphore */
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            vPrintString("Handler: semaphore taken, processing event...");
            led_toggle();
        }
    }
}

/*
 * Periodic task: lower priority, simulates an external event by giving the
 * semaphore every 3 seconds.
 */
void vPeriodicTask(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));
        vPrintString("Periodic: giving semaphore (simulating event)");
        xSemaphoreGive(xBinarySemaphore);
    }
}

/* ========================================================================== */

int main(void)
{
    log_init();
    led_init();

    /* Create the binary semaphore */
    vSemaphoreCreateBinary(xBinarySemaphore);

    if (xBinarySemaphore == NULL)
    {
        log_err("Could not create the binary semaphore");
        return -1;
    }

    /*
     * Important: take the semaphore once before starting.
     * vSemaphoreCreateBinary creates it in the "given" state, so we take it to
     * ensure the handler starts blocked.
     */
    xSemaphoreTake(xBinarySemaphore, 0);

    /* Handler task: higher priority (2), will preempt when semaphore is given
     */
    xTaskCreate(vHandlerTask, "Handler", 128, NULL, 2, NULL);

    /* Periodic task: lower priority (1), runs when handler is blocked */
    xTaskCreate(vPeriodicTask, "Periodic", 128, NULL, 1, NULL);

    vPrintString("Starting scheduler...");
    led_on();
    vTaskStartScheduler();

    /* Should never reach here */
    log_err("Scheduler failed to start");

    for (;;)
    {
        led_toggle();
        for (volatile int i = 0; i < 100000; i++)
        {
        }
    }

    return 0;
}

/* ========================================================================== */
