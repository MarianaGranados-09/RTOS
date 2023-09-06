#include <main.h>

TIM_HandleTypeDef htim1 = {0};

int main (void)
{
	HAL_Init();
	Clock_Init();
	GPIO_Init();
	TIM1_Init();
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(250);
	}
	return 0;
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
	osc_config.PLL.PLLP = 4;
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
	htim1.Init.Period = 999; //10 kHZ
	// UI_Freq = CK_PSC / (Prescaler * Periodo)
	HAL_TIM_Base_Init(&htim1);

	//Configuración del canal
	TIM_OC_InitTypeDef oc_config = {0};
	oc_config.OCMode = TIM_OCMODE_PWM1;
	oc_config.OCNPolarity = TIM_OCPOLARITY_HIGH;
	oc_config.Pulse = 500;
	if (HAL_TIM_OC_ConfigChannel(&htim1, &oc_config, TIM_CHANNEL_1) != HAL_OK)
			Error_Handler();

	TIM_BreakDeadTimeConfigTypeDef dead_time ={0};
	dead_time.BreakState = TIM_BREAK_DISABLE;
	dead_time.DeadTime = computeDeadTime(500);
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &dead_time) != HAL_OK)
		Error_Handler();

	//5to Arrancar el periférico
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
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

void Error_Handler(void)
{
	while(1);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
