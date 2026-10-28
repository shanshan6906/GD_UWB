CBU5000V210 UWB固件下载文件说明

本目录中的固件已按当前原理图修改：
- 仅使用UWB UART0，UART1已屏蔽。
- GPIO00配置为UART0_TX，连接GD PA3/USART1_RX。
- GPIO01配置为UART0_RX，连接GD PA2/USART1_TX。
- UART0波特率为115200 baud。

请配套使用GD工程Release目录下名称含“115200”的下载文件。
旧目录Release_UART0_ONLY中的文件为早期版本，不要与本版本混用。
