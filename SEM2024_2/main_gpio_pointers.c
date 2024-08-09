#include <main.h>

int main(int argc, char *argv[])
{
		volatile uint32_t *GPIOA_MODER = 0x0, *GPIOA_ODR = 0x0;

		GPIOA_MODER = (uint32_t*)0x40020000;
		GPIOA_ODR = (uint32_t*)(0x40020000 + 0x14);

		//Enable clock signal
		__HAL_RCC_GPIOA_CLK_ENABLE();

		*GPIOA_MODER = *GPIOA_MODER | 0x400;
		*GPIOA_ODR = *GPIOA_ODR | 0x20;
		while(1);
	//return 0;
}
