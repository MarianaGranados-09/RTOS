#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define UART_BUFFER_SIZE 128
char uartBuffer[UART_BUFFER_SIZE];
volatile uint8_t uartRxIndex = 0;
volatile uint8_t uartRxComplete = 0;

//extender definicion a archivo main_init.c

UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador
TIM_HandleTypeDef htim3 = {0};
TIM_HandleTypeDef htim2 = {0}; //timer2 32 bit timer
TIM_HandleTypeDef htim4 = {0};

char mess[] = "UART\r\n";
char mess1[] = "ISR TIM3\r\n";

long setpoint = 0;

uint8_t byte, idx = 0;
uint8_t buffer[50];

//pid constants
float kp = 4.0; //proportional gain
float ki = 0.00001; //integral gain
float kd = 0.0001; //derivative gain

//pid vars
float error = 0;
float integral = 0;
float prev_error = 0;

float pid_output = 0;
uint32_t pwm_duty = 0;

long current_pos = 0;

uint8_t buflen, buffer1[32], buffer2[32], buffer3[40], buffer4[40], buffer5[40], buffer6[40];



int main(void)
{

   HAL_UART_Receive_IT(&huart1, &byte, 1);

   while(1)
   {


	 if(uartRxComplete)
	 {
		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
		 setpoint = strtol(uartBuffer, NULL, 10);
		 uartRxIndex = 0;
		 uartRxComplete = 0;
		 HAL_Delay(100);
		 HAL_UART_Receive_IT(&huart1, &byte, 1);
		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

	 }

   }
   return 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//que timer se manda llamar dependiendo de la actividad
	if(htim -> Instance == TIM3) //en este caso el TIM3 es el que genera la interrupcion
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		//PID Algorithm control
		//1. read position
		//2. compare to set point - initially zero
		//3. calculate output (0 to 999)
		current_pos = __HAL_TIM_GET_COUNTER(&htim2);

		//current_pos = __HAL_TIM_GET_COUNTER(&htim2);
		error = setpoint - current_pos;
		integral += ki * error;

		snprintf((char*)buffer, sizeof(buffer), "Setpoint: %ld\r\n", setpoint);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);

		/*
		snprintf((char*)buffer1, sizeof(buffer1), "Error: %ld\r\n", error);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer1, strlen((char*)buffer1), HAL_MAX_DELAY);

		snprintf((char*)buffer2, sizeof(buffer2), "Current pos: %ld\r\n", current_pos);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen((char*)buffer2), HAL_MAX_DELAY);*/


		pid_output = (kp*error + ki*integral+ kd*(error-prev_error)) / 1000;

		//limitar salida del pid a 0-999
		if(pid_output > 999)
			pid_output = 999;
		if(pid_output < -999)
			pid_output = -999;

		if(pid_output < 0)
		{
			pid_output = -pid_output;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		}
		if(pid_output > 0)
		{
			pid_output = pid_output;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		}
		if(pid_output == 0)
		{
			pid_output = pid_output;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
		}

		//snprintf((char*)buffer3, sizeof(buffer2), "pid_output: %ld\r\n", pid_output);
		//HAL_UART_Transmit(&huart1, (uint8_t*)buffer3, strlen((char*)buffer3), HAL_MAX_DELAY);

		prev_error = error;
		//convertir salida de pid a valor de pwm
		pwm_duty = pid_output;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_duty);
	}
}

//manda llamar una vez que recibe la cantidad de bytes indicados,
//se ejecuta a traves del IRQHandler
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		if(byte == '\n')
		{
			uartRxComplete = 1;
			uartBuffer[uartRxIndex++] = '\0';
		}
		else{
			uartBuffer[uartRxIndex] = byte;
		}

		HAL_UART_Receive_IT(huart, &byte, 1);
	}

}


void SysTick_Handler(void)
{
   HAL_IncTick();
   HAL_SYSTICK_IRQHandler();
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);
}

void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart1);
}

void Error_Handler(void)
{
	while(1);
}
