/*
 * lis3dsh.c
 *
 *  Created on: May 5, 2026
 *      Author: 49157
 */

#include "lis3dsh.h"
#include "stm32f4xx.h"
#include "rcc.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"
#include "exti.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

USART_Handle_t usart2;
uint8_t buffer[6];


static uint8_t spi1PinSet[] = {GPIOPin5,GPIOPin7,GPIOPin6};
static GPIO_Handle_t spi1Pins, spi1Css;
static GPIO_Handle_t ledGreen, ledOrange, ledRed, ledBlue;


/* Pin configurations for LIS3DSH
		 * PA5 - SPI1 SCL
		 * PA7 - SPI1 MOSI
		 * PA6 - SPI1 MISO
		 * PE3 - SPI1 CSS
		 * Configure PE3 as normal GPIO pin
		 * Can drive it high or low based on comm requirements
		 */



static void SPI_GPIOInit(GPIO_Handle_t* pSPIPin, GPIO_RegDef_t* pGPIOx, uint8_t* pinset, uint8_t AFmode)
{
	pSPIPin->GPIOx = pGPIOx;
	pSPIPin->pinConfig.GPIO_mode = GPIO_alternate;
	pSPIPin->pinConfig.GPIO_pinAltFunMode = AFmode;
	pSPIPin->pinConfig.GPIO_otype = GPIO_pushPull;
	pSPIPin->pinConfig.GPIO_pupdtype= GPIO_noPupd;
	pSPIPin->pinConfig.GPIO_ospeed = GPIO_high;

	for(uint8_t i = 0; i < sizeof(pinset); i++)
	{
		pSPIPin->pinConfig.GPIO_pinNum = pinset[i];
		GPIO_Init(pSPIPin);
	}
}

static void SPI1_Init(SPI_Handle_t* pHandle)
{
	pHandle->pSPIx = SPI1;
	pHandle->spiConfig.SPI_mstr = SPI_master;
	pHandle->spiConfig.SPI_busConfig = SPI_FULLDUPLEX_MODE;
	pHandle->spiConfig.SPI_cpha = SPI_cpha1;   //lis3dsh has these configs
	pHandle->spiConfig.SPI_cpol = SPI_cpol1;   //lis3dsh has these configs
	pHandle->spiConfig.SPI_dff = SPI_dframe8;
	pHandle->spiConfig.SPI_ssm = SPI_ssmDi;
	//pHandle.SPIConfig.SPI_SSI = SPI_SSI_DI;
	pHandle->spiConfig.SPI_sclSpeed = SPI_PRESCALAR8;

	SPI_Init(pHandle);
	SPI_SSOEConfig(SPI1, ENABLE);
	SPI_CommunicationControl(SPI1, ENABLE);

}

static void lis3dsh_CommEnable()
{

		RCC_AHB1_Init(GpioA, ENABLE);
		SPI_GPIOInit(&spi1Pins, GPIOA, spi1PinSet, AF5);

		RCC_AHB1_Init(GpioE, ENABLE);
		spi1Css.GPIOx = GPIOE;
		spi1Css.pinConfig.GPIO_mode = GPIO_output;
		spi1Css.pinConfig.GPIO_pinNum = GPIOPin3;
		spi1Css.pinConfig.GPIO_ospeed = GPIO_high;
		spi1Css.pinConfig.GPIO_otype = GPIO_pushPull;
		spi1Css.pinConfig.GPIO_pupdtype = GPIO_pu;

		GPIO_Init(&spi1Css);

		RCC_APB2_Init(Spi1, ENABLE);
		SPI_Handle_t spi1;
		SPI1_Init(&spi1);

}

static void lis3dsh_CSS_Enable()
{
	// CSS PE3 pin is pulled Low
	spi1Css.GPIOx->GPIO_ODR &= ~(1U << 3);
}

static void lis3dsh_CSS_Disable()
{
	// CSS PE3 pin is pulled High
	spi1Css.GPIOx->GPIO_ODR |= (1U << 3);
}

void lis3dsh_Read_WHO_AM_I()
{
		//lis3dsh_CommEnable();
		// CSS PE3 pin is pulled Low
		lis3dsh_CSS_Enable();

		uint8_t dataReceived;
		uint8_t dataSent = 0x8F;
	    SPI_SendData(SPI1, &dataSent, sizeof(dataSent));
	    SPI_ReceiveData(SPI1, &dataReceived, sizeof(dataReceived));


	    dataSent = 0x00; // dummyData;
	    SPI_SendData(SPI1, &dataSent, sizeof(dataSent));
	    SPI_ReceiveData(SPI1, &dataReceived, sizeof(dataReceived));


	    // CSS PE3 pin is pulled High
	    lis3dsh_CSS_Disable();

}

