/*
 * usart_test.c
 *
 *  Created on: May 6, 2026
 *      Author: 49157
 */
#include "stm32f4xx.h"
#include "gpio.h"
#include "usart.h"
#include "rcc.h"

#include <string.h>

/*
 * Configure to use USART:
 * 1. Configure the pins as AF for USART
 * 2. Enable RCC
 * 3. Configure USART
 * 4. Send data
 * 5. On PC side, configure Putty with correct BR
 */
int main(void)
{
	GPIO_Handle_t usartTx, usartRx;
	USART_Handle_t usart2;

	usartTx.GPIOx = GPIOA;
	usartTx.pinConfig.GPIO_pinNum = GPIOPin2;
	usartTx.pinConfig.GPIO_mode = GPIO_alternate;
	usartTx.pinConfig.GPIO_pinAltFunMode= AF7;
	usartTx.pinConfig.GPIO_otype = GPIO_pushPull;
	usartTx.pinConfig.GPIO_pupdtype = GPIO_pu;
	usartTx.pinConfig.GPIO_ospeed= GPIO_high;

	usartRx.GPIOx = GPIOA;
	usartRx.pinConfig.GPIO_pinNum = GPIOPin3;
	usartRx.pinConfig.GPIO_mode = GPIO_alternate;
	usartRx.pinConfig.GPIO_pinAltFunMode= AF7;
	usartRx.pinConfig.GPIO_otype = GPIO_pushPull;
	usartRx.pinConfig.GPIO_pupdtype = GPIO_pu;
	usartRx.pinConfig.GPIO_ospeed= GPIO_high;

	usart2.pUSARTx = USART2;
	usart2.USART_config.USART_Mode = USART_MODE_TX;
	usart2.USART_config.USART_StopBits = USART_STOPBITS_1;
	usart2.USART_config.USART_WordLen = USART_WORDLEN_8bits;
	usart2.USART_config.USART_ParityCtrl = USART_DI_PARITY;
	usart2.USART_config.USART_HWFlowCtrl = USART_HWFlowCtrl_None;
	usart2.USART_config.USART_OverSampling = USART_OVER_16;
	usart2.USART_config.USART_BaudRate = USART_BAUD_115200;

	RCC_AHB1_Init(GpioA, ENABLE);
	GPIO_Init(&usartTx);
	GPIO_Init(&usartRx);

	RCC_APB1_Init(Usart2, ENABLE);
	USART_Init(&usart2);

	//extremely important!!!!!!!!!
	USART_EnableComm(USART2);

	uint8_t msg[] = "Good Luck\n";
	USART_SendData(&usart2, msg, strlen((char*)msg));

	return 0;
}


