#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

/* Connections confirmed from the current schematic. */
#define UWB_INT_GPIO_PORT       GPIOA
#define UWB_INT_GPIO_PIN        GPIO_PIN_1

#define UWB_UART_PERIPH         USART1
#define UWB_UART_RCU            RCU_USART1
#define UWB_UART_IRQn           USART1_IRQn
#define UWB_UART_TX_GPIO_PORT   GPIOA
#define UWB_UART_TX_GPIO_PIN    GPIO_PIN_2
#define UWB_UART_RX_GPIO_PORT   GPIOA
#define UWB_UART_RX_GPIO_PIN    GPIO_PIN_3
#define UWB_UART_GPIO_AF        GPIO_AF_1
#define UWB_UART_RX_DMA_CHANNEL DMA_CH4

#define M_LOG_UART_PERIPH       USART0
#define M_LOG_GPIO_PORT         GPIOA
#define M_LOG_GPIO_PIN          GPIO_PIN_9
#define M_LOG_GPIO_AF           GPIO_AF_1
#define M_LOG_TX_DMA_CHANNEL    DMA_CH1

void board_init(void);
uint32_t board_millis(void);
void board_delay_ms(uint32_t delay_ms);
void board_systick_irq_handler(void);
uint8_t board_uwb_int_level(void);
uint8_t board_uwb_int_take(void);
void board_uwb_int_irq_handler(void);

#endif /* BOARD_H */
