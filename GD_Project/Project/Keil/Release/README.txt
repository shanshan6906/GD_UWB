GD32E230K8U6 UWB固件下载文件说明

1. GD32E230K8U6_UWB_CONTINUOUS_115200.hex
   连续感知模式（sensing_mode=0），当前源代码默认配置。

2. GD32E230K8U6_UWB_FRAME_115200.hex
   基于帧的感知模式（sensing_mode=1）。

硬件接口：
- PA2 / USART1_TX：向UWB GPIO01/UART0_RX发送命令
- PA3 / USART1_RX：接收UWB GPIO00/UART0_TX数据
- PA9 / USART0_TX：M_LOG原样输出

两路串口均为115200 baud、8N1、无流控。
USART1_RX使用DMA通道4循环接收，USART0_TX使用DMA通道1发送；
两级DMA之间经过GD32的RAM缓冲区，并非外设到外设的直接DMA连接。

