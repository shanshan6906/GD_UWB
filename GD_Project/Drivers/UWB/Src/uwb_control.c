#include <stdint.h>

#include "uwb_app_config.h"
#include "uwb_control.h"
#include "uwb_uart.h"

#define UWB_COMMAND_BUFFER_SIZE  160U

static uint8_t command_append_char(uint8_t *buffer, uint16_t *length, uint8_t value)
{
    if (*length >= UWB_COMMAND_BUFFER_SIZE) {
        return 0U;
    }

    buffer[*length] = value;
    ++(*length);
    return 1U;
}

static uint8_t command_append_u32(uint8_t *buffer, uint16_t *length, uint32_t value)
{
    uint8_t digits[10];
    uint8_t count = 0U;

    do {
        digits[count] = (uint8_t)('0' + (value % 10U));
        value /= 10U;
        ++count;
    } while ((value != 0U) && (count < sizeof(digits)));

    while (count > 0U) {
        --count;
        if (command_append_char(buffer, length, digits[count]) == 0U) {
            return 0U;
        }
    }

    return 1U;
}

uint8_t uwb_control_send_configuration(void)
{
    uint8_t command[UWB_COMMAND_BUFFER_SIZE];
    uint16_t length = 0U;
    uint32_t sensing_interval_ms;
    uint32_t frame_interval_ms;
    uint32_t cir_per_frame;
    uint32_t deep_sleep_enable;
    const uint32_t parameters[] = {
        UWB_MODE_PRF,
        UWB_POWER_CODE,
        UWB_SCALE_BIT,
        UWB_GAIN_INDEX,
        UWB_CIR_TAP_START,
        UWB_CIR_TAP_END,
        UWB_SENSING_MODE_SELECT,
        UWB_SENSING_BURST_NUM,
        0U, /* sensing_interval_ms: filled below */
        0U, /* frame_interval_ms: filled below */
        0U, /* num_cir_per_frame: filled below */
        0U, /* deep_sleep_en: filled below */
        UWB_NUM_RX_MODE,
        UWB_PREAMBLE_INDEX
    };
    uint32_t resolved_parameters[sizeof(parameters) / sizeof(parameters[0])];
    uint8_t index;

#if (UWB_SENSING_MODE_SELECT == UWB_SENSING_MODE_CONTINUOUS)
    sensing_interval_ms = UWB_CONTINUOUS_SENSING_INTERVAL_MS;
    frame_interval_ms = UWB_FRAME_INTERVAL_MS;
    cir_per_frame = 1U;
    deep_sleep_enable = 0U;
#else
    sensing_interval_ms = UWB_FRAME_ROW_INTERVAL_MS;
    frame_interval_ms = UWB_FRAME_INTERVAL_MS;
    cir_per_frame = UWB_FRAME_CIR_COUNT;
    deep_sleep_enable = UWB_FRAME_DEEP_SLEEP_ENABLE;
#endif

    for (index = 0U; index < (sizeof(parameters) / sizeof(parameters[0])); ++index) {
        resolved_parameters[index] = parameters[index];
    }
    resolved_parameters[8] = sensing_interval_ms;
    resolved_parameters[9] = frame_interval_ms;
    resolved_parameters[10] = cir_per_frame;
    resolved_parameters[11] = deep_sleep_enable;

    if (command_append_char(command, &length, (uint8_t)'f') == 0U) {
        return 0U;
    }

    for (index = 0U; index < (sizeof(resolved_parameters) / sizeof(resolved_parameters[0])); ++index) {
        if ((command_append_char(command, &length, (uint8_t)',') == 0U) ||
            (command_append_u32(command, &length, resolved_parameters[index]) == 0U)) {
            return 0U;
        }
    }

    if (command_append_char(command, &length, (uint8_t)'\r') == 0U) {
        return 0U;
    }

    uwb_uart_write(command, length);
    return 1U;
}
