SET PATH=C:\Keil_v5\ARM\ARMCLANG\Bin;C:\Program Files (x86)\VMware\VMware Workstation\bin\;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files\dotnet\;C:\;C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility;C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\;C:\Program Files (x86)\Windows Kits\8.1\Windows Performance Toolkit\;C:\Users\Niu\AppData\Local\Programs\Python\Python312\Scripts\;C:\Users\Niu\AppData\Local\Programs\Python\Python312\;C:\Users\Niu\AppData\Local\Programs\Python\Launcher\;C:\Users\Niu\AppData\Local\Microsoft\WindowsApps;C:\Program Files\JetBrains\PyCharm 2025.1.3.1\bin;C:\Users\Niu\AppData\Local\Programs\Microsoft VS Code\bin;C:\Program Files\lceda-pro;;D:\Program Files\lceda-pro;C:\Users\Niu\AppData\Local\Programs\Python\Python312\Scripts\;C:\Users\Niu\AppData\Local\Programs\Python\Python312\;C:\Users\Niu\AppData\Local\Programs\Python\Launcher\;C:\Users\Niu\AppData\Local\Microsoft\WindowsApps;;C:\Program Files\JetBrains\PyCharm 2025.1.3.1\bin;;C:\Users\Niu\AppData\Local\Programs\Microsoft VS Code\bin
SET CPU_TYPE=GD32E230K8
SET CPU_VENDOR=GigaDevice
SET UV2_TARGET=GD32E230K8U6_UWB
SET CPU_CLOCK=0x044AA200
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\main.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_it.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\board.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\uwb_uart.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\uwb_control.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\m_log.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\startup_gd32e23x.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\system_gd32e23x.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_dma.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_exti.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_gpio.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_misc.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_rcu.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_syscfg.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @".\objects\gd32e23x_usart.__i"
"C:\Keil_v5\ARM\ARMCLANG\Bin\ArmLink" --Via ".\Objects\GD32E230K8U6_UWB.lnp"
"C:\Keil_v5\ARM\ARMCLANG\Bin\fromelf.exe" ".\Objects\GD32E230K8U6_UWB.axf" --i32combined --output ".\Objects\GD32E230K8U6_UWB.hex"
