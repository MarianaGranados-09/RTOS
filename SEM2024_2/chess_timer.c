#include <main.h>
#include <stdio.h>


/*
 *
 */

//Defining each bit to turn on for each display for anode display

#define DISPLAY1 0x1000 //turns on bit 12 on port b
#define DISPLAY2 0x2000 //turns on bit 13 on port b
#define DISPLAY3 0x8000 //turns on bit 14 on port b
#define DISPLAY4 0x4000 //turns on bit 15 on port b


 /* DP to A is PB0 TO PB7
 *
 * AN0 IS PB12
 * AN1 IS PB13
 * AN2 IS PB14
 * AN3 IS PB15
 */


/*
 * callback function: dd->state = (dd->state+1) %4 to update status
 */


TIM_HandleTypeDef htim3 = {0};
UART_HandleTypeDef huart2 = {0};

uint8_t buflen, buffer[32];

volatile uint8_t player = 1; //player 1 starts the game

volatile uint8_t counter = 10;
volatile uint8_t counter_ms = 60;
volatile uint16_t digits[4] = {0};

uint8_t sequenceSeg[10] = {
		0b11111100, // 0
		0b01100000, // 1
		0b11011010, // 2
		0b11110010, // 3
		0b01100110, // 4
		0b10110110, // 5
		0b10111110, // 6
		0b11100000, // 7
		0b11111110, // 8
		0b11100110  // 9
};

int main(void)
{

	System_Init();

	while(1)
	{


		if(counter > 0) counter--;

		else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);

		HAL_Delay(1000);

	}

	return 0;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t disp_sel = 0; // Selección del display


    if (htim->Instance == TIM3) // Verificar si es TIM3
    {
    	CalcDigits(counter);

    	GPIOB->ODR &= ~(DISPLAY1 | DISPLAY2 | DISPLAY3 | DISPLAY4); //apagar displays primero
    	if(player == 1)
    	{
    		switch(disp_sel)
    		    	{
    		    	case 0:
    		    		GPIOB->ODR = DISPLAY1 | sequenceSeg[digits[3]];
    		    		break;
    		    	case 1:
    		    		GPIOB->ODR = DISPLAY2 | sequenceSeg[digits[2]];
    		    		break;
    		    	/*case 2:
    		    	    GPIOB->ODR = DISPLAY2 | sequenceSeg[digits[2]];
    		    	    break;
    		    	case 3:
    		    	    GPIOB->ODR = DISPLAY1 | sequenceSeg[digits[3]];
    		    	    break;*/
    		    	default:
    		    		GPIOB->ODR = DISPLAY4 | sequenceSeg[0];
    		    		break;
    	}
    	}
    	else{
			switch(disp_sel)
			{
			case 0:
				GPIOB->ODR = DISPLAY3 | sequenceSeg[digits[3]];
				break;
			case 1:
				GPIOB->ODR = DISPLAY4 | sequenceSeg[digits[2]];
				break;
			/*case 2:
				GPIOB->ODR = DISPLAY2 | sequenceSeg[digits[2]];
				break;
			case 3:
				GPIOB->ODR = DISPLAY1 | sequenceSeg[digits[3]];
				break;*/
			default:
				GPIOB->ODR = DISPLAY4 | sequenceSeg[0];
				break;

			}
    	}

    	disp_sel = (disp_sel + 1) % 2;




    	//GPIOB->ODR = DISPLAY4 | sequenceSeg[digits[0]];
    	//GPIOB->ODR = DISPLAY3 | sequenceSeg[digits[1]]; //displaying tens in display3




    	/*switch(disp_sel)
    	{
    	case 0:
    		 GPIOB->ODR = DISPLAY4 | sequenceSeg[digits[0]]; //displaying unit in display4
    		 break;
    	case 1:
    		 GPIOB->ODR = DISPLAY3 | sequenceSeg[digits[1]]; //displaying tens in display3
    		 break;
    	default:
    		GPIOB->ODR = DISPLAY4 | sequenceSeg[0];
    	}*/


    	//disp_sel = (disp_sel + 1) % 4;




        // Apagar todos los displays
        //GPIOB->ODR &= ~(DISPLAY1 | DISPLAY2 | DISPLAY3 | DISPLAY4);

        // Mostrar el dígito correspondiente en el display actual
        /*switch(disp_sel)
        {
            case 0:
                GPIOB->ODR = DISPLAY1 | sequenceSeg[digits[0]];
                break;
            case 1:
                GPIOB->ODR = DISPLAY2 | sequenceSeg[digits[1]];
                break;
            case 2:
                GPIOB->ODR = DISPLAY3 | sequenceSeg[digits[2]];
                break;
            case 3:
                GPIOB->ODR = DISPLAY4 | sequenceSeg[digits[3]];
                break;
            default:
                break;
        }

        // Pasar al siguiente display
        disp_sel = (disp_sel + 1) % 4;
        */
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	//only if its player's 1 turn may their button restart the counter
	if(player == 2)
	{
		//if pin6 port c is pressed, then player 1 has finished their turn
		if (GPIO_Pin == GPIO_PIN_6) {  // PC6 triggered
		     // Handle interrupt for PC6
		   	player = 1;
		    //reset counter
		   	counter = 10;
		}
    }
	//only if its player's 2 turn may their button restart the counter
	if(player == 1)
	{
		//if pin8 port c is pressed, then player 2 has finished their turn
		 if (GPIO_Pin == GPIO_PIN_8) {  // PC8 triggered
		        // Handle interrupt for PC8
		    	player = 2;
		    	//reset counter
		    	counter = 10;
		    }
	}

}

void CalcDigits(uint16_t counter)
{
	//units
	//digits[0] = counter_ms % 10;
	//tens
	//digits[1] = counter_ms / 10;

	digits[2] = counter % 10;
	digits[3] = counter / 10;

}




