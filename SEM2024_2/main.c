#include <main.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semphr.h>
#include <queue.h>
#include <timers.h>
#include <time.h>

UART_HandleTypeDef huart2 = {0};
TIM_HandleTypeDef htim2 = {0}; //tim2 for reading encoder signals
TIM_HandleTypeDef htim4 = {0}; //tim4 for pwm signal


TimerHandle_t blink_timer;
TimerHandle_t pid_algo_timer;

TaskHandle_t pid_task_handle;
TaskHandle_t uart_task_handle;

QueueHandle_t uartQueueH;

uint8_t receivedByte;
uint8_t buflen = 0, buffer[32];
uint8_t flag_uart = 0;

uint8_t buflen2 = 0, buffer2[32];
uint8_t buflen3 = 0, buffer3[32];

long new_setpoint, prev_setpoint;
uint32_t duty = 0;

long current_pos = 0;
long prev_error = 0;
long error = 0;
long error_dif = 0;

long kp= 60, ti = 2, td = 1;
float ki, kd;

float prop_correc, inte_correc, prev_inte_correc, deriv_correc, total_correc;

float pid_time = 0.001;
long samples = 0;

void uart_task_parse(void *pvParameters)
{
	uint8_t receivedData;
	char numBuffer[16] = {0};
	uint8_t index = 0;
	for(;;)
	{
		if(xQueueReceive(uartQueueH, &receivedData, portMAX_DELAY) == pdPASS)
		{
			flag_uart = 1;

			if(receivedData >= '0' && receivedData <= '9')
			{

				numBuffer[index++] = receivedData;

			}
			else if(receivedData == '\n' || receivedData == '\r')
			{

					//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
					new_setpoint = atoi(numBuffer);
					snprintf((char *)buffer, sizeof(buffer), "Setpoint: %li\r\n", new_setpoint);
					HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);
					//flag_uart = 0;

					//memset(numBuffer, 0, sizeof(numBuffer));
					index = 0;
					//flag_uart = 0;

			}

		}
	}
}


//led blink timer callback
void BlinkLEDcallback(TimerHandle_t xTimer)
{
	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}


void PID_ALGOcallback(TimerHandle_t xTimer)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(pid_task_handle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void PID_ALGO_task(void *pvParameters)
{
	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(flag_uart == 1)
		{
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
			current_pos = __HAL_TIM_GET_COUNTER(&htim2);
			error = current_pos - new_setpoint;

			if(error < 0)
			{
				error = -error;
			}

			prop_correc = kp*error;
			if(ti != 0)
				ki = kp / ti;
			else
				ki = 0;


			inte_correc = prev_inte_correc + (ki*pid_time*error);

			kd = kp*td;
			error_dif = error - prev_error;

			deriv_correc = (kd * error_dif) / pid_time;

			total_correc = (prop_correc+inte_correc+deriv_correc) / 10000;
			if(total_correc > 100)
				total_correc = 100;
			if(total_correc < -100)
				total_correc = -100;

			if(new_setpoint < current_pos){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
			}
			if(new_setpoint > current_pos)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
			}
			if((current_pos-5 < new_setpoint) && (new_setpoint < current_pos+5))
			{
				error = 0;
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
			}

			duty = total_correc;
			if(duty < 0)
				duty = -duty;

			prev_inte_correc = inte_correc;
			prev_error = error;

			__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, duty);

			if(samples < 500)
			{
				buflen3 = snprintf((char*)buffer3, sizeof(buffer3), "%li  %li   %li\r\n", new_setpoint, current_pos, error);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer3, buflen3, HAL_MAX_DELAY);
			}

			samples++;
			//prev_setpoint = new_setpoint;


			//buflen2 = snprintf((char *)buffer2, sizeof(buffer2), "Error: %li \r\n", error);
			//HAL_UART_Transmit(&huart2, (uint8_t*)buffer2, buflen2, HAL_MAX_DELAY);
		}


	}
}


void uart_task_sendmssg(void *pvParameters)
{
	UNUSED(pvParameters);
	char mess[] = "Send setpoint: \r\n";
	for(;;)
	{
		if(flag_uart == 0)
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)mess, strlen(mess), HAL_MAX_DELAY);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}

	}
}


int main(void)
{

	System_Init();
	blink_timer = xTimerCreate("blinktimer", pdMS_TO_TICKS(100), pdTRUE, NULL, BlinkLEDcallback);
	pid_algo_timer = xTimerCreate("pidtimer", pdMS_TO_TICKS(50), pdTRUE, NULL, PID_ALGOcallback);

	if(pid_algo_timer == NULL)
	{
		Error_Handler();
	}

	if(blink_timer == NULL)
	{
		Error_Handler();
	}
	if(xTimerStart(blink_timer, 0) != pdPASS)
	{
		Error_Handler();
	}

	if(xTimerStart(pid_algo_timer, 0) != pdPASS)
	{
		Error_Handler();
	}

	//create uart task
	if(xTaskCreate(uart_task_sendmssg, "uart_send", configMINIMAL_STACK_SIZE*2, NULL, 1, &uart_task_handle) != pdPASS)
	{
		Error_Handler();
	}

	uartQueueH = xQueueCreate(16, sizeof(uint8_t));
	if(uartQueueH == NULL)
	{
		Error_Handler();
	}

	if(xTaskCreate(uart_task_parse, "uart_recep", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL) != pdPASS)
	{
		Error_Handler();
	}
	if(xTaskCreate(PID_ALGO_task, "PID_task", configMINIMAL_STACK_SIZE*2, NULL, 3, &pid_task_handle) != pdPASS)
	{
		Error_Handler();
	}
	HAL_UART_Receive_IT(&huart2, &receivedByte, 1);
	vTaskStartScheduler();
	while(1);
	return 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart -> Instance == USART2)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(uartQueueH, &receivedByte, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		HAL_UART_Receive_IT(&huart2, &receivedByte, 1);
	}
}



void Error_Handler()
{
	while(1)
	{

	}
}





