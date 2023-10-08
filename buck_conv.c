#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLES	4
#define BUFFER_SIZE 64

TIM_HandleTypeDef htim1 = {0};
TIM_HandleTypeDef htim3 = {0};
UART_HandleTypeDef huart1 = {0};
ADC_HandleTypeDef hadc1 = {0};

uint8_t buflen, buffer1[64];

char buffer[BUFFER_SIZE];

volatile uint8_t idx = 0;
char byte;

uint16_t adc_value = 0, i, value, duty = 0;
uint32_t suma;

float Vout = 0.0;
float V_Res = 0.0;
float Vref = 5.0;

char mess[] = "UART startup\r\n";

int main (void)
{
	HAL_Init();
	Clock_Init();
	GPIO_Init();
	TIM1_Init();
	TIM3_Init();
	ADC1_Init();
	UART1_Init();

	HAL_UART_Transmit(&huart1, (uint8_t*)mess, strlen(mess), HAL_MAX_DELAY);
	//

	HAL_UART_Receive_IT(&huart1, (uint8_t*)&byte, 1);

	while(1)
	{
		for(i = 0, suma = 0; i < SAMPLES; i++)
			{
				adc_value = HAL_ADC_GetValue(&hadc1);
				suma+= adc_value;
				HAL_Delay(1);
			}

		value = suma >> 2;
		//V_Res = value * 0.0008056;
		//Vout = V_Res * 5.54545;


		//adc_value = HAL_ADC_GetValue(&hadc1);

		buflen = sprintf((char*)buffer1, "Vref: %0.2f, adc: %i, Vres: %0.2f Vout: %0.2f\r\n",Vref, value, V_Res, Vout);
		//buflen = sprintf((char*)buffer1, "%.2f \r\n", Vref);
		HAL_UART_Transmit(&huart1, buffer1, buflen, HAL_MAX_DELAY);
		//HAL_Delay(50);
	}
	return 0;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	//que timer se manda llamar dependiendo de la actividad
	if(htim -> Instance == TIM3) //en este caso el TIM3 es el que genera la interrupcion
	{
		value = HAL_ADC_GetValue(&hadc1);
		V_Res = value * 0.0008056;
		Vout = V_Res * 4.9454545;

		if(Vref > Vout)
		{
			if(duty < 450)
				duty++; //increment duty so Vout goes up to reach Vref
		}
		if(Vref < Vout)
		{
			if(duty > 0)
				duty--; //decrement duty so Vout goes down to reach Vref
		}

		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
	}
}

void Clock_Init(void)
{
	//type
	RCC_OscInitTypeDef osc_config = {0};

	//choose type of oscillator and turn off the other options
	osc_config.HSEState = RCC_HSE_ON; 	/* Bypass lo genera otro dispositivo.*/
	osc_config.HSIState = RCC_HSI_OFF;		/* Cristal externo de baja velocidad.*/
	osc_config.LSEState = RCC_LSE_OFF;		/* Cristal interno de alta velocidad.*/
	osc_config.LSIState = RCC_LSI_OFF;		/* Cristal interno de baja velocidad.*/
	osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;		/* Especificamos que cristal le vamos a asignar al manejador*/

	osc_config.PLL.PLLState = RCC_PLL_ON;	/* Activamos los PLL*/
	osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE; /*Especificamos el crsital que vamos a configurar / usar */
	osc_config.PLL.PLLM = 8;	/* Dividir entre 8 para tener 1KHz.*/
	osc_config.PLL.PLLN = 100;	/* Multiplicas por el doble de la frecuencia que necesitas.*/
	osc_config.PLL.PLLP = 2;
	if (HAL_RCC_OscConfig(&osc_config) != HAL_OK)
		Error_Handler();

	//HAL_RCC_MCOConfig(RRC_MCO_1, HSI, 5);

	/*Configuraciones de reloj (clock) en el microcontrolador*/
	RCC_ClkInitTypeDef clk_conf = {0};	//Definimos nuestro tipo de dato (Handler)
	clk_conf.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK	| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_conf.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_conf.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_conf.APB1CLKDivider = RCC_HCLK_DIV2;
	clk_conf.APB2CLKDivider = RCC_HCLK_DIV1;

	if(HAL_RCC_ClockConfig(&clk_conf, FLASH_LATENCY_4) != HAL_OK)
		Error_Handler();
}

