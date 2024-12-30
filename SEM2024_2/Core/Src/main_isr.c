#include <main.h>

extern UART_HandleTypeDef huart1;

void USART_IRQHandler()
{
	HAL_UART_IRQHandler(&huart1);
}
