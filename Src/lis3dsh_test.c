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

#include <stdint.h>


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

int main(void)
{
	/* Pin configurations for LIS3DSH
	 * PA5 - SPI1 SCL
	 * PA7 - SPI1 MOSI
	 * PA6 - SPI1 MISO
	 * PE3 - SPI1 CSS
	 * Configure PE3 as normal GPIO pin
	 * Can drive it high or low based on comm requirements
	 */

	uint8_t spi1PinSet[] = {GPIOPin5,GPIOPin7,GPIOPin6};
	GPIO_Handle_t spi1Pins, spi1Css;

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
	SPI_SSOEConfig(SPI1, ENABLE);
	SPI_CommunicationControl(SPI1, ENABLE);

	// CSS PE3 pin is pulled Low
	spi1Css.GPIOx->GPIO_ODR &= ~(1U << 3);

	uint8_t dataSent = 0x8F;
    SPI_SendData(SPI1, &dataSent, sizeof(dataSent));
    dataSent = 0x00; // dummyData;
    SPI_SendData(SPI1, &dataSent, sizeof(dataSent));

    // CSS PE3 pin is pulled High
    spi1Css.GPIOx->GPIO_ODR |= (1U << 3);


	return 0;
}
