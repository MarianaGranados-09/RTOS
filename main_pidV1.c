#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


//extender definicion a archivo main_init.c

UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador
TIM_HandleTypeDef htim3 = {0};
TIM_HandleTypeDef htim2 = {0}; //timer2 32 bit timer
TIM_HandleTypeDef htim4 = {0};

char mess[] = "UART\r\n";
char mess1[] = "ISR TIM3\r\n";

long setpoint = 0;
long new_setpoint = 0;

uint8_t byte, idx = 0;
uint8_t buffer[50];

//pid constants
long kp = 4; //proportional gain
float ki = 0.001; //integral gain
float kd = 0.1; //derivative gain

//pid vars
long error = 0;
float integral = 0;
float prev_error = 0;

long pid_output = 0;
uint32_t pwm_duty = 0;

long current_pos = 0;

uint8_t buflen, buffer1[32], buffer2[32], buffer3[40], buffer4[40], buffer5[40], buffer6[40];



int main(void)
{
   System_Init();
   //HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);

   HAL_UART_Receive_IT(&huart1, &byte, 1);

   while(1)
   {
	   //snprintf((char*)buffer, sizeof(buffer), "Setpoint: %ld\r\n", setpoint);
	   //HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);

	   //HAL_Delay(100);

   }
   return 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//que timer se manda llamar dependiendo de la actividad
	if(htim -> Instance == TIM3) //en este caso el TIM3 es el que genera la interrupcion
	{

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);
		//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_duty);
		//PID Algorithm control
		//1. read position
		//2. compare to set point - initially zero
		//3. calculate output (0 to 999)
		current_pos = __HAL_TIM_GET_COUNTER(&htim2);

		snprintf((char*)buffer, sizeof(buffer), "Setpoint: %ld\r\n", setpoint);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);

		snprintf((char*)buffer2, sizeof(buffer2), "Current pos: %ld\r\n", current_pos);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer2, strlen((char*)buffer2), HAL_MAX_DELAY);

		error = setpoint - current_pos;
		//if(error < 0)
		//	error = -error;
		snprintf((char*)buffer1, sizeof(buffer1), "Error: %ld\r\n", error);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer1, strlen((char*)buffer1), HAL_MAX_DELAY);

		//integral += ki * error;
		//pid_output = (kp*error + ki*integral + kd*(error-prev_error)) / 1000;
		pid_output = (kp*error)/ 1000;

		//limitar salida del pid a 0-999
		if(pid_output > 999)
			pid_output = 999;
		if(pid_output < -999)
			pid_output = -999;

		if(setpoint < current_pos)
		{
			//pid_output = -pid_output;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		}
		if(setpoint > current_pos)
		{
			//pid_output = pid_output;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		}
		/*if(setpoint == current_pos)
		{
			pid_output = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
		}*/

		//prev_error = error;
		//convertir salida de pid a valor de pwm
		pwm_duty = pid_output;
		if(pwm_duty < 0)
			pwm_duty = -pwm_duty;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_duty);

		snprintf((char*)buffer3, sizeof(buffer2), "pid_output: %li\r\n", pid_output);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer3, strlen((char*)buffer3), HAL_MAX_DELAY);
	}

}

//manda llamar una vez que recibe la cantidad de bytes indicados,
//se ejecuta a traves del IRQHandler
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(byte == '\n')
	{
		buffer[idx++] = byte;
		HAL_UART_Transmit(&huart1, buffer, idx, HAL_MAX_DELAY);
		idx = 0;

	}
	else
	{
		buffer[idx++] = byte;
	}

	setpoint = strtol((char*)buffer, NULL, 10); //convert string to long int
	//memset(buffer, 0, sizeof(buffer));

	HAL_UART_Receive_IT(&huart1, &byte, 1); //start next receive


}


void Error_Handler(void)
{
	while(1);
}
