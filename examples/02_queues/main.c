/*
 * FreeRTOS Example: Queues
 *
 * Demonstrates task communication using queues.
 *
 * Producer task: Sends LED patterns to a queue
 * Consumer task: Receives patterns and blinks accordingly
 *
 * This decouples "what to do" from "how to do it"
 */

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"
#include "queue.h"

/*-----------------------------------------------------------
 * LED Configuration
 *----------------------------------------------------------*/

#define LED_PORT GPIOA
#define LED_PIN 5

static void led_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    LED_PORT->MODER &= ~(3U << (LED_PIN * 2));
    LED_PORT->MODER |= (1U << (LED_PIN * 2));
    LED_PORT->OTYPER &= ~(1U << LED_PIN);
    LED_PORT->OSPEEDR &= ~(3U << (LED_PIN * 2));
    LED_PORT->PUPDR &= ~(3U << (LED_PIN * 2));
}

static void led_on(void)  { LED_PORT->BSRR = (1U << LED_PIN); }
static void led_off(void) { LED_PORT->BSRR = (1U << (LED_PIN + 16)); }

/*-----------------------------------------------------------
 * Message types for the queue
 *----------------------------------------------------------*/

typedef enum {
    LED_CMD_OFF,
    LED_CMD_ON,
    LED_CMD_BLINK_SLOW,
    LED_CMD_BLINK_FAST
} LedCommand_t;

/* Global queue handle */
QueueHandle_t xLedQueue;

/*-----------------------------------------------------------
 * Producer Task: Sends commands to the queue
 *
 * Cycles through different LED patterns
 *----------------------------------------------------------*/
void vProducerTask(void *pvParameters) {
    (void)pvParameters;
    LedCommand_t cmd;

    for (;;) {
        /* Send: LED ON for 2 seconds */
        cmd = LED_CMD_ON;
        xQueueSend(xLedQueue, &cmd, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Send: Blink slow for 3 seconds */
        cmd = LED_CMD_BLINK_SLOW;
        xQueueSend(xLedQueue, &cmd, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(3000));

        /* Send: Blink fast for 2 seconds */
        cmd = LED_CMD_BLINK_FAST;
        xQueueSend(xLedQueue, &cmd, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Send: LED OFF for 1 second */
        cmd = LED_CMD_OFF;
        xQueueSend(xLedQueue, &cmd, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*-----------------------------------------------------------
 * Consumer Task: Receives commands and executes them
 *
 * Blocks on queue until a command arrives
 *----------------------------------------------------------*/
void vConsumerTask(void *pvParameters) {
    (void)pvParameters;
    LedCommand_t cmd;
    TickType_t blinkDelay = 0;

    for (;;) {
        /*
         * Check for new command (non-blocking with 100ms timeout)
         * This allows us to keep blinking while waiting for new commands
         */
        if (xQueueReceive(xLedQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
            /* New command received - update behavior */
            switch (cmd) {
                case LED_CMD_OFF:
                    led_off();
                    blinkDelay = 0;  /* Stop blinking */
                    break;

                case LED_CMD_ON:
                    led_on();
                    blinkDelay = 0;  /* Stop blinking */
                    break;

                case LED_CMD_BLINK_SLOW:
                    blinkDelay = pdMS_TO_TICKS(500);
                    break;

                case LED_CMD_BLINK_FAST:
                    blinkDelay = pdMS_TO_TICKS(100);
                    break;
            }
        }

        /* If blinking mode is active, toggle LED */
        if (blinkDelay > 0) {
            led_on();
            vTaskDelay(blinkDelay);
            led_off();
            vTaskDelay(blinkDelay);
        }
    }
}

/*-----------------------------------------------------------
 * Main
 *----------------------------------------------------------*/
int main(void) {
    led_init();

    /*
     * Create the queue
     *
     * xQueueCreate(length, item_size)
     *   - length: how many items the queue can hold
     *   - item_size: size of each item in bytes
     *
     * Returns NULL if not enough memory
     */
    xLedQueue = xQueueCreate(5, sizeof(LedCommand_t));

    if (xLedQueue == NULL) {
        /* Queue creation failed - blink error */
        for (;;) {
            led_on();
            for (volatile int i = 0; i < 50000; i++);
            led_off();
            for (volatile int i = 0; i < 50000; i++);
        }
    }

    /* Create tasks */
    xTaskCreate(vProducerTask, "Producer", 128, NULL, 1, NULL);
    xTaskCreate(vConsumerTask, "Consumer", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;);
    return 0;
}

/*-----------------------------------------------------------
 * System init
 *----------------------------------------------------------*/
void SystemInit(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
#endif
}
