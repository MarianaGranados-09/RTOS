#include <main.h>
#include <stdio.h>
#include <string.h>

//quadrature encoder interface - qei
int32_t pos = 0;


TIM_HandleTypeDef htim2 = {0}; //timer2 32 bit timer
UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador


void GPIO_Init(void);
void Error_Handler(void);
void Clock_Config(void);
void TIM2_Init(void); //measures signal
void UART1_Init(void);

uint8_t buflen, buffer[32];

char mess[] = "UART\r\n";



int main(void)
{
   System_Init();
   HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);
   //HAL_Delay(1000); // Delay for 1 second

   while(1)
   {
	   	  pos = __HAL_TIM_GET_COUNTER(&htim2);
    	  buflen = sprintf((char *)buffer, "Count: %li\r\n",pos);
    	  HAL_UART_Transmit(&huart1, buffer, buflen, HAL_MAX_DELAY);
    	  HAL_Delay(50);
      // HAL_UART_Transmit_IT(&huart1, buffer, buflen);
   }
   return 0;
}


void System_Init(void)
{
	   HAL_Init();
	   Clock_Config();
	   GPIO_Init();
	   TIM2_Init();
	   UART1_Init();
}

void TIM2_Init(void)
{
   /* Habilitar seniales de reloj */
   __HAL_RCC_TIM2_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();

   /* 2. Configuracion de bajo nivel */
   GPIO_InitTypeDef qei_pin = {0};
   qei_pin.Pin = GPIO_PIN_0 | GPIO_PIN_1;
   qei_pin.Mode = GPIO_MODE_AF_OD;
   qei_pin.Pull = GPIO_PULLUP;
   qei_pin.Alternate = GPIO_AF1_TIM2;
   HAL_GPIO_Init(GPIOA, &qei_pin);

   /* 3. Habilitar interrupcion */


   /* 4. Configuracion de alto nivel */
   htim2.Instance = TIM2;
   //htim2.Init.CounterMode = TIM_COUNTERMODE_UP; //default mode
   htim2.Init.Prescaler = 0; //100kHz / 100 - cada 1ms se incrementa la cuenta del timer
   htim2.Init.Period = 0xFFFFFFFF;

   TIM_Encoder_InitTypeDef qei_config = {0};
   qei_config.EncoderMode = TIM_ENCODERMODE_TI12;
   qei_config.IC1Selection = TIM_ICSELECTION_DIRECTTI;
   qei_config.IC2Selection = TIM_ICSELECTION_DIRECTTI;

   if(HAL_TIM_Encoder_Init(&htim2, &qei_config) != HAL_OK)
	   Error_Handler();


   /* 5. Arrancar el periferico */
   HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2); //IT for interrupts
}



void UART1_Init(void)
{
	//Habilitar reloj de periferico
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	//Configuracion de bajo nivel
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_9;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &tx_pin);
	//Habilitacion de la interrupcion

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.Mode = UART_MODE_TX;
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
