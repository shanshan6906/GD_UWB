#include "gd32e23x.h"
#include "board.h"

static volatile uint8_t s_uwb_int_pending;
static volatile uint32_t s_millisecond_counter;

void board_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_CFGCMP);

    gpio_mode_set(UWB_INT_GPIO_PORT,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLDOWN,
                  UWB_INT_GPIO_PIN);

    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN1);
    exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_1);
    nvic_irq_enable(EXTI0_1_IRQn, 1U);

    (void)SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t board_millis(void)
{
    return s_millisecond_counter;
}

void board_delay_ms(uint32_t delay_ms)
{
    uint32_t start_ms = board_millis();

    while ((uint32_t)(board_millis() - start_ms) < delay_ms) {
    }
}

void board_systick_irq_handler(void)
{
    ++s_millisecond_counter;
}

uint8_t board_uwb_int_level(void)
{
    return (gpio_input_bit_get(UWB_INT_GPIO_PORT, UWB_INT_GPIO_PIN) == SET) ? 1U : 0U;
}

uint8_t board_uwb_int_take(void)
{
    uint8_t pending;

    __disable_irq();
    pending = s_uwb_int_pending;
    s_uwb_int_pending = 0U;
    __enable_irq();

    return pending;
}

void board_uwb_int_irq_handler(void)
{
    if (exti_interrupt_flag_get(EXTI_1) != RESET) {
        exti_interrupt_flag_clear(EXTI_1);
        s_uwb_int_pending = 1U;
    }
}
