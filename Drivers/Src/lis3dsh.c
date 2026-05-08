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

#include <stdint.h>
#include <stdio.h>

/* Pin configurations for LIS3DSH
		 * PA5 - SPI1 SCL
		 * PA7 - SPI1 MOSI
		 * PA6 - SPI1 MISO
		 * PE3 - SPI1 CSS
		 * Configure PE3 as normal GPIO pin
		 * Can drive it high or low based on comm requirements
		 */

static uint8_t spi1PinSet[] = {GPIOPin5,GPIOPin7,GPIOPin6};
static GPIO_Handle_t spi1Pins, spi1Css;


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
	pHandle->spiConfig.SPI_cpha = SPI_cpha0;
	pHandle->spiConfig.SPI_cpol = SPI_cpol0;
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
		lis3dsh_CommEnable();
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
	lis3dsh_CommEnable();
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
	lis3dsh_CommEnable();
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

void lis3dsh_Init()
{
	// Configure reg4,5 & reg3 when interrupts are to be enabled
	// configure reg4
	// ODR = 100Hz
	// BDU = 1
	// rest are to default val : see lis3dsh datasheet
	lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);

	// configure reg 5, anti-aliasing filter BW is 50Hz for this application
	lis3dsh_Write_Reg(LIS3DSH_CTRL_REG5, 0xC0);

	// configure reg 3 for interrupts
}

void lis3dsh_Read_XYZ()
{
		uint8_t buffer[6]; // to hold the low & high values of x,y,z
		buffer[0] = lis3dsh_Read_Reg(LIS3DSH_OUT_X_L);
		buffer[1] = lis3dsh_Read_Reg(LIS3DSH_OUT_X_H);
		buffer[2] = lis3dsh_Read_Reg(LIS3DSH_OUT_Y_L);
		buffer[3] = lis3dsh_Read_Reg(LIS3DSH_OUT_Y_H);
		buffer[4] = lis3dsh_Read_Reg(LIS3DSH_OUT_Z_L);
		buffer[5] = lis3dsh_Read_Reg(LIS3DSH_OUT_Z_H);

}
