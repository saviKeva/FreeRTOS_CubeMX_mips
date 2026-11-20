#include "keypad.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "gpio.h"

char keys[2];
int keysChanged = 0;

static uint32_t keyCount = 0;
static uint32_t keyReleased = 1;

extern unsigned temp_granica;

TimerHandle_t KEY_TimerHandle;
TaskHandle_t KEY_TaskHandle;

const char KEY_MATRIX[4][3] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7',
		'8', '9' }, { '*', '0', '#' } };

void KEY_Timer(TimerHandle_t xTimer) {
	UNUSED(xTimer);
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
		xTimerStart(KEY_TimerHandle, portMAX_DELAY);
	} else {
		keyReleased = 1;
	}

}

void KEY_EXTI_Callback(){
	BaseType_t woken = pdFALSE;
	vTaskNotifyGiveFromISR(KEY_TaskHandle, &woken);
	portYIELD_FROM_ISR(woken);

}

static void KEY_Task(void *params) {
	UNUSED(params);

	keys[0] = '?';
	keys[1] = '?';

	while (1) {

		for (int row = 0; row < 4; row++) {
			HAL_GPIO_WritePin(GPIOB, 1u << row, GPIO_PIN_SET);

		}

		vTaskDelay(pdMS_TO_TICKS(10));

		if (!keyReleased) {
			continue;
		}

		for (int row = 0; row < 4; row++) {
			HAL_GPIO_WritePin(GPIOB, 1u << row, GPIO_PIN_RESET);
		}

		for (int row = 0; row < 4; row++) {
			HAL_GPIO_WritePin(GPIOB, 1u << row, GPIO_PIN_SET);

			for (int col = 0; col < 3; col++) {
				if (HAL_GPIO_ReadPin(GPIOB, 1<<(4+col)) == GPIO_PIN_SET

				&& !(row==0 && (col==0 || col ==2))
				) {
					keys[keyCount] = KEY_MATRIX[row][col];

					if(keyCount == 1){
						temp_granica = (keys[0]-'0')*10 + keys[1] - '0';
						keys[0]='?';
						keys[1]='?';
					}


					keyCount = 1- keyCount;
					keysChanged = 1;
					keyReleased = 0;

					//OBAVEZNO TAJMER
					xTimerStart(KEY_TimerHandle, portMAX_DELAY);
				}
			}

			HAL_GPIO_WritePin(GPIOB, 1u<<row, GPIO_PIN_RESET);

		}

	}
}

void KEY_Init() {

	xTaskCreate(KEY_Task, "KeyTaskHandle", 128, NULL, 5, &KEY_TaskHandle);
	KEY_TimerHandle = xTimerCreate("keyTimer", pdMS_TO_TICKS(10), pdFALSE, NULL, KEY_Timer);


}
