#include <main.h>
#include <FreeRTOS.h>
#include <task.h>

UART_HandleTypeDef huart2 = {0};

void task1(void *);
void task2(void *);
void task3(void *);

typedef struct {
    TaskHandle_t *suspendTask;
    TaskHandle_t *resumeTask;
} TaskControl;

TaskHandle_t rojotaskh;
TaskHandle_t ambartaskh;
TaskHandle_t verdetaskh;

void suspend_and_resume(TaskControl *taskControl) {
    vTaskResume(*taskControl->resumeTask);
    vTaskSuspend(*taskControl->suspendTask);
}

int main(void)
{
	System_Init();

	xTaskCreate(task1, "t1", configMINIMAL_STACK_SIZE, NULL, 1, &rojotaskh);
	xTaskCreate(task2, "t2", configMINIMAL_STACK_SIZE, NULL, 1, &ambartaskh);
	xTaskCreate(task3, "t3", configMINIMAL_STACK_SIZE, NULL, 1, &verdetaskh);

	vTaskSuspend(verdetaskh);
	vTaskSuspend(ambartaskh);

	vTaskStartScheduler();

	return 0;
}

void task1(void *pvParameters)
{
	unsigned int i;
	GPIO_InitTypeDef led_pin = {0};
	UNUSED(pvParameters);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	led_pin.Pin = GPIO_PIN_0;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &led_pin);

	TaskControl task1Control = { &rojotaskh, &ambartaskh };

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		for(i = 0; i < 100; i++);
		suspend_and_resume(&task1Control);
	}
}

void task2(void *pvParameters)
{
	unsigned int i;
	GPIO_InitTypeDef led_pin = {0};
	UNUSED(pvParameters);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	led_pin.Pin = GPIO_PIN_1;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &led_pin);
  
	TaskControl task2Control = { &ambartaskh, &verdetaskh };

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
		for(i = 0; i < 100; i++);
		suspend_and_resume(&task2Control);
	}
}

void task3(void *pvParameters)
{
	unsigned int i;
	GPIO_InitTypeDef led_pin = {0};
	UNUSED(pvParameters);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	led_pin.Pin = GPIO_PIN_2;
	led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &led_pin);

	TaskControl task3Control = { &verdetaskh, &rojotaskh };

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
		for(i = 0; i < 100; i++);
		suspend_and_resume(&task3Control); 
	}
}
