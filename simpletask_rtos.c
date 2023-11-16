#include <main.h>
#include <FreeRTOS.h>
#include <task.h>

void blinkTask(void *);

int main(void)
{

	xTaskCreate(blinkTask, "Blinky", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();

	return 0;
}

void blinkTask(void *pvParameters)
{
	UNUSED(pvParameters);
	GPIO_InitTypeDef led_pin = {0};
	led_pin.Pin = GPIO_PIN_5;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &led_pin);
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		//HAL_Delay(250);
		vTaskDelay(pdMS_TO_TICKS(100));//converts to ticks of operating system depending on the freq.
	}
}

