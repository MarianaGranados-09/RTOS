#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PID_SAMPLE_TIME_S		0.001


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
//long kp = 15; //proportional gain
//float ki = 0.1;//0.1;//integral gain
//float kd = 0.12;//0.03;//1.3; //derivative gain

float Td = 0.025921;
float kp = 13.9211;

long samples = 0;

//pid vars
long error = 0;
long abs_error = 0;
//float integral = 0.0;
float deriv = 0.0;
float prev_error = 0.0;

float pid_output = 0.0;
uint32_t pwm_duty = 0;

long current_pos = 0;

uint8_t buflen, buffer_len[40];



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

		error = setpoint - current_pos;
		if(error < 0)
			abs_error = -error;
		else
			abs_error = error;


		//integral += (abs_error*PID_SAMPLE_TIME_S*ki);
		//anti wind up, integral limits
		/*if(integral > 100)
			integral = 100;*/
		deriv = (abs_error - prev_error) / PID_SAMPLE_TIME_S;

		//pid_output = (kp*error + ki*integral + kd*(error-prev_error)) / 1000;
		//pid_output = (kp*abs_error + integral + kd*deriv) / 1000;

		pid_output = (kp*abs_error + Td*deriv) / 1000;

		if(pid_output > 100)
			pid_output = 100;
		if(pid_output < - 100)
			pid_output = -100;

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

		if((current_pos-5 < setpoint) && (setpoint < current_pos + 5))
		{
			error = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
		}

		pwm_duty = pid_output;
		if(pwm_duty < 0)
			pwm_duty = -pwm_duty;


		prev_error = abs_error;
		/*if(prev_error < 0)
			prev_error = -prev_error;*/

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_duty);
		if(samples < 100)
		{
			buflen = sprintf((char*)buffer_len, "setpoint: %li current pos: %li error: %li\r\n", setpoint, current_pos, error);
			HAL_UART_Transmit(&huart1, buffer_len, buflen, HAL_MAX_DELAY);
			//samples = 0;
		}

		samples++;
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
	samples = 0;
	//memset(buffer, 0, sizeof(buffer));

	HAL_UART_Receive_IT(&huart1, &byte, 1); //start next receive


}


void Error_Handler(void)
{
	while(1);
}
