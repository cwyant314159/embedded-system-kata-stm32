#ifndef UART_H
#define UART_H

#include "stm32f1xx.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(USART_TypeDef* p_uart);
bool_t uart_data_available(USART_TypeDef* p_uart);
u8_t uart_read(USART_TypeDef* p_uart);
bool_t uart_write(USART_TypeDef* p_uart, u8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */