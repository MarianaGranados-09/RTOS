#include <main.h>
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador


void GPIO_Init(void);
void Error_Handler(void);
void Clock_Config(void);
void UART1_Init(void);

char mess[] = "UART\r\n";

uint8_t byte, idx = 0;
uint8_t buffer[32];



int main(void)
{
   System_Init();
   HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);
   //HAL_Delay(1000); // Delay for 1 second

   while(1)
   {

	   HAL_UART_Receive(&huart1, &byte, 1, HAL_MAX_DELAY);
       if(byte == '\n')
       {
    	   buffer[idx++] = byte;
    	   HAL_UART_Transmit(&huart1, buffer, idx, HAL_MAX_DELAY);
    	   idx = 0;
       }
       else if(byte >= 'a' && byte <= 'z')
       {
    	   buffer[idx++] = byte - ('a'-'A');
       }
       else
       {
    	   buffer[idx++] = byte;
       }

   }
   return 0;
}


void System_Init(void)
{
	   HAL_Init();
	   Clock_Config();
	   GPIO_Init();
	   UART1_Init();
}



void UART1_Init(void)
{
	//Habilitar reloj de periferico
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	//Configuracion de bajo nivel
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &tx_pin);
	//Habilitacion de la interrupcion

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.Mode = UART_MODE_TX_RX;
	if(HAL_UART_Init(&huart1) != HAL_OK)
		Error_Handler();
	//Configuracion de alto nivel
	//Arrancar el periferico

}

void Clock_Config(void)
{
   RCC_OscInitTypeDef osc_init = {0};
   osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE; //Configure oscillator type as HSE
   osc_init.HSEState = RCC_HSE_ON;
   osc_init.LSEState = RCC_LSE_OFF;
   osc_init.HSIState = RCC_HSI_OFF;
   osc_init.LSIState = RCC_LSI_OFF;
   osc_init.PLL.PLLState = RCC_PLL_ON; //Turn on PLL
   osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE; //Set PLL source as HSE

   //SYSCLK = HSE * (N / M)
   osc_init.PLL.PLLM = 8;
   osc_init.PLL.PLLN = 200;
   osc_init.PLL.PLLP = RCC_PLLP_DIV2; //2
   osc_init.PLL.PLLQ = 2;
   //osc_init.PLL.PLLR = 2;
   if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
      Error_Handler();

   RCC_ClkInitTypeDef clock_init = {0};
   clock_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   clock_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
   clock_init.AHBCLKDivider = RCC_SYSCLK_DIV1;

   clock_init.APB1CLKDivider = RCC_HCLK_DIV2;
   clock_init.APB2CLKDivider = RCC_HCLK_DIV1;

   if (HAL_RCC_ClockConfig(&clock_init, FLASH_LATENCY_4) != HAL_OK)
      Error_Handler();
}

void Error_Handler(void)
{
   while(1)
   {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
   }
}

void GPIO_Init(void)
{
   /* Configurar pin PA5 como salida digital */
   __HAL_RCC_GPIOA_CLK_ENABLE();

   GPIO_InitTypeDef led_pin = {0};
   led_pin.Pin = GPIO_PIN_5;
   led_pin.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOA, &led_pin);
}

void SysTick_Handler(void)
{
   HAL_IncTick();
   HAL_SYSTICK_IRQHandler();
}
