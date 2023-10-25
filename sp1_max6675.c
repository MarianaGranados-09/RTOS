#include <main.h>
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart2 = {0}; //using uart 2 --Handler
SPI_HandleTypeDef hspi1 = {0}; //spi1 handler


//uint8_t buff_out[2] = {0x3A, 0xF5};

uint8_t temp_msb;
uint8_t temp_lsb;

uint8_t buflen, buffer[32];

float temperature = 0.0;

int main(void)
{
   System_Init();

   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

   HAL_SPI_Transmit(&hspi1, (uint8_t*)0x00, 1, HAL_MAX_DELAY);
   HAL_SPI_Receive(&hspi1, &temp_msb, 1, HAL_MAX_DELAY);
   HAL_SPI_Transmit(&hspi1, (uint8_t*)0x01, 1, HAL_MAX_DELAY);
   HAL_SPI_Receive(&hspi1, &temp_lsb, 1, HAL_MAX_DELAY);

   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  // temperature = (((temp_msb << 3) + (temp_lsb >> 5)) * 0.25);
   //temperature = (((temp_msb << 8) | temp_lsb) >> 3) * 0.25;
   //temperature = (((float)temp_msb << 8) | (float)temp_lsb) * 0.25;
   //Temp=((((DATARX[0]|DATARX[1]<<8)))>>3);               // Temperature Data Extraction
   //Temp*=0.25;

   temperature = (((temp_msb) | (temp_lsb << 8)) >> 3);
   temperature*=0.25;

   buflen = sprintf((char*)buffer,"%0.2f\r\n",temperature);
   HAL_UART_Transmit(&huart2, buffer, buflen, HAL_MAX_DELAY);

   HAL_Delay(1000);

   while (1)
       {

	   	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	      HAL_SPI_Transmit(&hspi1, (uint8_t*)0x00, 1, HAL_MAX_DELAY);
	      HAL_SPI_Receive(&hspi1, &temp_msb, 1, HAL_MAX_DELAY);
	      HAL_SPI_Transmit(&hspi1, (uint8_t*)0x01, 1, HAL_MAX_DELAY);
	      HAL_SPI_Receive(&hspi1, &temp_lsb, 1, HAL_MAX_DELAY);

	      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	      temperature = (((temp_msb) | (temp_lsb << 8)) >> 3);
	      temperature*=0.25;

	      buflen = sprintf((char*)buffer,"%0.2f C\r\n",temperature);
	      HAL_UART_Transmit(&huart2, buffer, buflen, HAL_MAX_DELAY);

	      HAL_Delay(1000);

       }
   return 0;
}

void System_Init(void)
{
	   HAL_Init();
	   Clock_Init();
	   GPIO_Init();
	   UART2_Init();
	   SPI1_Init();
}

void SPI1_Init(void)
{
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef spi_pin = {0};
	//spi_pin.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	spi_pin.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	spi_pin.Mode = GPIO_MODE_AF_PP;
	spi_pin.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &spi_pin);

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER; //microc as master device mode
	hspi1.Init.NSS = SPI_NSS_SOFT; //when using SOFT, it is necessary to set high and low the nss pin (pin_4) in the main func.
	//hspi1.Init.NSS = SPI_NSS_HARD_INPUT; //this line changes when transmitting or receiving data, OUTPUT OR INPUT, in each case
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB; //send the most significant bit first
	hspi1.Init.Direction = SPI_DIRECTION_2LINES; //use two lines, transmit and receive
	hspi1.Init.DataSize = SPI_DATASIZE_16BIT; //16-bit data stream
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE; //also revise this parameter
	hspi1.Init.CLKPolarity =  SPI_POLARITY_LOW;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;

	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;

	if(HAL_SPI_Init(&hspi1) != HAL_OK)
			Error_Handler();

}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2); //busca en los registros del periferico el motivo de la interrupcion
	//llamar las funciones callback
}


void UART2_Init(void)
{
	/* Habilitar seÃ±ales de reloj */
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 2. Configuracion de bajo nivel */
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	tx_pin.Mode = GPIO_MODE_AF_PP;
	tx_pin.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &tx_pin);
	/* 3. Habilitar interrupcion */
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/* 4. Configuracion de alto nivel */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.Mode = UART_MODE_TX_RX;
	if (HAL_UART_Init(&huart2) != HAL_OK)
		Error_Handler();

}

void Clock_Init(void)
{
   RCC_OscInitTypeDef osc_init = {0};
   osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE; //Configure oscillator type as HSE
   osc_init.HSEState = RCC_HSE_ON;
   osc_init.LSEState = RCC_LSE_OFF;
   osc_init.HSIState = RCC_HSI_OFF;
   osc_init.LSIState = RCC_LSI_OFF;
   osc_init.PLL.PLLState = RCC_PLL_ON; //Turn on PLL
   osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE; //Set PLL source as HSE

   //SYSCLK = HSE * (N / M)
   osc_init.PLL.PLLM = 8;
   osc_init.PLL.PLLN = 200;
   osc_init.PLL.PLLP = RCC_PLLP_DIV2; //2
   osc_init.PLL.PLLQ = 2;
   //osc_init.PLL.PLLR = 2;
   if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
      Error_Handler();

   RCC_ClkInitTypeDef clock_init = {0};
   clock_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   clock_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
   clock_init.AHBCLKDivider = RCC_SYSCLK_DIV1;

   clock_init.APB1CLKDivider = RCC_HCLK_DIV2;
   clock_init.APB2CLKDivider = RCC_HCLK_DIV1;

   if (HAL_RCC_ClockConfig(&clock_init, FLASH_LATENCY_4) != HAL_OK)
      Error_Handler();
}

void Error_Handler(void)
{
   while(1)
   {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      HAL_Delay(100);
   }
}

void GPIO_Init(void)
{
   /* Configurar pin PA5 como salida digital */
   __HAL_RCC_GPIOA_CLK_ENABLE();

   GPIO_InitTypeDef led_pin = {0};
   led_pin.Pin = GPIO_PIN_5;
   led_pin.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOA, &led_pin);

   //only necessary when using the SOFT MODE for the NSS pin

    GPIO_InitTypeDef cse_pin = {0};
   	cse_pin.Pin = GPIO_PIN_4;
   	cse_pin.Mode = GPIO_MODE_OUTPUT_PP;
   	HAL_GPIO_Init(GPIOA, &cse_pin);

}

void SysTick_Handler(void)
{
   HAL_IncTick();
   HAL_SYSTICK_IRQHandler();
}