uint8_t lis3dsh_Read_Reg(uint8_t regName)
{
	lis3dsh_CSS_Enable();

	uint8_t regNameSent = (regName | LIS3DSH_READ);
	uint8_t regVal;
	SPI_SendData(SPI1, &regNameSent, sizeof(regNameSent));
	SPI_ReceiveData(SPI1, &regVal, sizeof(regVal));

	regNameSent = LIS3DSH_DUMMY_DATA;
	SPI_SendData(SPI1, &regNameSent, sizeof(regNameSent));
	SPI_ReceiveData(SPI1, &regVal, sizeof(regVal));

	lis3dsh_CSS_Disable();

	return regVal;

}

void lis3dsh_Write_Reg(uint8_t regName, uint8_t val)
{
	lis3dsh_CSS_Enable();

	uint8_t writeData = (regName | LIS3DSH_WRITE);
	uint8_t flushData;
	SPI_SendData(SPI1, &writeData, sizeof(writeData));
	SPI_ReceiveData(SPI1, &flushData, sizeof(flushData));

	writeData = val;
	SPI_SendData(SPI1, &writeData, sizeof(writeData));
	SPI_ReceiveData(SPI1, &flushData, sizeof(flushData));

	lis3dsh_CSS_Disable();
}

static void lis3dsh_USART_GpioInit()
{
	GPIO_Handle_t usartTx;
	usartTx.GPIOx = GPIOA;
	usartTx.pinConfig.GPIO_pinNum = GPIOPin2;
	usartTx.pinConfig.GPIO_mode = GPIO_alternate;
	usartTx.pinConfig.GPIO_pinAltFunMode= AF7;
	usartTx.pinConfig.GPIO_otype = GPIO_pushPull;
	usartTx.pinConfig.GPIO_pupdtype = GPIO_pu;
	usartTx.pinConfig.GPIO_ospeed= GPIO_high;

	RCC_AHB1_Init(GpioA, ENABLE);
	GPIO_Init(&usartTx);

}

static void lis3dsh_Display_USARTInit(USART_Handle_t* usartx)
{
	usartx->pUSARTx = USART2;
	usartx->USART_config.USART_Mode = USART_MODE_TX;
	usartx->USART_config.USART_StopBits = USART_STOPBITS_1;
	usartx->USART_config.USART_WordLen = USART_WORDLEN_8bits;
	usartx->USART_config.USART_ParityCtrl = USART_DI_PARITY;
	usartx->USART_config.USART_HWFlowCtrl = USART_HWFlowCtrl_None;
	usartx->USART_config.USART_OverSampling = USART_OVER_16;
	usartx->USART_config.USART_BaudRate = USART_BAUD_115200;

	RCC_APB1_Init(Usart2, ENABLE);
	USART_Init(usartx);

	//extremely important!!!!!!!!!
	USART_EnableComm(USART2);

}


void lis3dsh_Init()
{

	    lis3dsh_CommEnable();

	    // soft reset — clears all registers to default
	    lis3dsh_Write_Reg(LIS3DSH_CTRL_REG3, 0x01);  // STRT bit = 1, resets state machine
	    // small delay to let reset complete
	    for(volatile uint32_t i = 0; i < 10000; i++);

	    lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);
	    lis3dsh_Write_Reg(LIS3DSH_CTRL_REG5, 0xC0);
	    lis3dsh_Write_Reg(LIS3DSH_CTRL_REG6, 0x00);  // explicitly disable FIFO
	    lis3dsh_USART_GpioInit();
	    lis3dsh_Display_USARTInit(&usart2);
	    lis3dsh_Write_Reg(LIS3DSH_CTRL_REG3, 0xC8);  // INT1 enable, IEA=1, DR_EN

	/*lis3dsh_CommEnable();
	// Configure reg4,5 & reg3 when interrupts are to be enabled
	// configure reg4
	// ODR = 100Hz
	// BDU = 1
	// rest are to default val : see lis3dsh datasheet
	lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);

	// configure reg 5, anti-aliasing filter BW is 50Hz for this application
	lis3dsh_Write_Reg(LIS3DSH_CTRL_REG5, 0xC0);


	lis3dsh_USART_GpioInit();
	lis3dsh_Display_USARTInit(&usart2);


	// configure reg 3 for interrupts
	lis3dsh_Write_Reg(LIS3DSH_CTRL_REG3, 0xC8);*/
}

