/**
 * @file    app_gpio.c
 * @brief   GPIO Application Module
 * @details This module provides functions for GPIO initialization, control, reading inputs, and handling interrupts for different applications.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "app_gpio.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "AppSysIrqCallback.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_GPIO_UARTPRINT_ENABLE APP_FALSE

#if (APP_CPU_GPIO_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_gpio_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_gpio_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define RF0_SWITCH_PIN      EN_GPIO_PIN_5
#define RF2_SWITCH_PIN      EN_GPIO_PIN_6

#define LED_ON        1              // Define the state for turning the LED on
#define LED_OFF       0              // Define the state for turning the LED off

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
uint8_t rf0_current_state = 0;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   Initializes the GPIO module for LED control, button input with interrupt, and sensor input.
 * @details Configures GPIO pins for different functionalities: control an LED, read from a sensor, and handle button press interrupts.
 */
void app_gpio_init(void)
{
    stGPIO_InitTypeDef led1_pin,led2_pin,sensor_pin,button_pin;

    cb_scr_gpio_module_on();

    /*LED1 as GPIO Output Pin*/
    led1_pin.Pin = RF0_SWITCH_PIN;
    led1_pin.Mode = EN_GPIO_MODE_OUTPUT;
    led1_pin.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&led1_pin);

		app_gpio_set_rf0_switch(0);		//Low

    /*LED2 as GPIO Output Pin*/
    led2_pin.Pin = RF2_SWITCH_PIN;
    led2_pin.Mode = EN_GPIO_MODE_OUTPUT;
    led2_pin.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&led2_pin);
		
		app_gpio_set_rf2_switch(0);		//Low
	
	  rf0_current_state = 0;

}

/**
 * @brief   Toggles an LED connected to a GPIO pin.
 * @details This function toggles the state of the LED.
 */
void app_gpio_toggle_rf0_switch(void)
{
	if(rf0_current_state == 0)
	{
		rf0_current_state = 1;
    app_gpio_set_rf0_switch(1);
	}
	else
	{
		rf0_current_state = 0;
    app_gpio_set_rf0_switch(0);
	}
}

void app_gpio_toggle_rf0_2_switch(void)
{
	#if 0 //fixed test
	if(rf0_current_state == 0)
	{
		rf0_current_state = 1;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(0);
	}
	else if(rf0_current_state == 1)
	{
		rf0_current_state = 2;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(0);
	}
	else if(rf0_current_state == 2)
	{
		rf0_current_state = 3;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(0);
	}
	else if(rf0_current_state == 3)
	{
		rf0_current_state = 0;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(0);
	}
	#else
	if(rf0_current_state == 0)
	{
		rf0_current_state = 1;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(1);
	}
	else if(rf0_current_state == 1)
	{
		rf0_current_state = 2;
    app_gpio_set_rf0_switch(1);
		app_gpio_set_rf2_switch(0);
	}
	else if(rf0_current_state == 2)
	{
		rf0_current_state = 3;
    app_gpio_set_rf0_switch(1);
		app_gpio_set_rf2_switch(1);
	}
	else if(rf0_current_state == 3)
	{
		rf0_current_state = 0;
    app_gpio_set_rf0_switch(0);
		app_gpio_set_rf2_switch(0);
	}
	#endif
}

/**
 * @brief   Sets the LED state.
 * @param   state The desired state of the LED (0 for off, 1 for on).
 */
void app_gpio_set_rf0_switch(uint8_t state)
{
    if (state != 0)
    {
      cb_gpio_write_pin(RF0_SWITCH_PIN,EN_GPIO_PIN_SET);
    }
    else
    {
      cb_gpio_write_pin(RF0_SWITCH_PIN,EN_GPIO_PIN_RESET);
    } 
}

/**
 * @brief   Sets the LED state.
 * @param   state The desired state of the LED (0 for off, 1 for on).
 */
void app_gpio_set_rf2_switch(uint8_t state)
{
    if (state != 0)
    {
      cb_gpio_write_pin(RF2_SWITCH_PIN,EN_GPIO_PIN_SET);
    }
    else
    {
      cb_gpio_write_pin(RF2_SWITCH_PIN,EN_GPIO_PIN_RESET);
    } 
}

