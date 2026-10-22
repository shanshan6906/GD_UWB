#ifndef GD32E23X_IT_H
#define GD32E23X_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI0_1_IRQHandler(void);

#endif /* GD32E23X_IT_H */
