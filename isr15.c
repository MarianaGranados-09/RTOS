#include <main.h>

int main(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_Init();

	//clock enable
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	//config. led
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); //port on nucleo 401re

	//config. user button
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


	//config. NVIC
	//HAL_NVIC_SetPriority(EXTI0_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //INTERRUPT ROUTINE

	while(1)
	{
		HAL_Delay(50);

	}
	return 0;
	}

void EXTI15_10_IRQHandler(void)
{
	//HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
