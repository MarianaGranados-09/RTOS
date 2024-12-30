#include <main.h>

extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;


void System_Init(void)
{
	GPIO_Init();
	UART1_Init();
	ADC_Init();
}

void ADC_Init(void)
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

void Error_Handler(void)
{
	while(1)
		{
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			HAL_Delay(50);
		}
}

void GPIO_Init(void){
	GPIO_InitTypeDef led_pin = {0};
	led_pin.Pin = GPIO_PIN_13;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &led_pin);

	GPIO_InitTypeDef m_pin = {0};
	m_pin.Pin = GPIO_PIN_0;
	m_pin.Mode = GPIO_MODE_OUTPUT_PP;
	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_Init(GPIOB, &m_pin);
}

