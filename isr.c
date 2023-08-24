#include <main.h>

int main(void)
{
	HAL_Init();
	GPIO_InitTypeDef led_pin = {0}; //Initialization
	led_pin.Pin = GPIO_PIN_5; //define to use pin 11
	led_pin.Mode = GPIO_MODE_OUTPUT_PP; //set pin to output mode
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//HAL_GPIO_Init(GPIOC, &led_pin); //led port and pointer
	HAL_GPIO_Init(GPIOA, &led_pin); //port on nucleo 401re

	/*CONFIGURACION DEL PIN*/
	GPIO_InitTypeDef btn_pin = {0};
	btn_pin.Pin = GPIO_PIN_13;
	btn_pin.Mode = GPIO_MODE_IT_FALLING;
	btn_pin.Pull = GPIO_PULLUP;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &btn_pin);

	HAL_NVIC_SetPriority(EXTI0_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn); //INTERRUPT ROUTINE

	//uint8_t value;
	//uint8_t flag = 0;


	while(1)
	{
		HAL_Delay(50);

	}
	return 0;
	}

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); //necesario especificar el pin de la int
}

void SysTick_Handler(void) //rutina de interrupcion
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