//computeDeadTime function used to insert a deadtime between the complementary PWM signals
//to minimize the risk of two complementary transistors shorting to ground

//commonly used for half-bridges or full-bridges
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
	else
	    death_time = dead_time;
	return death_time;
}

void TIM1_Init(void)
{
	/*1er Habilitar el reloj del periférico*/
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*2do Configuración de bajo nivel (pines)*/
	GPIO_InitTypeDef PWM_Pin = {0};
	PWM_Pin.Pin = GPIO_PIN_7 | GPIO_PIN_8;
	PWM_Pin.Mode = GPIO_MODE_AF_PP;
	PWM_Pin.Alternate = GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOA, &PWM_Pin);

	//3ro Habilitación de la interrupción

	//4to Configuración de alto nivel
	htim1.Instance = TIM1;
	/* CK_CNT = CK_PSC / (1 + Prescaler)*/
	htim1.Init.Prescaler = 0;
	/* UI_Freq = CK_CNT / (1 + Periodo)*/
	htim1.Init.Period = 499; //10 kHZ
	// UI_Freq = CK_PSC / (Prescaler * Periodo)
	HAL_TIM_Base_Init(&htim1);

	//uint32_t pulso = 207; //for 5 V output
	//uint32_t pulso = 0;

	//Configuración del canal
	TIM_OC_InitTypeDef oc_config = {0};
	oc_config.OCMode = TIM_OCMODE_PWM1;
	//oc_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	oc_config.OCPolarity = TIM_OCPOLARITY_HIGH;
	//oc_config.OCNPolarity = TIM_OCNPOLARITY_LOW;
	oc_config.Pulse = 0;
	if (HAL_TIM_OC_ConfigChannel(&htim1, &oc_config, TIM_CHANNEL_1) != HAL_OK)
			Error_Handler();

	TIM_BreakDeadTimeConfigTypeDef dead_time ={0};
	dead_time.BreakState = TIM_BREAK_DISABLE;
	dead_time.DeadTime = computeDeadTime(100);
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &dead_time) != HAL_OK)
		Error_Handler();

	//5to Arrancar el periférico
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}

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
	htim3.Init.Prescaler = 4999;///4999;//999; //prescaler value for 100Mhz / (99 + 1) = 100kHz

	////UI_Freq = CK_CNT //(1 + Period)
	//htim3.Init.Period = 250; //hasta donde llega el contador antes de desbordarse

	htim3.Init.Period = 1000;//100; //period for 1 ms interrupt (100kHz / 1000 = 100Hz)
	if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
		Error_Handler();

	//5. Arrancar el periferico
	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_Base_Start_IT(&htim3); //Apuntador al periferico
}

void GPIO_Init(void)
{
	//Configuración LEDS
	GPIO_InitTypeDef led_pin = {0};
	led_pin.Pin = GPIO_PIN_5;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	__HAL_RCC_GPIOA_CLK_ENABLE(); /* Habilita el reloj del puerto A*/
	HAL_GPIO_Init(GPIOA, &led_pin);
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
	adc_chan.SamplingTime = ADC_SAMPLETIME_15CYCLES;
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(byte == '\n')
	{
		buffer[idx++] = '\0';
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, idx, HAL_MAX_DELAY);
		Vref = atof(buffer);
		idx = 0;
		//memset(buffer, 0, sizeof(buffer));

	}
	else
	{
		buffer[idx++] = byte;
	}

	//Vref = strtof((char*)buffer, NULL, 10); //convert string to long int


	HAL_UART_Receive_IT(&huart1,  (uint8_t*)&byte, 1); //start next receive


}

void Error_Handler(void)
{
	while(1);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM3_IRQHandler(void)
{
	//la funcion que ejecuta el timer debe de ser ejecutada por la funcion callback del timer3, no el IRQHandler
	//HAL_TIM_PeriodElapsedCallback
	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	HAL_TIM_IRQHandler(&htim3);
}

void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart1); //busca en los registros del periferico el motivo de la interrupcion
	//llamar las funciones callback
}


