#include <main.h>

//timer 4 canal 1
TIM_HandleTypeDef htim4 = {0};

int main(void)
{
	uint8_t dir = 0;
	uint8_t duty = 0;
	HAL_Init();
	TIM4_Init();

	while(1)
	{
		if(dir == 0)
		{
			duty++;
			if(duty >= 99)
			{
				dir = 1; //start decreasing duty cycle
			}
		}
		else{
			duty--;
			if(duty == 0){
				dir = 0; //increase duty cycle
			}
		}

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, duty);
		HAL_Delay(20);



	}

	return 0;
}

void Error_Handler(void)
{
	while(1);
}



void TIM4_Init(void)
{
	//1. Habilitar reloj para periferico
	__HAL_RCC_TIM4_CLK_ENABLE();
	//pin b6 para canal 1 tim4
	__HAL_RCC_GPIOB_CLK_ENABLE();


	//2. Configuracion de bajo nivel (pins que va a utilizar)
	GPIO_InitTypeDef oc_pin = {0};
	oc_pin.Pin = GPIO_PIN_6; //using pinb6 from alternate function mapping page 46
	oc_pin.Mode = GPIO_MODE_AF_PP; //alternate function push pull mode
	oc_pin.Alternate = GPIO_AF2_TIM4; //specifies which peripheral to associate to the pin
	HAL_GPIO_Init(GPIOB, &oc_pin);


	htim4.Instance = TIM4; //miembro que va a asociar a que timer va
	//CK_CNT = CK_PSC // (1 + Prescaler)
	htim4.Init.Prescaler = 15; //cada 1000 cuentas se resetea 16M/16k
	//16M/7999 = 2k
	////UI_Freq = CK_CNT //(1 + Period)
	//htim3.Init.Period = 250; //hasta donde llega el contador antes de desbordarse

	htim4.Init.Period = 99;
	//CAMBIAR PARA 1 SEG ENC, 1 SEG APA. 0.5Hz
	//UI_Freq = CLK_PSC / (Prescaler * Period)
	HAL_TIM_Base_Init(&htim4); //cargarle al timer la configuracion

	//5. Running the peripheral


	//producto del periodo*prescalador debe de ser 1600 (16M/10Khz)

	TIM_OC_InitTypeDef oc_config = {0};
	oc_config.OCMode = TIM_OCMODE_PWM1;
	oc_config.OCNPolarity = TIM_OCPOLARITY_HIGH;
	oc_config.Pulse = 50;//when timer reaches this value, the signal will alternate //50%

	if(HAL_TIM_PWM_ConfigChannel(&htim4, &oc_config, TIM_CHANNEL_1) != HAL_OK) //loading pin configuration for ch1
	{
		Error_Handler();
	}

	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

}


void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
