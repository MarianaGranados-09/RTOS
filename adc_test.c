#include <main.h>
#include <stdio.h>
#include <string.h>

#define SAMPLES 4

UART_HandleTypeDef huart1 = {0}; //using uart 1 --manejador
ADC_HandleTypeDef hadc1 = {0}; //adc handler

void GPIO_Init(void);
void Error_Handler(void);
void Clock_Config(void);
void UART1_Init(void);


char mess[] = "UART\r\n";

uint8_t buflen, buffer[32];



int main(void)
{
   HAL_Init();
   Clock_Config();
   GPIO_Init();
   UART1_Init();
   ADC1_Init();

   uint16_t adc_value, i, value;
   uint32_t suma;

   while(1)
   {
	   for(i = 0, suma=0; i < SAMPLES; i++)
	   {
		   adc_value = HAL_ADC_GetValue(&hadc1);
		   suma+= adc_value;
		   HAL_Delay(1);

	   }
	   value = suma >> 2;
	   buflen = sprintf((char*)buffer, "%i %i\r\n", value, adc_value);
	   HAL_UART_Transmit(&huart1, buffer, buflen, HAL_MAX_DELAY);
	   HAL_Delay(50);
   }
   return 0;
}

void ADC1_Init(void)
{
	//1. habilitar reloj
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();

	//2. configuracion de bajo nivel -- pines
	GPIO_InitTypeDef adc_pin = {0};
	adc_pin.Pin = GPIO_PIN_0;
	adc_pin.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &adc_pin);
	//3. habilitacion de interrupcion
	//4. configuracion de alto nivel
	hadc1.Instance = ADC1;
	hadc1.Init.ContinuousConvMode = ENABLE; //specifies whether conversion is performed in single mode or continuous mode
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START; //who/what triggers the adc conversion, for example, the overflow of a timer, a GPIO pin, etc.
	hadc1.Init.NbrOfConversion = 1; //one conversion per cycle
	hadc1.Init.ScanConvMode = ENABLE;
	//hadc1.Init.EOCSelection

	if(HAL_ADC_Init(&hadc1) != HAL_OK)
		Error_Handler();

	ADC_ChannelConfTypeDef adc_chan = {0};
	adc_chan.Channel = ADC_CHANNEL_0;
	adc_chan.Rank = 1;
	if(HAL_ADC_ConfigChannel(&hadc1, &adc_chan) != HAL_OK)
		Error_Handler();

	//5. arrancar el periferico
	HAL_ADC_Start(&hadc1); //conversion of channel 0, continuous mode

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
   clock_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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

