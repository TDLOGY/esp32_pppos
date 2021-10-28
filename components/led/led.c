#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "esp_system.h"

#include "driver/gpio.h"
#include "led.h"

#define LED_RED_CONN_PIN       	(17)
#define LED_GREEN_CONN_PIN      (17)

#define LED_RED_SYS_PIN       	(16)
#define LED_BLUE_SYS_PIN       	(16)

#define GPIO_OUTPUT_PIN_SEL ((1UL << LED_BLUE_SYS_PIN) | \
                            (1UL << LED_RED_CONN_PIN) | \
							(1UL << LED_RED_SYS_PIN) | \
                            (1UL << LED_GREEN_CONN_PIN))

#define LEDS_RED_CONN_OFF()		gpio_set_level(LED_RED_CONN_PIN, 0)
#define LEDS_RED_CONN_ON()		gpio_set_level(LED_RED_CONN_PIN, 1)

#define LEDS_GREEN_CONN_OFF()	gpio_set_level(LED_GREEN_CONN_PIN, 0)
#define LEDS_GREEN_CONN_ON()	gpio_set_level(LED_GREEN_CONN_PIN, 1)

#define LEDS_RED_SYS_OFF()		gpio_set_level(LED_RED_SYS_PIN, 0)
#define LEDS_RED_SYS_ON()		gpio_set_level(LED_RED_SYS_PIN, 1)

#define LEDS_BLUE_SYS_OFF()		gpio_set_level(LED_BLUE_SYS_PIN, 0)
#define LEDS_BLUE_SYS_ON()		gpio_set_level(LED_BLUE_SYS_PIN, 1)

static LEDDiplayMode_t ledConnDisplayMode;
static uint8_t ledConnCnt;

static LEDDiplayMode_t ledSysDisplayMode;
static uint8_t ledSysCnt;

static void led_task(void * param);
void led_conn_display(void);
void led_sys_display(void);

void leds_init(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

	xTaskCreate(led_task, "led_task", 1024U, NULL, 7, NULL);

	LEDS_RED_SYS_OFF();
	LEDS_BLUE_SYS_OFF();

	LEDS_RED_CONN_OFF();
	LEDS_GREEN_CONN_OFF();
}

static void led_task(void * param)
{
	while(true)
	{
		led_conn_display();
		led_sys_display();

		if((ledSysDisplayMode == LED_NONE) && 
			(ledConnDisplayMode == LED_NONE))
		{
			vTaskDelay(5000 / portTICK_RATE_MS);	
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

void led_sys_display(void)
{
	switch (ledSysDisplayMode)
	{
	case LED_NONE:
		break;
	case LED_CONNECTING:
		ledSysCnt++;
		if(ledSysCnt == 10)
		{
			LEDS_BLUE_SYS_OFF();
		}
		else if(ledSysCnt >= 20)
		{
			ledSysCnt = 0;
			LEDS_BLUE_SYS_ON();
		}
		break;
	case LED_CONNECTED:
		LEDS_BLUE_SYS_ON();
		LEDS_RED_SYS_ON();
	default:
		break;
	}
}

void led_sys_set_display(LEDDiplayMode_t mode)
{
	if(ledSysDisplayMode == mode)
	{
		return;
	}
	ledSysDisplayMode = mode;

	LEDS_BLUE_SYS_OFF();
	LEDS_RED_SYS_OFF();

	ledSysCnt = 0;
}

void led_conn_display(void)
{
	switch(ledConnDisplayMode)
	{
	case LED_START_UP:
		break;
	case LED_CONNECTING:
		ledConnCnt++;
		if(ledConnCnt == 10)
		{
			LEDS_RED_CONN_OFF();
			LEDS_GREEN_CONN_OFF();
		}
		else if(ledConnCnt >= 20)
		{
			ledConnCnt = 0;
			LEDS_RED_CONN_ON();
			LEDS_GREEN_CONN_ON();
		}

		break;
	case LED_CONNECTED:
		ledConnCnt++;
		if(ledConnCnt == 1)
		{
			LEDS_RED_CONN_OFF();
			LEDS_GREEN_CONN_OFF();
		}
		else if(ledConnCnt >= 10)
		{
			ledConnCnt = 0;
			LEDS_RED_CONN_ON();
			LEDS_GREEN_CONN_ON();
		}
		break;
	case LED_UPDATE_FW:
		ledConnCnt++;
		if(ledConnCnt == 1)
		{
			LEDS_RED_CONN_OFF();
			LEDS_GREEN_CONN_OFF();
		}
		else if(ledConnCnt >= 2)
		{
			ledConnCnt = 0;
			LEDS_RED_CONN_ON();
			LEDS_GREEN_CONN_ON();
		}
		break;
	default:
		break;
	}
}

void led_conn_set_mode(LEDDiplayMode_t mode)
{
	if(ledConnDisplayMode == mode)
	{
		return;
	}
	ledConnDisplayMode = mode;
	ledConnCnt = 0;

	LEDS_GREEN_CONN_OFF();
	LEDS_RED_CONN_OFF();

	switch(ledConnDisplayMode)
	{
	case LED_START_UP:
		LEDS_RED_CONN_ON();
		break;
	case LED_CONNECTING:
		break;
	case LED_CONNECTED:
		break;
	default:
		break;
	}
}
