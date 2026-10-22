#include "gd32e23x_it.h"
#include "board.h"
#include "uwb_uart.h"

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    board_systick_irq_handler();
}

void EXTI0_1_IRQHandler(void)
{
    board_uwb_int_irq_handler();
}

