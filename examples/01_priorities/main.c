/*
 * FreeRTOS STM32F4 Template
 *
 * Example: Two tasks with different priorities
 */

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

/*-----------------------------------------------------------
 * LED Configuration (PA5 on Nucleo-F401RE)
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
 * Task 1: Slow blink (Priority 1)
 *
 * Turns LED ON for 1 second, then OFF for 1 second
 *----------------------------------------------------------*/
void vTaskSlowBlink(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(1000));  // ON for 1 second

        led_off();
        vTaskDelay(pdMS_TO_TICKS(1000));  // OFF for 1 second
    }
}

/*-----------------------------------------------------------
 * Task 2: Fast blink burst (Priority 2 - HIGHER)
 *
 * Every 5 seconds, does a fast blink burst
 * Because it has higher priority, it "interrupts" Task 1
 *----------------------------------------------------------*/
void vTaskFastBurst(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        // Wait 5 seconds (during this time, Task 1 runs)
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Fast burst (10 quick blinks)
        for (int i = 0; i < 10; i++) {
            led_on();
            vTaskDelay(pdMS_TO_TICKS(50));
            led_off();
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

/*-----------------------------------------------------------
 * Main
 *----------------------------------------------------------*/
int main(void) {
    led_init();

    // Task 1: Lower priority (1)
    xTaskCreate(vTaskSlowBlink, "SlowBlink", 128, NULL, 1, NULL);

    // Task 2: Higher priority (2)
    xTaskCreate(vTaskFastBurst, "FastBurst", 128, NULL, 2, NULL);

    // Start scheduler
    vTaskStartScheduler();

    // Error: should never reach here
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
