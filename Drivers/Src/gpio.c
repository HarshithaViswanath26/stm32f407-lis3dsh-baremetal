#include "gpio.h"

void GPIO_Init(GPIO_Handle_t* pHandle)
{
    uint32_t temp = 0;

    // Configure MODE
    if(pHandle->pinConfig.mode < alternate)
    {
    	// only input & output mode without interrupt
		temp = pHandle->pinConfig.mode << (2 * pHandle->pinConfig.pinNum);
		pHandle->GPIOx->MODER &= ~((uint32_t)0x3 << (2 * pHandle->pinConfig.pinNum)); // clear
		pHandle->GPIOx->MODER |= temp; // set
		temp = 0;
    }

    // Configure OTYPE
    temp =  (pHandle->pinConfig.otype << pHandle->pinConfig.pinNum);
    pHandle->GPIOx->OTYPER &= ~(1 << pHandle->pinConfig.pinNum);
    pHandle->GPIOx->OTYPER |= temp;
    temp = 0;

    // Configure OSPEED
    temp = pHandle->pinConfig.ospeed << (2 * pHandle->pinConfig.pinNum);
    pHandle->GPIOx->OSPEEDR &= ~((uint32_t)0x3 << (2 * pHandle->pinConfig.pinNum));
    pHandle->GPIOx->OSPEEDR |= temp;
    temp = 0;

    // configure PUPD
    temp = pHandle->pinConfig.pupdtype << (2 * pHandle->pinConfig.pinNum);
    pHandle->GPIOx->PUPDR &= ~((uint32_t)0x3 << (2 * pHandle->pinConfig.pinNum));
    pHandle->GPIOx->PUPDR |= temp;
    temp = 0;
}

void GPIO_Led_ON(GPIO_Handle_t* pHandle)
{

	pHandle->GPIOx->ODR |= (1 << pHandle->pinConfig.pinNum);
}

void GPIO_Led_OFF(GPIO_Handle_t* pHandle)
{
	pHandle->GPIOx->ODR &= ~(1 << pHandle->pinConfig.pinNum);
}

void GPIO_Toggle(GPIO_Handle_t* pHandle)
{
	pHandle->GPIOx->ODR ^= (1 << pHandle->pinConfig.pinNum);
}
