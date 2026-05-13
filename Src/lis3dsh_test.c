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


/*void delay()
{

	for(uint32_t i = 0; i < 500000; i++);
}*/

int main(void)
{
	lis3dsh_Init();
	lis3dsh_INT1_Init();
	uint8_t valReg = lis3dsh_Read_Reg(LIS3DSH_STATUS);

	//lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);
	//lis3dsh_Read_Reg(LIS3DSH_CTRL_REG4);
	lis3dsh_Read_XYZ();
	while(1)
	{

	}

	return 0;
}
