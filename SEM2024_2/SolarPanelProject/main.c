#include <main.h>
#include <lcd_display.h>


//I2C TO MANAGE INA219
//UART TO SEND VALUES OF CURRENT, VOLTAGE AND POWER TO THE SERIAL TERMINAL
//TIM3 TO ESTABLISH WHEN THE VALUES ARE UPDATED (EACH 2S)
//TIM4 TO GENERATE PWM TO DRIVE THE IRLZ44N MOSFET



I2C_HandleTypeDef hi2c1 = {0}; //using i2c 1 handler
UART_HandleTypeDef huart2 = {0}; //using uart 2 -- handler
TIM_HandleTypeDef htim3 = {0}; //using timer3 -- handler
TIM_HandleTypeDef htim4 = {0}; //using timer4 -- handler

uint16_t duty = 0;
float input_volt, current, powerW;



int main(void)
{

	uint8_t buflen, bufout[128], bufoutvolt[128], bufoutcurrent[128], bufoutpower[128], bufoutduty[128];
	uint8_t buffin[2], buffer[2];

	uint8_t buflenvolt, buflencurrent, buflenpower, buflenduty;

	const char *format = "Volt(V): %.3f\nCurrent(A): %.3f\nPower(W):%.3f\n";

	const char *formatvolt = "Volt: %.2f V";
	const char *formatcurrent = "Current: %.2f A";
	const char *formatpower = "Power: %.2f W";
	const char *formatduty = "Duty Cycle: %u %%";

	const char *mssg = "Startup";

   System_Init();
   INA219_Init();
   lcd_init(&hi2c1);
   lcd_clear(&hi2c1);
   lcd_set_cursor(&hi2c1, 0, 0);
   lcd_send_string(&hi2c1, mssg, strlen(mssg));


   while(1)
   {

	   buffin[0] = 0x01;
	   HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, buffin, 1, HAL_MAX_DELAY);
	   HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, buffer, 2, HAL_MAX_DELAY);
	   int16_t shunt_voltage_raw = (((int16_t)buffer[0]) << 8) + buffer[1];
	   float shunt_voltage = shunt_voltage_raw * 0.009765625;

	   buffin[0] = 0x02;
	   HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, buffin, 1, HAL_MAX_DELAY);
	   HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, buffer, 2, HAL_MAX_DELAY);
	   int16_t bus_voltage_raw = (((int16_t)buffer[0]) << 8) + buffer[1];
	   float bus_voltage = bus_voltage_raw * 0.0004993623;

	   buffin[0] = 0x03;
	   HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, buffin, 1, HAL_MAX_DELAY);
	   HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, buffer, 2, HAL_MAX_DELAY);
	   int16_t power_raw = (((int16_t)buffer[0]) << 8) + buffer[1];
	   powerW = power_raw * 0.002;

	   buffin[0] = 0x03;
	   HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, buffin, 1, HAL_MAX_DELAY);
	   HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, buffer, 2, HAL_MAX_DELAY);
	   int16_t current_raw = (((int16_t)buffer[0]) << 8) + buffer[1];
	   current = current_raw * 0.0001;

	   input_volt = bus_voltage + shunt_voltage * 0.001;
	   buflen = sprintf((char *) bufout, format, input_volt, current, powerW);
	   HAL_UART_Transmit(&huart2, bufout, buflen, HAL_MAX_DELAY);


	   buflenvolt = sprintf((char *)bufoutvolt, formatvolt, input_volt);
	   buflencurrent = sprintf((char *)bufoutcurrent, formatcurrent, current);
	   buflenpower = sprintf((char *)bufoutpower, formatpower, powerW);
	   buflenduty  =  sprintf((char *)bufoutduty, formatduty, duty);

	   //sprintf(buffer_current, "%.2f", current);
	  // sprintf(buffer_power, "%.2f", powerW);*/

	   lcd_clear(&hi2c1);

	   lcd_set_cursor(&hi2c1, 0, 0);
	   lcd_send_string(&hi2c1, (char *)bufoutvolt, buflenvolt);

	   lcd_set_cursor(&hi2c1, 1, 0);
	   lcd_send_string(&hi2c1, (char *)bufoutcurrent, buflencurrent);

	   HAL_Delay(1800);

	   lcd_clear(&hi2c1);

	   lcd_set_cursor(&hi2c1, 0, 0);
	   lcd_send_string(&hi2c1, (char *)bufoutpower, buflenpower);

	   lcd_set_cursor(&hi2c1, 1, 0);
	   lcd_send_string(&hi2c1, (char *)bufoutduty, buflenduty);



	   HAL_Delay(2000);

   }
   return 0;
}

//called each time the tim3 counter overflows, here we will execute the update of the values
//on the LCD - or the serial terminal

/*void HAL_TIM_PeriodElaspedCallback(TIM_Base_InitTypeDef *htim)
{


}*/



