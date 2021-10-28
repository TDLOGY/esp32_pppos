/**
 * ******************************************************************************
 * @file:    led.h
 * @author:  Hong Duc Vo
 * @version: v1.0
 * @date:    2019-Jun-30
 * @brief:   Main
 * ******************************************************************************
 * @par: Microcontroller
 *   STM32F030C8Tx
 * @par: Compiler
 *   arm-none-eabi-gcc
 * ******************************************************************************
 * COPYRIGHT (c) 2019
 *
 * @par: Project
 *  Fuel
 * ******************************************************************************
 */

#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	LED_NONE,
	LED_START_UP,
	LED_CONNECTING,
	LED_CONNECTED,
	LED_UPDATE_FW,
}LEDDiplayMode_t;

void leds_init(void);
void led_conn_set_mode(LEDDiplayMode_t mode);
void led_sys_set_display(LEDDiplayMode_t mode);

#ifdef __cplusplus
}
#endif


#endif
