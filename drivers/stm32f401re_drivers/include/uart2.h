/*
 * STM32F401RE Simple Drivers
 * USART2 (VCP via ST-Link): PA2 (TX), PA3 (RX)
 */

#ifndef UART2_H
#define UART2_H

#include <stdint.h>

void uart_init(void);

void uart_send(const char* data);

#endif /* UART2_H */
