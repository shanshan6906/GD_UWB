#include "gd32e23x.h"
#include "board.h"
#include "uwb_uart.h"

#define UWB_UART_RX_BUFFER_SIZE  2048U
#define UWB_UART_RX_BUFFER_MASK  (UWB_UART_RX_BUFFER_SIZE - 1U)

static uint8_t s_rx_buffer[UWB_UART_RX_BUFFER_SIZE];
static uint16_t s_rx_tail;
static volatile uint32_t s_rx_overflow_count;

static uint16_t uwb_uart_dma_head(void)
{
    uint32_t remaining = dma_transfer_number_get(UWB_UART_RX_DMA_CHANNEL);

    return (uint16_t)((UWB_UART_RX_BUFFER_SIZE - remaining) & UWB_UART_RX_BUFFER_MASK);
}

void uwb_uart_init(uint32_t baudrate)
{
    dma_parameter_struct dma_config;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(UWB_UART_RCU);
    rcu_periph_clock_enable(RCU_DMA);

    gpio_af_set(UWB_UART_TX_GPIO_PORT, UWB_UART_GPIO_AF, UWB_UART_TX_GPIO_PIN);
    gpio_af_set(UWB_UART_RX_GPIO_PORT, UWB_UART_GPIO_AF, UWB_UART_RX_GPIO_PIN);

    gpio_mode_set(UWB_UART_TX_GPIO_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_PULLUP,
                  UWB_UART_TX_GPIO_PIN);
    gpio_output_options_set(UWB_UART_TX_GPIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            UWB_UART_TX_GPIO_PIN);

    gpio_mode_set(UWB_UART_RX_GPIO_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_PULLUP,
                  UWB_UART_RX_GPIO_PIN);

    usart_deinit(UWB_UART_PERIPH);
    usart_baudrate_set(UWB_UART_PERIPH, baudrate);
    usart_word_length_set(UWB_UART_PERIPH, USART_WL_8BIT);
    usart_stop_bit_set(UWB_UART_PERIPH, USART_STB_1BIT);
    usart_parity_config(UWB_UART_PERIPH, USART_PM_NONE);
    usart_receive_config(UWB_UART_PERIPH, USART_RECEIVE_ENABLE);
    usart_transmit_config(UWB_UART_PERIPH, USART_TRANSMIT_ENABLE);

    s_rx_tail = 0U;
    s_rx_overflow_count = 0U;

    dma_deinit(UWB_UART_RX_DMA_CHANNEL);
    dma_struct_para_init(&dma_config);
    dma_config.periph_addr = (uint32_t)&USART_RDATA(UWB_UART_PERIPH);
    dma_config.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_config.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_config.memory_addr = (uint32_t)s_rx_buffer;
    dma_config.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_config.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_config.number = UWB_UART_RX_BUFFER_SIZE;
    dma_config.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_config.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init(UWB_UART_RX_DMA_CHANNEL, &dma_config);
    dma_circulation_enable(UWB_UART_RX_DMA_CHANNEL);

    dma_channel_enable(UWB_UART_RX_DMA_CHANNEL);
    usart_dma_receive_config(UWB_UART_PERIPH, USART_DENR_ENABLE);
    usart_enable(UWB_UART_PERIPH);
}

void uwb_uart_set_baudrate(uint32_t baudrate)
{
    usart_disable(UWB_UART_PERIPH);
    usart_baudrate_set(UWB_UART_PERIPH, baudrate);
    usart_enable(UWB_UART_PERIPH);
}

void uwb_uart_write(const uint8_t *data, uint16_t length)
{
    uint16_t index;

    if (data == 0) {
        return;
    }

    for (index = 0U; index < length; ++index) {
        while (usart_flag_get(UWB_UART_PERIPH, USART_FLAG_TBE) == RESET) {
        }
        usart_data_transmit(UWB_UART_PERIPH, data[index]);
    }

    while (usart_flag_get(UWB_UART_PERIPH, USART_FLAG_TC) == RESET) {
    }
}

uint16_t uwb_uart_read(uint8_t *data, uint16_t capacity)
{
    uint16_t count = 0U;
    uint16_t rx_head;

    if (data == 0) {
        return 0U;
    }

    rx_head = uwb_uart_dma_head();
    while ((count < capacity) && (s_rx_tail != rx_head)) {
        data[count] = s_rx_buffer[s_rx_tail];
        s_rx_tail = (uint16_t)((s_rx_tail + 1U) & UWB_UART_RX_BUFFER_MASK);
        ++count;
    }

    return count;
}

uint16_t uwb_uart_available(void)
{
    return (uint16_t)((uwb_uart_dma_head() - s_rx_tail) & UWB_UART_RX_BUFFER_MASK);
}

uint32_t uwb_uart_overflow_count(void)
{
    return s_rx_overflow_count;
}

void uwb_uart_irq_handler(void)
{
    /* USART1 RX is handled by DMA channel 4; no per-byte interrupt is required. */
}
