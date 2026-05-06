/*
 * spi.c
 *
 *  Created on: Apr 30, 2026
 *      Author: 49157
 */

#include "spi.h"

void SPI_Init(SPI_Handle_t* pHandle)
{
	// configure dff
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_dff << 11);

	//configure mode
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_mstr << 2);

	//configure clk phase
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_cpha << 0);

	//configure clk polarity
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_cpol << 1);

	//configure SW slave management
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_ssm << 9);

	//configure baudrate
	pHandle->pSPIx->SPI_CR1 |= (pHandle->spiConfig.SPI_sclSpeed << 3);

	//configure bus configs
	if(pHandle->spiConfig.SPI_busConfig == SPI_SIMPLEX_RX_ONLY)
	{
		// simplex RX only
		pHandle->pSPIx->SPI_CR1 |= (SPI_FULLDUPLEX_MODE << 15);
		pHandle->pSPIx->SPI_CR1 |= (SPI_SIMPLEX_RX_ONLY << 10);
	}
	else if(pHandle->spiConfig.SPI_busConfig == SPI_FULLDUPLEX_MODE)
		pHandle->pSPIx->SPI_CR1 |= (SPI_FULLDUPLEX_MODE << 15);

	else if(pHandle->spiConfig.SPI_busConfig == SPI_HALFDUPLEX_RX_ONLY)
	{
		pHandle->pSPIx->SPI_CR1 |= (SPI_HALFDUPLEX_MODE << 15);
		pHandle->pSPIx->SPI_CR1 |= (SPI_HALFDUPLEX_RX_ONLY << 14);
	}
	else if(pHandle->spiConfig.SPI_busConfig == SPI_HALFDUPLEX_TX_ONLY)
	{
		pHandle->pSPIx->SPI_CR1 |= (SPI_HALFDUPLEX_MODE << 15);
		pHandle->pSPIx->SPI_CR1 |= (SPI_HALFDUPLEX_TX_ONLY << 14);
	}


}
