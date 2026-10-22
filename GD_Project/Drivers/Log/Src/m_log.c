#include "gd32e23x.h"
#include "board.h"
#include "m_log.h"

void m_log_init(uint32_t baudrate)
{
    dma_parameter_struct dma_config;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_DMA);
    rcu_usart_clock_config(RCU_USART0SRC_CKAPB2);

    gpio_af_set(M_LOG_GPIO_PORT, M_LOG_GPIO_AF, M_LOG_GPIO_PIN);
    gpio_mode_set(M_LOG_GPIO_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_PULLUP,
                  M_LOG_GPIO_PIN);
    gpio_output_options_set(M_LOG_GPIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            M_LOG_GPIO_PIN);

    usart_deinit(M_LOG_UART_PERIPH);
    usart_baudrate_set(M_LOG_UART_PERIPH, baudrate);
    usart_word_length_set(M_LOG_UART_PERIPH, USART_WL_8BIT);
    usart_stop_bit_set(M_LOG_UART_PERIPH, USART_STB_1BIT);
    usart_parity_config(M_LOG_UART_PERIPH, USART_PM_NONE);
    usart_receive_config(M_LOG_UART_PERIPH, USART_RECEIVE_DISABLE);
    usart_transmit_config(M_LOG_UART_PERIPH, USART_TRANSMIT_ENABLE);

    dma_deinit(M_LOG_TX_DMA_CHANNEL);
    dma_struct_para_init(&dma_config);
    dma_config.periph_addr = (uint32_t)&USART_TDATA(M_LOG_UART_PERIPH);
    dma_config.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_config.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_config.memory_addr = 0U;
    dma_config.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_config.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_config.number = 0U;
    dma_config.priority = DMA_PRIORITY_HIGH;
    dma_config.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(M_LOG_TX_DMA_CHANNEL, &dma_config);

    usart_dma_transmit_config(M_LOG_UART_PERIPH, USART_DENT_ENABLE);
    usart_enable(M_LOG_UART_PERIPH);
}

void m_log_write(const uint8_t *data, uint16_t length)
{
    if ((data == 0) || (length == 0U)) {
        return;
    }

    dma_channel_disable(M_LOG_TX_DMA_CHANNEL);
    dma_flag_clear(M_LOG_TX_DMA_CHANNEL, DMA_FLAG_G);
    dma_memory_address_config(M_LOG_TX_DMA_CHANNEL, (uint32_t)data);
    dma_transfer_number_config(M_LOG_TX_DMA_CHANNEL, length);
    dma_channel_enable(M_LOG_TX_DMA_CHANNEL);

    while (dma_flag_get(M_LOG_TX_DMA_CHANNEL, DMA_FLAG_FTF) == RESET) {
    }

    dma_channel_disable(M_LOG_TX_DMA_CHANNEL);
    dma_flag_clear(M_LOG_TX_DMA_CHANNEL, DMA_FLAG_G);

    while (usart_flag_get(M_LOG_UART_PERIPH, USART_FLAG_TC) == RESET) {
    }
}
