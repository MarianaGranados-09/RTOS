#include <main.h>
#include <FreeRTOS.h>
#include <task.h>

UART_HandleTypeDef huart2 = {0};

void Task1(void *);
void Task2(void *);


int main(void)
{
		UART2_Init();
		xTaskCreate(Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 1,	NULL);
		xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 1,	NULL);
		vTaskStartScheduler();
		return 0;
}

void Task1(void *pvParameters)
{
	int i;
	UNUSED(pvParameters);
	const char *msg = "Hello from task one\r\n";
	while(1)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, 21, HAL_MAX_DELAY);
		for(i = 0; i < 10000; i++);
		//taskYIELD();
	}
}

void Task2(void *pvParameters)
{
	int i;
	UNUSED(pvParameters);
	const char *msg = "Hello from task two\r\n";
	while(1)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, 21, HAL_MAX_DELAY);
		for(i = 0; i < 10000; i++);
		//taskYIELD();
	}
}

void UART2_Init(void)
{
	/* Habilitar seÃ±ales de reloj */
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 2. Configuracion de bajo nivel */
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &tx_pin);
	/* 3. Habilitar interrupcion */
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/* 4. Configuracion de alto nivel */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.Mode = UART_MODE_TX_RX;
	if (HAL_UART_Init(&huart2) != HAL_OK)
		Error_Handler();
	// 5. Iniciar el periférico (todos aquellos que comienzan a trabajar y generan algun tipo de resultado)

}

void Error_Handler(void)
{
   while(1)
   {

   }
}

