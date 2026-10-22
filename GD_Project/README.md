# GD32E230K8U6 UWB控制工程

本工程用于GD32E230K8U6与CBU5000V210 UWB雷达固件联合调试。

## 硬件连接与串口分配

| GD32E230K8U6 | GD外设 | 方向 | UWB/测试点 | 用途 |
|---|---|---|---|---|
| PA1 | EXTI1 | 输入 | UWB INT | 预留UWB中断信号 |
| PA2 | USART1_TX | 输出 | UWB GPIO01/UART0_RX | 向UWB发送配置命令 |
| PA3 | USART1_RX | 输入 | UWB GPIO00/UART0_TX | 接收UWB全部串口数据 |
| PA9 | USART0_TX | 输出 | M_LOG | 原样转发UWB串口数据 |

- UWB通信串口：115200、8N1、无流控。
- M_LOG输出串口：115200、8N1、无流控。
- USART1_RX使用DMA通道4循环接收。
- USART0_TX使用DMA通道1分块发送。

## 运行过程

1. 初始化板级资源、M_LOG和UWB通信串口。
2. 等待UWB启动，默认等待200ms。
3. 等待期间收到的UWB启动信息同样原样转发到M_LOG。
4. 根据宏配置生成`f,<14个参数>\r`命令并发送一次。
5. 持续将UWB串口收到的所有字节不加前缀、不解析、不改写地输出到M_LOG。

所有可调参数位于`Application/Inc/uwb_app_config.h`。修改
`UWB_SENSING_MODE_SELECT`即可选择连续感知或基于帧的感知。

## 编译

1. 打开`Project/Keil/GD32E230K8U6_UWB.uvprojx`。
2. 选择`GD32E230K8U6_UWB`目标并执行Build。
3. AXF和HEX文件生成在`Project/Keil/Objects`目录。

也可以在PowerShell中运行`./build_keil.ps1`进行完整重编译。

详细设计、参数含义和数据带宽限制见`程序逻辑设计说明.md`。
