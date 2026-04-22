#ifndef RCC_H
#define RCC_H

#include "stm32f4xx.h"
#include <stdint.h>

#define RCC_BASEADDR		(AHB1_BASEADDR + 0x3800)

typedef enum
{
	GpioA = 0,
	GpioB = 1,
	GpioC = 2,
	GpioD = 3,
	GpioE = 4,
	GpioF = 5,
	GpioG = 6,
	GpioH = 7,
	GpioI = 8,
	CRC = 12,
	BKPSRAM = 18,
	CCMDATARAM = 20,
	DMA1 = 21,
	DMA2 = 22,
	ETHMAC = 25,
	ETHMACTX = 26,
	ETHMACRX = 27,
	ETHMACPTP = 28,
	OTGHS = 29,
	OTGHSULPI = 30
}AHB1_Peripherals_t;


typedef struct
{
	volatile uint32_t CR;
	volatile uint32_t PLLCFGR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t AHB1RSTR;
	volatile uint32_t AHB2RSTR;
	volatile uint32_t AHB3RSTR;
	uint32_t Reserved0;
	volatile uint32_t APB1RSTR;
	volatile uint32_t APB2RSTR;
	uint32_t Reserved1[2];
	volatile uint32_t AHB1ENR;
	volatile uint32_t AHB2ENR;
	volatile uint32_t AHB3ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t APB2ENR;
	uint32_t Reserved2[2];
	volatile uint32_t AHB1LPENR;
	volatile uint32_t AHB2LPENR;
	volatile uint32_t AHB3LPENR;
	uint32_t Reserved3;
	volatile uint32_t APB1LPENR;
	volatile uint32_t APB2LPENR;
	uint32_t Reserved4[2];
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
	uint32_t Reserved5[2];
	volatile uint32_t SSCGR;
	volatile uint32_t PLLI2SCFGR;
}RCC_RegDef_t;

#define RCC ((RCC_RegDef_t*) RCC_BASEADDR)

void RCC_AHB1_init(AHB1_Peripherals_t peripheral, FunctionalState_t state);
void RCC_AHB1_deinit(AHB1_Peripherals_t peripheral, FunctionalState_t state);

#endif /*RCC_H*/
