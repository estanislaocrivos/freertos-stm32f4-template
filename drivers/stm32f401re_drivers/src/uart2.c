#include "uart2.h"

#include "stm32f4xx.h"

/* ========================================================================== */

#define UART_BAUD_RATE 115200U

/* ========================================================================== */

void uart_init(void)
{
    /* Enable clocks for GPIOA and USART2 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Configure PA2 (TX) and PA3 (RX) as alternate function */
    /* MODER: 10 = Alternate function mode */
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |= (0x2U << GPIO_MODER_MODER2_Pos)
                    | (0x2U << GPIO_MODER_MODER3_Pos);

    /* OSPEEDR: 11 = High speed */
    GPIOA->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED2_Pos)
                      | (0x3U << GPIO_OSPEEDR_OSPEED3_Pos);

    /* OTYPER: 0 = Push-pull (default) */
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);

    /* PUPDR: 01 = Pull-up */
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
    GPIOA->PUPDR |= (0x1U << GPIO_PUPDR_PUPD2_Pos)
                    | (0x1U << GPIO_PUPDR_PUPD3_Pos);

    /* AFR[0] (AFRL): AF7 for PA2 and PA3 (USART2) */
    GPIOA->AFR[0] &= ~((0xFU << (2 * 4)) | (0xFU << (3 * 4)));
    GPIOA->AFR[0] |= (7U << (2 * 4)) | (7U << (3 * 4));

    /* Configure USART2 */
    /* Disable USART before configuration */
    USART2->CR1 = 0;

    /* CR1: 8 data bits (M=0), no parity (PCE=0) */
    /* CR2: 1 stop bit (STOP=00) - default */
    /* CR3: no flow control - default */

    /*
     * Baud rate calculation (oversampling by 16):
     * BRR = fck / baud
     * For HSI = 16 MHz, baud = 115200:
     * BRR = 16000000 / 115200 = 138.89
     * Mantissa = 138 >> 0 = 138 (but stored shifted left by 4)
     * Fraction = 0.89 * 16 = 14.22 ≈ 14
     * BRR = (138 << 4) | 14 = 2222
     *
     * Using SystemCoreClock for flexibility.
     * Note: APB1 clock = SystemCoreClock when APB1 prescaler = 1 (default)
     */
    uint32_t apb1_clock = SystemCoreClock; /* Assuming APB1 prescaler = 1 */
    uint32_t usartdiv   = (apb1_clock + (UART_BAUD_RATE / 2U)) / UART_BAUD_RATE;
    USART2->BRR         = (uint16_t)usartdiv;

    /* Enable TX and RX */
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

    /* Enable USART */
    USART2->CR1 |= USART_CR1_UE;
}

void uart_putc(char c)
{
    /* Wait until TXE (Transmit Data Register Empty) is set */
    while ((USART2->SR & USART_SR_TXE) == 0)
    {
        /* Busy wait */
    }
    USART2->DR = (uint8_t)c;
}

void uart_puts(const char* str)
{
    while (*str != '\0')
    {
        uart_putc(*str);
        str++;
    }
}

void uart_send(const char* str)
{
    while (*str != '\0')
    {
        uart_putc(*str);
        str++;
    }
    uart_puts("\r\n");
}
