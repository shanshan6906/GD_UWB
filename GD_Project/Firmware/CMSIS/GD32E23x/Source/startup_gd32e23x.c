/*
 * C startup file for GD32E23x and Arm Compiler 6.
 * The interrupt order follows GigaDevice startup_gd32e23x.s.
 */
#include "gd32e23x.h"

extern uint32_t __INITIAL_SP;
extern __NO_RETURN void __PROGRAM_START(void);

typedef void (*VECTOR_TABLE_Type)(void);

__NO_RETURN void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_DEFAULT_HANDLER(name) \
    void name(void) __attribute__((weak, alias("Default_Handler")))

WEAK_DEFAULT_HANDLER(NMI_Handler);
WEAK_DEFAULT_HANDLER(HardFault_Handler);
WEAK_DEFAULT_HANDLER(SVC_Handler);
WEAK_DEFAULT_HANDLER(PendSV_Handler);
WEAK_DEFAULT_HANDLER(SysTick_Handler);

WEAK_DEFAULT_HANDLER(WWDGT_IRQHandler);
WEAK_DEFAULT_HANDLER(LVD_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_IRQHandler);
WEAK_DEFAULT_HANDLER(FMC_IRQHandler);
WEAK_DEFAULT_HANDLER(RCU_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI0_1_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI2_3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI4_15_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA_Channel0_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA_Channel1_2_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA_Channel3_4_IRQHandler);
WEAK_DEFAULT_HANDLER(ADC_CMP_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER0_BRK_UP_TRG_COM_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER0_Channel_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER2_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER5_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER13_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER14_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER15_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMER16_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C0_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI0_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI1_IRQHandler);
WEAK_DEFAULT_HANDLER(USART0_IRQHandler);
WEAK_DEFAULT_HANDLER(USART1_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C0_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_ER_IRQHandler);

extern const VECTOR_TABLE_Type __VECTOR_TABLE[51];
const VECTOR_TABLE_Type __VECTOR_TABLE[51] __VECTOR_TABLE_ATTRIBUTE = {
    (VECTOR_TABLE_Type)(&__INITIAL_SP),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,

    WWDGT_IRQHandler,
    LVD_IRQHandler,
    RTC_IRQHandler,
    FMC_IRQHandler,
    RCU_IRQHandler,
    EXTI0_1_IRQHandler,
    EXTI2_3_IRQHandler,
    EXTI4_15_IRQHandler,
    0,
    DMA_Channel0_IRQHandler,
    DMA_Channel1_2_IRQHandler,
    DMA_Channel3_4_IRQHandler,
    ADC_CMP_IRQHandler,
    TIMER0_BRK_UP_TRG_COM_IRQHandler,
    TIMER0_Channel_IRQHandler,
    0,
    TIMER2_IRQHandler,
    TIMER5_IRQHandler,
    0,
    TIMER13_IRQHandler,
    TIMER14_IRQHandler,
    TIMER15_IRQHandler,
    TIMER16_IRQHandler,
    I2C0_EV_IRQHandler,
    I2C1_EV_IRQHandler,
    SPI0_IRQHandler,
    SPI1_IRQHandler,
    USART0_IRQHandler,
    USART1_IRQHandler,
    0,
    0,
    0,
    I2C0_ER_IRQHandler,
    0,
    I2C1_ER_IRQHandler
};

__NO_RETURN void Reset_Handler(void)
{
    SystemInit();
    __PROGRAM_START();
}

void Default_Handler(void)
{
    while (1) {
    }
}
