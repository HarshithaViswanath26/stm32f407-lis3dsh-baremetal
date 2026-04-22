#include "rcc.h"
#include <stdint.h>

void RCC_AHB1_init(AHB1_Peripherals_t peripheral, FunctionalState_t state)
{
	if(state)
		RCC->AHB1ENR |= (1U << peripheral);
}

void RCC_AHB1_deinit(AHB1_Peripherals_t peripheral, FunctionalState_t state)
{
	if(!state)
		RCC->AHB1ENR &= ~(1U << peripheral);
}
