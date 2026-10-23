#ifndef UWB_APP_CONFIG_H
#define UWB_APP_CONFIG_H

/*
 * UWB <-> GD32 serial port settings.
 * Both sides must use the same baud rate, 8 data bits, no parity and 1 stop bit.
 */
#define UWB_UART_BAUDRATE                         115200U

/* Wait for the UWB firmware to finish booting before sending its configuration. */
#define UWB_STARTUP_DELAY_MS                      200U

/*
 * M_LOG is an independent output-only debug UART on PA9.
 * It uses the same 115200 baud rate requested for the complete serial path.
 * Keep enough time between CIR frames because the output has no rate margin.
 */
#define M_LOG_UART_BAUDRATE                       115200U

/* Select exactly one sensing mode here. */
#define UWB_SENSING_MODE_CONTINUOUS               0U
#define UWB_SENSING_MODE_FRAME                    1U
#define UWB_SENSING_MODE_SELECT                   UWB_SENSING_MODE_CONTINUOUS

/* Common parameters used by both sensing modes. */
#define UWB_MODE_PRF                              0U   /* 0=BPRF, 1=HPRF, 2=LG4A */
#define UWB_POWER_CODE                            7U   /* UWB transmit power code */
#define UWB_SCALE_BIT                             6U   /* CIR amplitude scale setting */
#define UWB_GAIN_INDEX                            5U   /* Receiver gain index */
#define UWB_CIR_TAP_START                         0U   /* First CIR tap, inclusive */
#define UWB_CIR_TAP_END                           100U /* Last CIR tap, inclusive; maximum 240 */
#define UWB_SENSING_BURST_NUM                     1U   /* Bursts averaged into one CIR */
#define UWB_NUM_RX_MODE                           2U   /* 1=1T1R, 2=1T2R */
#define UWB_PREAMBLE_INDEX                        10U  /* BPRF uses 9..24 */

/* sensing_mode = 0: UWB continuously pushes one CIR at this interval. */
#define UWB_CONTINUOUS_SENSING_INTERVAL_MS        100U

/*
 * sensing_mode = 1: one frame contains multiple CIR rows.
 * Rows are acquired at UWB_FRAME_ROW_INTERVAL_MS. The complete frame repeats
 * at UWB_FRAME_INTERVAL_MS. Deep sleep is available only in frame mode.
 */
#define UWB_FRAME_ROW_INTERVAL_MS                 10U
#define UWB_FRAME_INTERVAL_MS                     100U
#define UWB_FRAME_CIR_COUNT                       32U
#define UWB_FRAME_DEEP_SLEEP_ENABLE               0U

#if ((UWB_SENSING_MODE_SELECT != UWB_SENSING_MODE_CONTINUOUS) && \
     (UWB_SENSING_MODE_SELECT != UWB_SENSING_MODE_FRAME))
#error "UWB_SENSING_MODE_SELECT must be CONTINUOUS or FRAME"
#endif

#if (UWB_CIR_TAP_START > UWB_CIR_TAP_END) || (UWB_CIR_TAP_END > 240U)
#error "Invalid UWB CIR tap range"
#endif

#if ((UWB_NUM_RX_MODE != 1U) && (UWB_NUM_RX_MODE != 2U))
#error "UWB_NUM_RX_MODE must be 1 (1T1R) or 2 (1T2R)"
#endif

#endif /* UWB_APP_CONFIG_H */
