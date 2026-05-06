/*
 * spi.h
 *
 *  Created on: Apr 30, 2026
 *      Author: 49157
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "stm32f4xx.h"
#include "rcc.h"
#include "gpio.h"
#include <stdint.h>

#define SPI1_BASEADDR			(APB2_BASEADDR + 0x3000U)
#define SPI2_BASEADDR			(APB1_BASEADDR + 0x3800U)

typedef struct
{
	volatile uint32_t SPI_CR1;
	volatile uint32_t SPI_CR2;
	volatile uint32_t SPI_SR;
	volatile uint32_t SPI_DR;
	volatile uint32_t SPI_CRCPR;
	volatile uint32_t SPI_RXCRCR;
	volatile uint32_t SPI_TXCRCR;
	volatile uint32_t SPI_I2SCFGR;
	volatile uint32_t SPI_I2SPR;
}SPI_RegDef_t;

typedef struct
{
	uint8_t SPI_dff;  // SPI data frame format
	uint8_t SPI_sclSpeed; // SPI SCL BaudRate
	uint8_t SPI_busConfig; // SPI config of full duplex, half duplex or RX only
	uint8_t SPI_ssm;  // SPI SW slave management
	uint8_t SPI_mstr; // SPI Master selection
	uint8_t SPI_cpol; // SPI clock polarity
	uint8_t SPI_cpha; // SPI clock phase
}SPI_Config_t;

typedef struct
{
	SPI_RegDef_t* pSPIx;
	SPI_Config_t spiConfig;
}SPI_Handle_t;


typedef enum
{
	SPI_dframe8 = 0,
	SPI_dframe16 = 1
}SPI_Dataframes_t;

typedef enum
{
	SPI_ssmDi = 0,
	SPI_ssmEn = 1
}SPI_Slave_Manage_t;


// SSM needs to be set to enable this
typedef enum
{
	SPI_ssiDi = 0,
	SPI_ssiEn = 1
}SPI_Int_Slave_Select_t;

typedef enum
{
	SPI_slave = 0,
	SPI_master = 1
}SPI_Mstr_Select;

typedef enum
{
	SPI_cpol0 = 0,
	SPI_cpol1 = 1
}SPI_CPOL_t;

typedef enum
{
	SPI_cpha0 = 0,
	SPI_cpha1 = 1
}SPI_CPHA_t;

typedef enum
{
	SPI_PRESCALAR2 = 0,
	SPI_PRESCALAR4 = 1,
	SPI_PRESCALAR8 = 2,
	SPI_PRESCALAR16 = 3,
	SPI_PRESCALAR32 = 4,
	SPI_PRESCALAR64 = 5,
	SPI_PRESCALAR128 = 6,
	SPI_PRESCALAR256 = 7,
}SPI_SCL_Speed_t;

#define SPI1		((SPI_RegDef_t*)SPI1_BASEADDR)
#define SPI2		((SPI_RegDef_t*)SPI2_BASEADDR)

#define SPI_DISABLE					0
#define SPI_ENABLE					1

// for RXONLY bit
#define SPI_SIMPLEX_RX_ONLY				1

// for BIDIMODE bit
#define SPI_FULLDUPLEX_MODE				0
#define SPI_HALFDUPLEX_MODE				1

// for BIDIOE bit
#define SPI_HALFDUPLEX_RX_ONLY			0
#define SPI_HALFDUPLEX_TX_ONLY			1


#define SPI_RXNE_FLAG				(1U << 0)
#define SPI_TXE_FLAG				(1U << 1)

void SPI_Init(SPI_Handle_t* pHandle);

void SPI_SendData(SPI_RegDef_t* pSPIx, uint8_t* pTXBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t* pSPIx, uint8_t* pRXBuffer, uint32_t Len);

void SPI_SSOEConfig(SPI_RegDef_t* pSPIx, uint8_t EnorDi);
void SPI_CommunicationControl(SPI_RegDef_t* pSPIx, uint8_t EnorDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t* pSPIx, uint32_t flagName);

#endif /* INC_SPI_H_ */
