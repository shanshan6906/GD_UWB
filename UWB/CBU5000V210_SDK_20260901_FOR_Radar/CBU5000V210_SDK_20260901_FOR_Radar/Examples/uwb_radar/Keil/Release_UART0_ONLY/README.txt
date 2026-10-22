CBU5000V210 UWB Radar - UART0 only build

Build time: 2026-10-22
Build result: completed successfully; AXF and HEX generated

UART configuration:
- UART0 enabled, 921600 baud, 8 data bits, no parity, 1 stop bit
- UWB GPIO00 = UART0_TXD -> GD32 PA3 / USART0_RX
- UWB GPIO01 = UART0_RXD <- GD32 PA2 / USART0_TX
- UART1 initialization and IRQ priority configuration disabled

Files:
- CBU5000V210_uwb_radar_UART0_ONLY.hex: download image
- CBU5000V210_uwb_radar_UART0_ONLY.axf: debug image with symbols

Notes:
- UART commands and UWB data now share UART0 at 921600 baud.
- Commands are terminated by carriage return (0x0D, '\r').
- The vendor email GPIO description is not used; schematic and source code are authoritative.
