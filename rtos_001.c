#include <main.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>


static void task1_handler(void* parameters);
static void task2_handler(void* parameters);

void UART2_Init(void);

TaskHandle_t task1_h; //task1 handler
TaskHandle_t task2_h;

UART_HandleTypeDef huart2 = {0};

BaseType_t status;


int main(void)
{

		UART2_Init();
		status = xTaskCreate(task1_handler, "Task-1", 200, "Hello world from Task-1", 2, &task1_h);

		configASSERT(status == pdPASS); //task1 was created successfully

		status = xTaskCreate(task2_handler, "Task-2", 200, "Hello world from Task-2", 2, &task2_h);
		configASSERT(status == pdPASS); //task2 was created successfully

		vTaskStartScheduler();

		return 0;
}

static void task1_handler(void* parameters){

	for(;;)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)parameters, 24, HAL_MAX_DELAY);
	}

}

static void task2_handler(void* parameters){

	for(;;)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)parameters, 24, HAL_MAX_DELAY);
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

