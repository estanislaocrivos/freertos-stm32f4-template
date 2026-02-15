/*
 * FreeRTOS STM32F4 Template
 *
 * Basic example with two tasks that blink the LED.
 * Nucleo-F401RE onboard LED: PA5
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

/* ========================================================================== */

#define LED_PORT GPIOA
#define LED_PIN  5

static void led_init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Configure PA5 as output */
    LED_PORT->MODER &= ~(3U << (LED_PIN * 2)); /* Clear bits */
    LED_PORT->MODER |= (1U << (LED_PIN * 2));  /* 01 = Output */

    /* Push-pull, no pull-up/down, low speed (enough for LED) */
    LED_PORT->OTYPER &= ~(1U << LED_PIN);
    LED_PORT->OSPEEDR &= ~(3U << (LED_PIN * 2));
    LED_PORT->PUPDR &= ~(3U << (LED_PIN * 2));
}

static void led_toggle(void)
{
    LED_PORT->ODR ^= (1U << LED_PIN);
}

static void led_on(void)
{
    LED_PORT->BSRR = (1U << LED_PIN);
}

static void led_off(void)
{
    LED_PORT->BSRR = (1U << (LED_PIN + 16)); /* Reset bit */
}

/* ========================================================================== */

/*
 * Task 1: Toggle LED every 500ms
 *
 * pvParameters allows passing data to the task when creating it.
 * In this case we don't use it.
 */
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

/*
 * Task 2: Fast blink (100ms) for 1 second, then pause 2 seconds
 *
 * This demonstrates that both tasks run "simultaneously".
 * In reality the scheduler alternates them very quickly.
 */
void vTask2(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* Fast blink for 1 second (10 toggles) */
        for (int i = 0; i < 10; i++)
        {
            led_toggle();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        /* 2 second pause with LED off */
        led_off();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ========================================================================== */

int main(void)
{
    /* Initialize hardware */
    led_init();

    /* Create tasks
     *
     * xTaskCreate(
     *     function,          - Pointer to the task function
     *     "name",            - Name for debugging
     *     stack_size,        - Stack size in words
     *     parameters,        - Data to pass to the task (or NULL)
     *     priority,          - 0 = lowest, configMAX_PRIORITIES-1 = highest
     *     &handle            - Handle to control the task (or NULL)
     * );
     */

    /* Only creating Task1 for now.
     * Uncomment Task2 to see how they interact. */
    xTaskCreate(vTask1, "LED_Blink", 128, NULL, 1, NULL);
    /* xTaskCreate(vTask2, "LED_Fast", 128, NULL, 1, NULL); */

    /* Start the scheduler.
     * This function NEVER returns if everything is OK.
     * From here on, FreeRTOS takes control of the CPU. */
    vTaskStartScheduler();

    /* If we get here, there was an error (e.g., no memory for idle task) */
    for (;;)
    {
        /* Very fast error blink */
        led_toggle();
        for (volatile int i = 0; i < 100000; i++)
            ;
    }

    return 0; /* Never reached */
}

/* ========================================================================== */
