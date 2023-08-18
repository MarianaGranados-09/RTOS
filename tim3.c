#include <main.h>

TIM_HandleTypeDef htim3 = {0};

int main(void)
{
	HAL_Init();
	GPIO_Init();
	TIM3_Init();

	while(1)
	{
		HAL_Delay(50);

	}

	return 0;
}

void TIM3_IRQHandler(void)
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	HAL_TIM_IRQHandler(&htim3);
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
	htim3.Init.Prescaler = 7999; //cada 1000 cuentas se resetea 16M/16k

	////UI_Freq = CK_CNT //(1 + Period)
	//htim3.Init.Period = 250; //hasta donde llega el contador antes de desbordarse

	htim3.Init.Period = 999;
	//CAMBIAR PARA 1 SEG ENC, 1 SEG APA. 0.5Hz
	//UI_Freq = CLK_PSC / (Prescaler * Period)
	HAL_TIM_Base_Init(&htim3); //cargarle al timer la configuracion

	//5. Arrancar el periferico
	HAL_TIM_Base_Start_IT(&htim3); //Apuntador al periferico

}

void GPIO_Init(void)
{
	GPIO_InitTypeDef led_pin = {0};
	led_pin.Pin = GPIO_PIN_5;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &led_pin);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
