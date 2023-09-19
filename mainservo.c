#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//extender definicion a archivo main_init.c

UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador
TIM_HandleTypeDef htim3 = {0};
TIM_HandleTypeDef htim2 = {0}; //timer2 32 bit timer
TIM_HandleTypeDef htim4 = {0};

char mess[] = "UART\r\n";
long setpoint = 0;


uint8_t byte, idx = 0;
uint8_t buffer[50];



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









