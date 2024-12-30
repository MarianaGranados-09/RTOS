#include <main.h>

UART_HandleTypeDef huart2 = {0};
TIM_HandleTypeDef htim2 = {0};

char mess[] = "UART\r\n";
uint8_t buflen, buffer[40];
long current_pos = 0;

int main(void)
{
	System_Init();
	while(1)
	{

		current_pos = __HAL_TIM_GET_COUNTER(&htim2);
		snprintf((char*)buffer, sizeof(buffer), "Current pos: %ld\r\n", current_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen((char*)buffer), HAL_MAX_DELAY);
		HAL_Delay(100);
	}


	return 0;
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
