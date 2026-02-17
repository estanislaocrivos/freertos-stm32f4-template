/*
 * FreeRTOS STM32F4 Template
 *
 * Basic example on how to create and use queues
 * USART2 (VCP via ST-Link): PA2 (TX), PA3 (RX)
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "led.h"
#include "logging.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "task.h"

#include <stdint.h>
#include <string.h>

/* ========================================================================== */

struct xTaskpvParameters
{
    const char*  description;
    xQueueHandle queue;
};

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
    struct xTaskpvParameters* task1_parameters
        = (struct xTaskpvParameters*)pvParameters;
    uint8_t value_to_send = 1;

    for (;;)
    {
        vPrintString(task1_parameters->description);
        portBASE_TYPE xStatus
            = xQueueSendToBack(task1_parameters->queue, &value_to_send, 0);
        if (xStatus != pdPASS)
        {
            log_err("Could not send data to the queue from Task 1");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTask2(void* pvParameters)
{
    struct xTaskpvParameters* task1_parameters
        = (struct xTaskpvParameters*)pvParameters;
    uint8_t received_value = 0;

    for (;;)
    {
        vPrintString(task1_parameters->description);
        portBASE_TYPE xStatus = xQueueReceive(
            task1_parameters->queue, &received_value, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            vPrintString("Received data on Task 2 from Task 1");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/* ========================================================================== */

int main(void)
{
    log_init();
    led_init();

    xQueueHandle xQueue = xQueueCreate(10, sizeof(uint8_t));

    if (xQueue == NULL)
    {
        log_err("Could not create the queue");
        return -1;
    }

    static struct xTaskpvParameters task1_parameters;
    task1_parameters.description = "Task 1 running";
    task1_parameters.queue       = xQueue;

    static struct xTaskpvParameters task2_parameters;
    task2_parameters.description = "Task 2 running";
    task2_parameters.queue       = xQueue;

    xTaskCreate(vTask1, "Task 1", 128, (void*)(&task1_parameters), 1, NULL);

    xTaskCreate(vTask2, "Task 2", 128, (void*)(&task2_parameters), 2, NULL);

    led_on();
    vTaskStartScheduler();

    /* Should never reach here */
    log_err("Scheduler failed to start");

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
