/*
 * driver_uart.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "driver_uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#include "usart.h"

// TRANSMIT
// -----------------------------------------------------------------------------

static TaskHandle_t UART_TransmitTaskHandle;
static QueueHandle_t UART_TransmitQueueHandle;
static SemaphoreHandle_t UART_TransmitMutexHandle;

static void UART_TransmitTask(void *parameters) {

	UNUSED(parameters);
	char buff;
	while (1) {
		xQueueReceive(UART_TransmitQueueHandle, &buff, portMAX_DELAY);
		HAL_UART_Transmit_IT(&huart1, &buff, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == huart1.Instance) {
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(UART_TransmitTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}

}

// RECEIVE
// -----------------------------------------------------------------------------

static TaskHandle_t UART_ReceiveTaskHandle;
static QueueHandle_t UART_ReceiveQueueHandle;
static SemaphoreHandle_t UART_ReceiveMutexHandle;

static void UART_ReceiveTask(void *parameters) {

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

}

// GENERAL
// -----------------------------------------------------------------------------

void UART_Init() {
	UART_TransmitQueueHandle = xQueueCreate(64, sizeof(float));
	UART_TransmitMutexHandle = xSemaphoreCreateMutex();
	xTaskCreate(UART_TransmitTask, "UartTransmitTask", 64, NULL, 4,
			&UART_TransmitTaskHandle);

}

// TRANSMIT UTIL
// -----------------------------------------------------------------------------

void UART_AsyncTransmitCharacter(char character) {
	xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);
	xQueueSendToBack(UART_TransmitQueueHandle, &character, portMAX_DELAY);
	xSemaphoreGive(UART_TransmitMutexHandle);

}

void UART_AsyncTransmitString(char const *string) {

	if (string != NULL) {
		xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);
		while(*string){
			xQueueSendToBack(UART_TransmitQueueHandle, string++, portMAX_DELAY);
		}

		xSemaphoreGive(UART_TransmitMutexHandle);
	}

}

void UART_AsyncTransmitDecimal(uint32_t decimal) {

}

// RECEIVE UTIL
// -----------------------------------------------------------------------------

char UART_BlockReceiveCharacter() {

}

char* UART_BlockReceiveString() {

}

uint32_t UART_BlockReceiveDecimal() {

}
