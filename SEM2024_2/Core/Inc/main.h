#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void UART2_Init(void);
void TIM2_Init(void);
void Error_Handler(void);
void GPIO_Init(void);
void Clock_Init(void);

void System_Init(void);

#endif /* __MAIN_H */
