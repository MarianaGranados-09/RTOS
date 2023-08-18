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

	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef btn_pin = {0};
	btn_pin.Pin = GPIO_PIN_13;
	btn_pin.Mode = GPIO_MODE_INPUT;
	btn_pin.Pull = GPIO_PULLDOWN;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &btn_pin);

	uint8_t value;
	uint8_t flag = 0;


	while(1)
	{
		value = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
		if(value == 0)
		{
			flag = !flag;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, flag);

			HAL_Delay(150);
		}

	}
	return 0;
}

void SysTick_Handler(void) //rutina de interrupcion
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
