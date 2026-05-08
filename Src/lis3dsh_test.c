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




int main(void)
{
	lis3dsh_Init();
	lis3dsh_Read_XYZ();
	return 0;
}
