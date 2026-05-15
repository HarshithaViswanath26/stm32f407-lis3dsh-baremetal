/*
 * lis3dsh_test.c
 *
 *  Created on: May 7, 2026
 *      Author: 49157
 */

#include "stm32f4xx.h"
#include "rcc.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"
#include "lis3dsh.h"

#include <stdint.h>
#include <string.h>


/*void delay()
{

	for(uint32_t i = 0; i < 500000; i++);
}*/
volatile uint8_t dataReady = 0;
int main(void)
{
	lis3dsh_Init();
	//lis3dsh_LED_Init();
	//lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x2F);  // BDU=0
	//uint8_t r3 = lis3dsh_Read_Reg(0xE8 & 0x7F);//lis3dsh_Read_Reg(LIS3DSH_CTRL_REG3);
	//uint8_t r1 = lis3dsh_Read_Reg(0x18);
	//uint8_t r2 = lis3dsh_Read_Reg(0x25);
	//uint8_t s1 = lis3dsh_Read_Reg(LIS3DSH_STATUS);
	lis3dsh_Read_XYZ();
	uint8_t s2 = lis3dsh_Read_Reg(LIS3DSH_WHO_AM_I);
	lis3dsh_INT1_Init();
	//uint8_t valReg = lis3dsh_Read_Reg(LIS3DSH_STATUS);

	//lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);
	//lis3dsh_Read_Reg(LIS3DSH_CTRL_REG4);
	//EXTI->EXTI_SWIER |= (1U << 0);
	while(1)
	{
		if(dataReady)
		{
			dataReady = 0;
			char msg[200];
			int16_t x = (int16_t)(((buffer[1] << 8) | buffer[0]));
			int16_t y = (int16_t)(((buffer[3] << 8) | buffer[2]));
			int16_t z = (int16_t)(((buffer[5] << 8) | buffer[4]));

			int32_t x_mg = (int32_t)(x*LIS3DSH_SENSITIVITY);
			int32_t y_mg = (int32_t)(y*LIS3DSH_SENSITIVITY);
			int32_t z_mg = (int32_t)(z*LIS3DSH_SENSITIVITY);

			sprintf(msg, "X: %ldmicrog -------  Y: %ldmicrog ------- Z: %ldmicrog\r\n", x_mg, y_mg, z_mg);
			USART_SendData(&usart2, (uint8_t*)msg, strlen(msg));


		}
	}

	return 0;
}


void EXTI0_IRQHandler()
{
	//NVIC_IRQHandling(GPIOPin0);
	lis3dsh_Read_XYZ();
	dataReady = 1;
	NVIC_IRQHandling(GPIOPin0);

}
