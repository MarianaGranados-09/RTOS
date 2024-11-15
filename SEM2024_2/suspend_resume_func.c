#include <main.h>
#include <FreeRTOS.h>
#include <task.h>

UART_HandleTypeDef huart2 = {0};

void task1(void *);
void task2(void *);
void task3(void *);

TaskHandle_t rojotaskh;//administrar tarea con manejador de tarea
TaskHandle_t ambartaskh;
TaskHandle_t verdetaskh;

//funcion que toma como argumentos dos manejadores de tareas, en este caso le
//pasamos la tarea que queremos que resuma y la tarea que queremos que suspenda mediante un apuntador
void suspend_resume(TaskHandle_t *suspendTask, TaskHandle_t *resumeTask)
{
	vTaskResume(*resumeTask);
	vTaskSuspend(*suspendTask);
}

int main(void)
{

	System_Init();

	xTaskCreate(task1, "t1", configMINIMAL_STACK_SIZE, NULL, 1, &rojotaskh); //la tarea queda enlazada con el manejador, al usar &rojotaskh
	xTaskCreate(task2, "t2", configMINIMAL_STACK_SIZE, NULL, 1, &ambartaskh);
	xTaskCreate(task3, "t3", configMINIMAL_STACK_SIZE, NULL, 1, &verdetaskh);
	//las tareas comienzan encendidas, por lo que es necesario suspenderlas antes de iniciar,
	//se suspende la verde y ambar y asi inicia la tarea del led rojo
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

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		for(i = 0; i < 100; i++);
		suspend_resume(&rojotaskh, &ambartaskh);
	}
	//vTaskDelete(NULL);
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

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
		for(i = 0; i < 100; i++);
		suspend_resume(&ambartaskh, &verdetaskh);
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

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
		for(i = 0; i < 100; i++);
		suspend_resume(&verdetaskh, &rojotaskh);
	}
}






