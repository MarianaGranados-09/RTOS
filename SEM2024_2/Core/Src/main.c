#include <main.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <semphr.h>
#include <queue.h>
#include <timers.h>
#include <time.h>

UART_HandleTypeDef huart1 = {0};
ADC_HandleTypeDef hadc1 = {0};

char bufin[128];
uint8_t idx = 0;
uint8_t byteRec;
int duty = 0;

SemaphoreHandle_t xsem;
QueueHandle_t queue;
TimerHandle_t xtimer;

void samplingTask(void *);
void BlinkTask(void *);
void parsingTask(void *);

void blinkTimerCallBack(TimerHandle_t);

char mess[] = "UART\r\n";

int main(void)
{

	System_Init();
	xsem = xSemaphoreCreateBinary();
	//HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);
	queue = xQueueCreate(128, sizeof(char)); //fila de 128 elementos
	xtimer = xTimerCreate("Blink", pdMS_TO_TICKS(250), pdTRUE, NULL, blinkTimerCallBack);

	//xTaskCreate(BlinkTask, "Blink", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(parsingTask, "Parse", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(samplingTask, "Sample", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL);

	//arrancar timer antes del calendarizador
	xTimerStart(xtimer, 10);

	vTaskStartScheduler();
	return 0;
}

void samplingTask(void *pvParameters)
{

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(5); //each 5 ms
	uint8_t i;
	uint16_t samples[8] = {0}, value;
	uint32_t suma;
	float volt, fvolt;
	uint8_t buffer[32], buflen;

	UNUSED(pvParameters);
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		for(i = 7; i > 0; i--)
		{
			samples[i] = samples[i-1];
		}
		samples[0] = HAL_ADC_GetValue(&hadc1);
		for(i = 0; i < 8; i++)
		{
			suma = suma + samples[i];
		}
		value = suma >> 3;
		volt = (3.3 / 4096.0) * samples[0];
		fvolt = (3.3 / 4096.0) * value;
		//buflen = sprintf((char *)buffer, "%.3f\t%.3f\r\n", volt*10, fvolt*10);
		buflen = sprintf((char *)buffer, "%.3f\t%.3f\r\n", volt*1, fvolt*1);
		for(i = 0; i < buflen; i++)
		{
			xQueueSend(queue, buffer+i, portMAX_DELAY);
		}
		//HAL_UART_Transmit(&huart1, buffer, buflen, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		//vTaskDelay(pdMS_TO_TICKS(500));
	}
	//vTaskDelete(NULL);
}

void parsingTask(void *pvParameters)
{
	UNUSED(pvParameters);
	HAL_UART_Receive_IT(&huart1, &byteRec, 1);
	while(1)
	{

		xSemaphoreTake(xsem, portMAX_DELAY);
		sscanf(bufin, "%i", &duty); //take what is in bufin, turn to int and put it in duty
	}
}

void BlinkTask(void *pvParameters)
{
	UNUSED(pvParameters);
	while(1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		vTaskDelay(pdMS_TO_TICKS(300));
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//UNUSED(huart);

	if(byteRec == '\n')
	{
		bufin[idx++] = '\0';
		idx = 0;
		xSemaphoreGiveFromISR(xsem, &xHigherPriorityTaskWoken);
	}
	else if(byteRec == '\r')
	{
		bufin[idx++] = byteRec;
	}

	HAL_UART_Receive_IT(&huart1, &byteRec, 1);


	if(xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

}

//funcion breve
void vApplicationIdleHook(void)
{
	char byteToSend;
	if(xQueueReceive(queue, &byteToSend, 0) == pdTRUE)
	{
		HAL_UART_Transmit(&huart1, (uint8_t*)&byteToSend, 1, HAL_MAX_DELAY);
	}
}

void blinkTimerCallBack(TimerHandle_t xtimer)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}





