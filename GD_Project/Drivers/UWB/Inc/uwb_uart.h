#ifndef UWB_UART_H
#define UWB_UART_H

#include <stdint.h>

void uwb_uart_init(uint32_t baudrate);
void uwb_uart_set_baudrate(uint32_t baudrate);
void uwb_uart_write(const uint8_t *data, uint16_t length);
uint16_t uwb_uart_read(uint8_t *data, uint16_t capacity);
uint16_t uwb_uart_available(void);
uint32_t uwb_uart_overflow_count(void);
void uwb_uart_irq_handler(void);

#endif /* UWB_UART_H */
