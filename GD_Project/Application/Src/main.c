#include <stdint.h>

#include "gd32e23x.h"
#include "board.h"
#include "m_log.h"
#include "uwb_app_config.h"
#include "uwb_control.h"
#include "uwb_uart.h"

static void forward_uwb_data_to_log(void)
{
    uint8_t buffer[64];
    uint16_t received_length;

    received_length = uwb_uart_read(buffer, sizeof(buffer));
    if (received_length > 0U) {
        m_log_write(buffer, received_length);
    }
}

int main(void)
{
    uint32_t startup_time_ms;

    board_init();
    m_log_init(M_LOG_UART_BAUDRATE);
    uwb_uart_init(UWB_UART_BAUDRATE);

    startup_time_ms = board_millis();
    while ((uint32_t)(board_millis() - startup_time_ms) < UWB_STARTUP_DELAY_MS) {
        /* Forward the UWB boot messages while waiting for initialization. */
        forward_uwb_data_to_log();
    }

    (void)uwb_control_send_configuration();

    while (1) {
        forward_uwb_data_to_log();

        if (board_uwb_int_take() != 0U) {
            /* Reserved for future UWB interrupt-driven functions. */
        }
    }
}
