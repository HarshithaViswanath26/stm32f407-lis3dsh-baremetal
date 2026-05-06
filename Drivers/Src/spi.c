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

	// setting BR = fclk/256 to prevent overrun on slave side
	pHandle->pSPIx->SPI_CR1 &= ~(7U << 3);
	pHandle->pSPIx->SPI_CR1 |= (7U << 3);
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t* pSPIx, uint32_t flagName)
{
	return (pSPIx->SPI_SR & flagName) ? 1 : 0;
}

// this func must be called after SPI_Init()
void SPI_CommunicationControl(SPI_RegDef_t* pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
		pSPIx->SPI_CR1 |= (1U << 6);
	else
		pSPIx->SPI_CR1 &= ~(1U << 6);
}

// to set the SSOE bit
void SPI_SSOEConfig(SPI_RegDef_t* pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		pSPIx->SPI_CR2 |= (1U << 2);
	}
	else
		pSPIx->SPI_CR2 &= ~(1U << 2);
}


void SPI_SendData(SPI_RegDef_t* pSPIx, uint8_t* pTXBuffer, uint32_t Len) // data is 8 bits and standard length
{
	/*if should not be used whole data should be sent
	 * this creates a blocking call along with
	 * another while loop below */

	// Polling type : Until all the data is transferred func does not return control
	// If there is any error with SPI config can hand the code here permanently

	while(Len > 0)
	{
		while((pSPIx->SPI_SR & SPI_TXE_FLAG) == FLAG_RESET);

		if(!(pSPIx->SPI_CR1 & (1 << 11)))  // careful with such cond.
		{
			// 8 bit Dataframe
			pSPIx->SPI_DR = *pTXBuffer;  // 1B data
			// increement the buffer addr
			pTXBuffer++;
			Len--;
		}
		else
		{
			pSPIx->SPI_DR = *((uint16_t*)pTXBuffer); // fetches 2B data
			pTXBuffer += 2;  // increment 2B
			Len -= 2;
		}


	}
}

void SPI_ReceiveData(SPI_RegDef_t* pSPIx, uint8_t* pRXBuffer, uint32_t Len)
{
	while(Len > 0)
	{
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);
		// when SET the loop breaks

		if(pSPIx->SPI_CR1 & (1 << 11))
		{
			// 16 bit data frame
			*((uint16_t*)pRXBuffer) = (uint16_t)pSPIx->SPI_DR; // DR reads from RX buffer
			pRXBuffer += 2;
			Len -= 2;
		}
		else
		{
			// 8 bit data frame
			*pRXBuffer = (uint8_t)pSPIx->SPI_DR;
			pRXBuffer++;
			Len--;
		}
	}
}



