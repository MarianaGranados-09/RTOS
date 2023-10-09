#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

ADC_HandleTypeDef hadc1 = {0};
UART_HandleTypeDef huart1 = {0};
TIM_HandleTypeDef htim1 = {0};
TIM_HandleTypeDef htim3 = {0};

float Vout = 0.0;
float Vref = 5.0;

uint16_t duty = 50;

uint8_t byte, buffer_uart[32];
uint8_t buflen_uart = 0;

uint8_t buflen_p, buffer_p[32];

uint16_t adc_value;
float adc_volt;

int main(void)
{
	System_Init();
	while(1)
	{
		//buflen_p = sprintf((char *)buffer_p, "adc_value: %0.2f\r\n", adc_value);
		buflen_p = sprintf((char *)buffer_p, "Vout: %0.2f\r\n", Vout);
		HAL_UART_Transmit(&huart1, buffer_p, buflen_p, HAL_MAX_DELAY);
		HAL_Delay(100);
	}
	return 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	adc_value = HAL_ADC_GetValue(&hadc1);

	adc_volt = ((adc_value* 3.3) / 4095); //3.3 V on a 12 bit resolution

	Vout = adc_volt * 5.3545;

	if(Vref > Vout)
	{
		if(duty < 450)
			duty++;
	}
	if(Vref < Vout)
	{
		if(duty > 50)
			duty--;
	}

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	if(byte == '\n')
	{
		buffer_uart[buflen_uart++] = '\0';
		sscanf((const char *)buffer_uart, "%f", &Vref);
		buflen_uart = 0;
	}
	else
		buffer_uart[buflen_uart++] = byte;

	HAL_UART_Receive_IT(&huart1, &byte, 1);
}

void System_Init(void)
{
	HAL_Init();
	GPIO_Init();
	TIM1_Init();
	Clock_Init();
	ADC1_Init();
	TIM3_Init();
	UART1_Init();

}
void ADC1_Init(void){
	/* 1. Activar el reloj del periferico */
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*2. Configurar los pines */
	GPIO_InitTypeDef adc_pin = {0};
	adc_pin.Pin = GPIO_PIN_0;
	adc_pin.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &adc_pin);

	/*3. Habilitar la interrupcion*/

	/*4. Configuracion de alto nivel*/
	hadc1.Instance = ADC1;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
		Error_Handler();

	ADC_ChannelConfTypeDef channel_0 = {0};
	channel_0.Channel = ADC_CHANNEL_0;
	channel_0.Rank = 1;
	channel_0.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &channel_0) != HAL_OK)
		Error_Handler();
	HAL_ADC_Start(&hadc1);
}
void UART1_Init(void)
{
	/* Habilitar seÃ±ales de reloj */
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 2. Configuracion de bajo nivel */
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &tx_pin);
	/* 3. Habilitar interrupcion */
	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	/* 4. Configuracion de alto nivel */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.Mode = UART_MODE_TX_RX;
	if (HAL_UART_Init(&huart1) != HAL_OK)
		Error_Handler();
	// 5. Iniciar el periférico (todos aquellos que comienzan a trabajar y generan algun tipo de resultado)
	HAL_UART_Receive_IT(&huart1, &byte, 1);
}

void TIM3_Init(void)
{
	// 1. Activar el reloj del periférico
	__HAL_RCC_TIM3_CLK_ENABLE();

	// 2. Configurar los pines

	// 3. Habilitar la interrupción
	HAL_NVIC_SetPriority(TIM3_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	// 4. Configuracion de alto nivel
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 9;
	htim3.Init.Period = 999;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
		Error_Handler();
	//5. Iniciar el periférico
	HAL_TIM_Base_Start_IT(&htim3);
}

void Clock_Init(void)
{
	RCC_OscInitTypeDef osc_config = {0};
	osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_config.HSEState = RCC_HSE_ON;
	osc_config.HSIState = RCC_HSI_OFF;
	osc_config.LSEState = RCC_LSE_OFF;
	osc_config.LSIState = RCC_LSI_OFF;
	osc_config.PLL.PLLState = RCC_PLL_ON;
	osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc_config.PLL.PLLM = 8;
	osc_config.PLL.PLLN = 100;
	osc_config.PLL.PLLP = RCC_PLLP_DIV2;

	if(HAL_RCC_OscConfig(&osc_config) != HAL_OK)
		Error_Handler();

	RCC_ClkInitTypeDef clk_conf = {0};
	clk_conf.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
	| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_conf.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_conf.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_conf.APB1CLKDivider = RCC_HCLK_DIV2;
	clk_conf.APB2CLKDivider = RCC_HCLK_DIV1;

	if(HAL_RCC_ClockConfig(&clk_conf, FLASH_LATENCY_4) != HAL_OK)
	Error_Handler();
}

void TIM1_Init(void)
{
	// 1. Activar el reloj del periferico//
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//2. Configurar los pines(En este caso no la utilizamos)//
	GPIO_InitTypeDef pwm_pin = {0};
	pwm_pin.Pin = GPIO_PIN_7 | GPIO_PIN_8;
	pwm_pin.Mode = GPIO_MODE_AF_PP; //alternate function
	pwm_pin.Alternate = GPIO_AF1_TIM1;//
	HAL_GPIO_Init(GPIOA, &pwm_pin);
	// 3. Habilitar la interrupción.

	// 4. Configuracion de alto nivel.

	// 5. Activar el timer.
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.Period = 499;

	if(HAL_TIM_Base_Init(&htim1) != HAL_OK)
	Error_Handler();

	TIM_OC_InitTypeDef tim_channel = {0};
	tim_channel.OCMode = TIM_OCMODE_PWM1;
	tim_channel.OCPolarity = TIM_OCNPOLARITY_HIGH;
	tim_channel.Pulse = 0;//pwm duty cycle

	if(HAL_TIM_OC_ConfigChannel(&htim1, &tim_channel, TIM_CHANNEL_1) != HAL_OK)
	Error_Handler();

	TIM_BreakDeadTimeConfigTypeDef brk_conf = {0};
	brk_conf.DeadTime = computeDeadTime(100);
	if(HAL_TIMEx_ConfigBreakDeadTime(&htim1, &brk_conf) != HAL_OK)
	Error_Handler();

	// 5. Iniciar el periférico.
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}



uint16_t computeDeadTime(uint16_t dead_time)
{
	uint16_t death_time;
	float clockFreq;
	uint32_t sysClock = HAL_RCC_GetSysClockFreq();
	clockFreq = 1000000000.0 / sysClock;
	if(dead_time >=0 && dead_time <= 1764)
	death_time = dead_time / clockFreq;
	else if(dead_time >= 1778 && dead_time <= 3529)
	death_time = dead_time / (clockFreq * 2) + 64;
	else if(dead_time >= 3556 && dead_time <= 7001)
	death_time = dead_time / (clockFreq * 8) + 160;
	else if(dead_time >= 7112 && dead_time <= 14001)
	death_time = dead_time / (clockFreq * 16) + 192;
	else death_time = dead_time;
	//return death_time;
	return death_time;
}

void GPIO_Init(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef led_pin = {0};
	led_pin.Pin = GPIO_PIN_5;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &led_pin);
}

void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart1);
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void Error_Handler(void)
{
	while(1);
}


