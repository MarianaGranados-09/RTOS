#ifndef __MAIN_H
#define __MAIN_H


#include "stm32f4xx_hal.h"
#include <stdio.h>

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

void UART1_Init(void);
void GPIO_Init(void);
void ADC_Init(void);
void Error_Handler(void);
void System_Init(void);

#endif /* __MAIN_H */
