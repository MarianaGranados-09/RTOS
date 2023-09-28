#include <main.h>

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

void System_Init(void)
{
	   HAL_Init();
	   Clock_Init();
	   GPIO_Init();
	   UART1_Init();
	   TIM3_Init();
	   TIM2_Init();
	   TIM4_Init();
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

//setup timer3 to trigger the interrput each 1 ms
void TIM3_Init(void)
{
	//1. Habilitar reloj para periferico
	__HAL_RCC_TIM3_CLK_ENABLE();
	//2. Configuracion de bajo nivel (pins que va a utilizar)

	//3.Habilitacion de la interrupcion
	HAL_NVIC_SetPriority(TIM3_IRQn, 15, 0); //NESTED VECTOR INTERRUPT CONTROLLER
	HAL_NVIC_EnableIRQ(TIM3_IRQn); //interrupcion a traves del canal 3
	//4. Configuracion de alto nivel
	htim3.Instance = TIM3; //miembro que va a asociar a que timer va
	//CK_CNT = CK_PSC // (1 + Prescaler)
	htim3.Init.Prescaler = 999; //prescaler value for 100Mhz / (99 + 1) = 100kHz

	////UI_Freq = CK_CNT //(1 + Period)
	//htim3.Init.Period = 250; //hasta donde llega el contador antes de desbordarse

	htim3.Init.Period = 100; //period for 1 ms interrupt (100kHz / 1000 = 100Hz)
	if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
		Error_Handler();

	//5. Arrancar el periferico
	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_Base_Start_IT(&htim3); //Apuntador al periferico
}

void TIM4_Init(void)
{
	//1. Habilitar reloj para periferico
	__HAL_RCC_TIM4_CLK_ENABLE();
	//pin b6 para canal 1 tim4
	__HAL_RCC_GPIOB_CLK_ENABLE();


	//2. Configuracion de bajo nivel (pins que va a utilizar)
	GPIO_InitTypeDef oc_pin = {0};
	oc_pin.Pin = GPIO_PIN_6; //using pinb6 from alternate function mapping page 46
	oc_pin.Mode = GPIO_MODE_AF_PP; //alternate function push pull mode
	oc_pin.Alternate = GPIO_AF2_TIM4; //specifies which peripheral to associate to the pin
	HAL_GPIO_Init(GPIOB, &oc_pin);


	htim4.Instance = TIM4; //miembro que va a asociar a que timer va
	//CK_CNT = CK_PSC // (1 + Prescaler)
	htim4.Init.Prescaler = 8; //cada 1000 cuentas se resetea 16M/16k
	//16M/7999 = 2k
	////UI_Freq = CK_CNT //(1 + Period)
	//htim3.Init.Period = 250; //hasta donde llega el contador antes de desbordarse

	htim4.Init.Period = 200; //1kHz
	//CAMBIAR PARA 1 SEG ENC, 1 SEG APA. 0.5Hz
	//UI_Freq = CLK_PSC / (Prescaler * Period)
	HAL_TIM_Base_Init(&htim4); //cargarle al timer la configuracion

	//5. Running the peripheral

	TIM_OC_InitTypeDef oc_config = {0};
	oc_config.OCMode = TIM_OCMODE_PWM1;
	oc_config.OCNPolarity = TIM_OCPOLARITY_HIGH;
	oc_config.Pulse = 0;//when timer reaches this value, the signal will alternate //50%

	if(HAL_TIM_PWM_ConfigChannel(&htim4, &oc_config, TIM_CHANNEL_1) != HAL_OK) //loading pin configuration for ch1
	{
		Error_Handler();
	}

	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

}


void UART1_Init(void)
{
	//Habilitar reloj de periferico
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//__HAL_RCC_GPIOB_CLK_ENABLE();
	//Configuracion de bajo nivel
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &tx_pin);


	//Habilitacion de la interrupcion
	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);


	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.Mode = UART_MODE_TX_RX;
	if(HAL_UART_Init(&huart1) != HAL_OK)
		Error_Handler();
	//Configuracion de alto nivel
	//Arrancar el periferico


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


