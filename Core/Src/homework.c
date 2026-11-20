/*
 * homework.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "timers.h"
#include "homework.h"
#include <stdlib.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_uart.h"
#include "driver_motor.h"
#include "driver_temp.h"

#include "keypad.h"

FanState fanState = TURNED_OFF;

unsigned temp_granica = 30;
unsigned tempValue;
unsigned passedMs;
unsigned rainFall;
char rainText[4];

TimerHandle_t ledTimHandle;
TimerHandle_t homeworkHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_7) {
		KEY_EXTI_Callback();
	} else if (GPIO_Pin == GPIO_PIN_14) {
		homeworkOverflow();
	}
}

void ledCounter(TimerHandle_t xTimer) {
	UNUSED(xTimer);
	if (TEMP_GetCurrentValue() < temp_granica) {
		//zeleno
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	} else {
		//crveno
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
	}
}

void homeworkCounter(TimerHandle_t xTimer) {
	UNUSED(xTimer);
	passedMs++;
}

void homeworkOverflow() {
	rainFall = 36000 / passedMs;
	passedMs = 0;
}

void LCD_WriteString(char *string, int len) {
	int j = 0;
	while (*string && j < len) {
		LCD_CommandEnqueue(LCD_DATA, *string++);
		j++;
	}
}

static void homeworkTask(void *parameters) {

	UNUSED(parameters);

	char msgTemp[9] = "Temp:   ";
	char msgKisa[8] = "Kisa:  ";
	char msgThld[14] = "Thld:(??)=>30";

	LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
	LCD_WriteString(msgKisa, 8);
	LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0X50);
	LCD_WriteString(msgThld, 14);

	UART_AsyncTransmitString(msgTemp);

	while (1) {
		tempValue = TEMP_GetCurrentValue();
		itoa(rainFall, rainText, 10);

		FanState fanStateTarget;

		if (tempValue >= temp_granica) {
			fanStateTarget = SLOW;
		} else {
			fanStateTarget = TURNED_OFF;
		}

		while (fanState != fanStateTarget) {
			if (fanState < fanStateTarget) {
				MOTOR_SpeedIncrease();
				fanState++;
			} else {
				MOTOR_SpeedDecrease();
				fanState--;
			}
		}

		fanState = fanStateTarget;

		UART_AsyncTransmitCharacter('\b');
		UART_AsyncTransmitCharacter('\b');

		if (tempValue < 10) {
			UART_AsyncTransmitCharacter(' ');
		} else {
			UART_AsyncTransmitCharacter(tempValue / 10 + '0');
		}

		UART_AsyncTransmitCharacter(tempValue % 10 + '0');

		LCD_CommandEnqueue(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x06);
		LCD_WriteString(rainText, (rainFall < 100) ? 3 : 4);

		if (keysChanged) {
			keysChanged = 0;

			LCD_CommandEnqueue(LCD_INSTRUCTION,
								LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x56);
			LCD_CommandEnqueue(LCD_DATA, keys[0]);
			LCD_CommandEnqueue(LCD_DATA, keys[1]);

			LCD_CommandEnqueue(LCD_INSTRUCTION,
					LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x5B);
			if (temp_granica < 10) {
				LCD_CommandEnqueue(LCD_DATA, ' ');
			} else {
				LCD_CommandEnqueue(LCD_DATA, temp_granica / 10 + '0');
			}
			LCD_CommandEnqueue(LCD_DATA, temp_granica % 10 + '0');
		}


		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void homeworkInit() {

	TEMP_Init();
	MOTOR_Init();
	UART_Init();
	LCD_Init();
	KEY_Init();

	xTaskCreate(homeworkTask, "homeworkTask", 128, NULL, 5, NULL);
	ledTimHandle = xTimerCreate("ledTimer", pdMS_TO_TICKS(500), pdTRUE, NULL,
			ledCounter);
	homeworkHandle = xTimerCreate("homewTimer", pdMS_TO_TICKS(1), pdTRUE, NULL,
			homeworkCounter);

	xTimerStart(homeworkHandle, portMAX_DELAY);
	xTimerStart(ledTimHandle, portMAX_DELAY);

}