void lis3dsh_LED_Init()
{
	RCC_AHB1_Init(GpioD, ENABLE);

	ledGreen.GPIOx = GPIOD;
	ledGreen.pinConfig.GPIO_mode = GPIO_output;
	ledGreen.pinConfig.GPIO_ospeed = GPIO_high;
	ledGreen.pinConfig.GPIO_otype = GPIO_pushPull;
	ledGreen.pinConfig.GPIO_pinNum = GPIOPin12;
	ledGreen.pinConfig.GPIO_pupdtype = GPIO_noPupd;

	ledOrange.GPIOx = GPIOD;
	ledOrange.pinConfig.GPIO_mode = GPIO_output;
	ledOrange.pinConfig.GPIO_ospeed = GPIO_high;
	ledOrange.pinConfig.GPIO_otype = GPIO_pushPull;
	ledOrange.pinConfig.GPIO_pinNum = GPIOPin13;
	ledOrange.pinConfig.GPIO_pupdtype = GPIO_noPupd;


	ledRed.GPIOx = GPIOD;
	ledRed.pinConfig.GPIO_mode = GPIO_output;
	ledRed.pinConfig.GPIO_ospeed = GPIO_high;
	ledRed.pinConfig.GPIO_otype = GPIO_pushPull;
	ledRed.pinConfig.GPIO_pinNum = GPIOPin14;
	ledRed.pinConfig.GPIO_pupdtype = GPIO_noPupd;


	ledBlue.GPIOx = GPIOD;
	ledBlue.pinConfig.GPIO_mode = GPIO_output;
	ledBlue.pinConfig.GPIO_ospeed = GPIO_high;
	ledBlue.pinConfig.GPIO_otype = GPIO_pushPull;
	ledBlue.pinConfig.GPIO_pinNum = GPIOPin15;
	ledBlue.pinConfig.GPIO_pupdtype = GPIO_noPupd;

	GPIO_Init(&ledRed);
	GPIO_Init(&ledBlue);
	GPIO_Init(&ledOrange);
	GPIO_Init(&ledGreen);

	GPIO_Led_ON(&ledBlue);
	GPIO_Led_ON(&ledRed);
	GPIO_Led_ON(&ledOrange);
	GPIO_Led_ON(&ledGreen);
}

void lis3dsh_Green_ON()
{
	GPIO_Led_OFF(&ledBlue);
	GPIO_Led_OFF(&ledOrange);
	GPIO_Led_OFF(&ledRed);
	GPIO_Led_ON(&ledGreen);

}

void lis3dsh_Blue_ON()
{
	GPIO_Led_ON(&ledBlue);
	GPIO_Led_OFF(&ledOrange);
	GPIO_Led_OFF(&ledRed);
	GPIO_Led_OFF(&ledGreen);

}

void lis3dsh_Orange_ON()
{
	GPIO_Led_OFF(&ledBlue);
	GPIO_Led_ON(&ledOrange);
	GPIO_Led_OFF(&ledRed);
	GPIO_Led_OFF(&ledGreen);

}

void lis3dsh_Red_ON()
{
	GPIO_Led_OFF(&ledBlue);
	GPIO_Led_OFF(&ledOrange);
	GPIO_Led_ON(&ledRed);
	GPIO_Led_OFF(&ledGreen);

}


void lis3dsh_Read_XYZ()
{
		/*lis3dsh_LED_Init();
		GPIO_Led_ON(&ledBlue);
		GPIO_Led_ON(&ledOrange);
		GPIO_Led_ON(&ledRed);
		GPIO_Led_ON(&ledGreen);*/


		buffer[0] = lis3dsh_Read_Reg(LIS3DSH_OUT_X_L);
		buffer[1] = lis3dsh_Read_Reg(LIS3DSH_OUT_X_H);
		buffer[2] = lis3dsh_Read_Reg(LIS3DSH_OUT_Y_L);
		buffer[3] = lis3dsh_Read_Reg(LIS3DSH_OUT_Y_H);
		buffer[4] = lis3dsh_Read_Reg(LIS3DSH_OUT_Z_L);
		buffer[5] = lis3dsh_Read_Reg(LIS3DSH_OUT_Z_H);

		uint8_t b0 = buffer[0];
		uint8_t b1 = buffer[1];
		uint8_t b2 = buffer[2];
		uint8_t b3 = buffer[3];
		uint8_t b4 = buffer[4];
		uint8_t b5 = buffer[5];

		//lis3dsh_Read_Reg(LIS3DSH_STATUS);  // force DR clear
}

void lis3dsh_INT1_Init()
{
	// see schematics: INT1 occurs on PE0
	// Configure PE0 to process the interrupt
	GPIO_Handle_t int1Pin;

	RCC_AHB1_Init(GpioE, ENABLE);

	int1Pin.GPIOx = GPIOE;
	int1Pin.pinConfig.GPIO_mode = GPIO_input;
	int1Pin.pinConfig.GPIO_pupdtype = GPIO_noPupd; // LIS3DSH interrupt output is active high, push-pull by default.
	int1Pin.pinConfig.GPIO_pinNum = GPIOPin0;
	GPIO_Init(&int1Pin);

	Interrupt_Handle_t ext0;

	RCC_APB2_Init(Syscfg, ENABLE);
	ext0.Exti_triggerMode = EXTI_RISING_TRIGGER_ONLY;  // why rising??
	ext0.pGpioHandle = &int1Pin;
	Interrupt_Init(&ext0);

	NVIC_IRQConfig(EXTI0, ENABLE);
	NVIC_IRQPriorityConfig(EXTI0, EXTI0_PRIORITY);

	//uint8_t pendVal = EXTI_IntStatus(GPIOPin0);
}
