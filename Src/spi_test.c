/*
 * spi_test.c
 *
 *  Created on: May 6, 2026
 *      Author: 49157
 */

#include "stm32f4xx.h"
#include "rcc.h"
#include "gpio.h"
#include "spi.h"

#include <string.h>

/* SPI1 pinset
 * PA5 - SCK
 * PA7- MOSI
 * PA6 - MISO
 * PA4 - NSS
 */


/* SPI2 pinset
 * PB13 - SCK
 * PB15 - MOSI
 * PB14 - MISO
 * PB12 - NSS
 */

void SPI_GPIOInit(GPIO_Handle_t* pSPIPin, GPIO_RegDef_t* pGPIOx, uint8_t* pinset, uint8_t AFmode)
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

void SPI1_Init(void)
{
	SPI_Handle_t spi1;

	spi1.pSPIx = SPI1;
	spi1.spiConfig.SPI_mstr = SPI_master;
	spi1.spiConfig.SPI_busConfig = SPI_FULLDUPLEX_MODE;
	spi1.spiConfig.SPI_cpha = SPI_cpha0;
	spi1.spiConfig.SPI_cpol = SPI_cpol0;
	spi1.spiConfig.SPI_dff = SPI_dframe8;
	spi1.spiConfig.SPI_ssm = SPI_ssmDi;
	//spi1.SPIConfig.SPI_SSI = SPI_SSI_DI;
	spi1.spiConfig.SPI_sclSpeed = SPI_PRESCALAR8;

	SPI_Init(&spi1);
}

void SPI2_Init(void)
{
	SPI_Handle_t spi2;

	spi2.pSPIx = SPI2;
	spi2.spiConfig.SPI_mstr = SPI_slave;
	spi2.spiConfig.SPI_busConfig = SPI_FULLDUPLEX_MODE;
	spi2.spiConfig.SPI_cpha = SPI_cpha0;
	spi2.spiConfig.SPI_cpol = SPI_cpol0;
	spi2.spiConfig.SPI_dff = SPI_dframe8;
	spi2.spiConfig.SPI_ssm = SPI_ssmDi;
	//spi2.SPIConfig.SPI_SSI = SPI_SSI_DI;
	spi2.spiConfig.SPI_sclSpeed = SPI_PRESCALAR8;

	SPI_Init(&spi2);
}

int main(void)
{
	GPIO_Handle_t spi1pins, spi2pins;
	uint8_t spi1Pinset[] = {5, 7, 4};
	uint8_t spi2Pinset[] = {13, 15, 12};

	RCC_AHB1_Init(GpioA, ENABLE);
	RCC_AHB1_Init(GpioB, ENABLE);
	SPI_GPIOInit(&spi1pins, GPIOA, spi1Pinset, AF5);
	SPI_GPIOInit(&spi2pins, GPIOB, spi2Pinset, AF5);

	RCC_APB2_Init(Spi1, ENABLE);
	RCC_APB1_Init(Spi2, ENABLE);
	// Master
	SPI1_Init();
	SPI_SSOEConfig(SPI1, ENABLE);
	// Slave
	SPI2_Init();

	SPI_CommunicationControl(SPI1, ENABLE);

	SPI_CommunicationControl(SPI2, ENABLE);

	// SPI communication
	uint8_t userData = 0x05;
	uint8_t dataLen = sizeof(userData);
	SPI_SendData(SPI1, &dataLen, 1); // slave does not know the length of data
	SPI_SendData(SPI1, &userData, sizeof(userData));
	// read data from DR as Master to avoid OVR flag error

	//while(SPI_GetFlagStatus(SPI1, SPI_BUSY_FLAG));
	//SPI_PeriClockControl(SPI1, DISABLE);


	return 0;
}
