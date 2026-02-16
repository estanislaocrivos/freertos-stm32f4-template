#include "led.h"

#include "stm32f4xx.h"

/* ========================================================================== */

#define LED_PORT GPIOA
#define LED_PIN  5

/* ========================================================================== */

void led_init(void)
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

void led_toggle(void)
{
    LED_PORT->ODR ^= (1U << LED_PIN);
}

void led_on(void)
{
    LED_PORT->BSRR = (1U << LED_PIN);
}

void led_off(void)
{
    LED_PORT->BSRR = (1U << (LED_PIN + 16)); /* Reset bit */
}

/* ========================================================================== */
