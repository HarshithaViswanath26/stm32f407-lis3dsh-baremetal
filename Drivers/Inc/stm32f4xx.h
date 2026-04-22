#ifndef INC_STM32F4XX_H
#define INC_STM32F4XX_H

#include <stdint.h>

#define __vo volatile

#define FLASH_BASEADDR		0x80000000U
#define SRAM1_BASEADDR		0x20000000U
#define SRAM2_BASEADDR		0x2001C000U
#define ROM_BASEADDR		0x1FFF0000U

#define SRAM_BASEADDR		SRAM1_BASEADDR

#define AHB1_BASEADDR		0x40020000U

typedef enum
{
	DISABLE = 0,
	ENABLE = 1
}FunctionalState_t;

#endif /*INC_STM32F4XX_H*/
