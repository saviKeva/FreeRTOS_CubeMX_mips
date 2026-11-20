
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "adc.h"

#define MAX_VOLTAGE 5.0
#define RESOLUTION 4096.0

TaskHandle_t TEMP_TaskHandle;
QueueHandle_t TEMP_MailboxHandle;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == hadc1.Instance) {
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(TEMP_TaskHandle, &woken);
		portYIELD_FROM_ISR(woken)
	}

}

static void TEMP_Task(void *params) {

	UNUSED(params);
	float result;
	while (1) {
		HAL_ADC_Start_IT(&hadc1);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		result = HAL_ADC_GetValue(&hadc1);

		result *= MAX_VOLTAGE / RESOLUTION;
		result *= 100;

		xQueueOverwrite(TEMP_MailboxHandle, &result);

		vTaskDelay(pdMS_TO_TICKS(200));

	}
}

float TEMP_GetCurrentValue() {
	float result=0.0;
	xQueuePeek(TEMP_MailboxHandle, &result, portMAX_DELAY);
	if (result < 0) {
		return 0.0;
	} else if (result > 60) {
		return 60.0;
	}
	return result;

}

void TEMP_Init() {
	TEMP_MailboxHandle = xQueueCreate(1, sizeof(float));
	xTaskCreate(TEMP_Task, "TEMP_Task", 64, NULL, 2, &TEMP_TaskHandle);
}
