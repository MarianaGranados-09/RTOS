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
long setpoint = 0;


uint8_t byte, idx = 0;
uint8_t buffer[50];

//pid constants
double kp = 0.1; //proportional gain
double ki = 0.01; //integral gain
double kd = 0.001; //derivative gain

//pid vars
double error = 0;
double integral = 0;
double prev_error = 0;

double pid_output = 0;
double pwm_duty = 0;



int main(void)
{
   System_Init();
   HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);

   HAL_UART_Receive_IT(&huart1, &byte, 1);


   //HAL_UART_Receive_IT(&huart1, &byte, 1); //modo: recibir solo un byte y llamar funcion callback a traves de USART_IRQ
   //HAL_Delay(1000); // Delay for 1 second

   while(1)
   {

	   //HAL_UART_Receive_IT(&huart1, &byte, 1); //modo: recibir solo un byte y llamar funcion callback a traves de USART_IRQ
	   snprintf((char*)buffer, sizeof(buffer), "Setpoint: %ld\r\n", setpoint);
	   HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);
	   HAL_Delay(1000); //1 second delay


   }
   return 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//que timer se manda llamar dependiendo de la actividad
	if(htim -> Instance == TIM3) //en este caso el TIM3 es el que genera la interrupcion
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	//PID Algorithm control
	//1. read position
	//2. compare to set point - initially zero
	//3. calculate output (0 to 999)

	uint32_t current_pos = __HAL_TIM_GET_COUNTER(&htim2);

	error = setpoint - current_pos;

	pid_output = kp*error + ki*integral+ kd*(error-prev_error);

	//limitar salida del pid a 0-999
	pid_output = fmax(0, fmin(pid_output, 999));

	//convertir salida de pid a valor de pwm
	pwm_duty = (pid_output / 999)*100;

	TIM_OC_InitTypeDef oc_config = {0};
	oc_config.OCMode = TIM_OCMODE_PWM1;
	oc_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	oc_config.Pulse = pwm_duty;
	//Establecer valor de PWM
	if(HAL_TIM_PWM_ConfigChannel(&htim4, &oc_config, TIM_CHANNEL_1) != HAL_OK) //loading pin configuration for ch1
		Error_Handler();

	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

	integral += ki * error;
	prev_error = error;



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
	HAL_UART_Receive_IT(&huart1, &byte, 1); //start next receive

}



void Error_Handler(void)
{
	while(1);
}









