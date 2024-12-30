#include <main.h>

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;




void System_Init(void)
{
	 HAL_Init();
	 Clock_Init();
	 GPIO_Init();
	 UART2_Init();
	 TIM2_Init();
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


void Clock_Init(void)
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

void GPIO_Init(void)
{
   /* Configurar pin PA5 como salida digital */
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();

   GPIO_InitTypeDef led_pin = {0};
   led_pin.Pin = GPIO_PIN_5;
   led_pin.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOA, &led_pin);

   GPIO_InitTypeDef led_debug = {0};
   led_debug.Pin = GPIO_PIN_4 | GPIO_PIN_5;
   led_debug.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOC, &led_debug);


   GPIO_InitTypeDef in_driver = {0};
   in_driver.Pin =  GPIO_PIN_0 | GPIO_PIN_1;
   in_driver.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOC, &in_driver);

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

void Error_Handler(void)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(50);
	}
}

