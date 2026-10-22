/*
    Copyright (C) 2018 GigaDevice

    2019-03-15, V1.0.0, firmware for GD32E23x
*/

#ifndef GD32E23x_LIBOPT_H
#define GD32E23x_LIBOPT_H

/* Comment the line below to disable peripheral header file inclusion */
#include "RTE_Components.h"

#ifdef RTE_DEVICE_STDPERIPHERALS_ADC
#include "gd32e23x_adc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CMP
#include "gd32e23x_cmp.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CRC
#include "gd32e23x_crc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DBG
#include "gd32e23x_dbg.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DMA
#include "gd32e23x_dma.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_EXTI
#include "gd32e23x_exti.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FMC
#include "gd32e23x_fmc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FWDGT
#include "gd32e23x_fwdgt.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_GPIO
#include "gd32e23x_gpio.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_I2C
#include "gd32e23x_i2c.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_MISC
#include "gd32e23x_misc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_PMU
#include "gd32e23x_pmu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RCU
#include "gd32e23x_rcu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RTC
#include "gd32e23x_rtc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SPI
#include "gd32e23x_spi.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SYSCFG
#include "gd32e23x_syscfg.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_TIMER
#include "gd32e23x_timer.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_USART
#include "gd32e23x_usart.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_WWDGT
#include "gd32e23x_wwdgt.h"
#endif

#endif /* GD32E23x_LIBOPT_H */

