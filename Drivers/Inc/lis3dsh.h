/*
 * lis3dsh.h
 *
 *  Created on: May 5, 2026
 *      Author: 49157
 */

#ifndef INC_LIS3DSH_H_
#define INC_LIS3DSH_H_

#include "lis3dsh.h"
#include "stm32f4xx.h"
#include "rcc.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"
#include "exti.h"
#include <stdint.h>

#define LIS3DSH_WHO_AM_I					0x0F
#define LIS3DSH_CTRL_REG4					0x20
#define LIS3DSH_CTRL_REG3					0x23
#define LIS3DSH_CTRL_REG5					0x24
#define LIS3DSH_CTRL_REG6					0x25
#define LIS3DSH_STATUS						0x27
#define LIS3DSH_OUT_X_L						0x28
#define LIS3DSH_OUT_X_H						0x29
#define LIS3DSH_OUT_Y_L						0x2A
#define LIS3DSH_OUT_Y_H						0x2B
#define LIS3DSH_OUT_Z_L						0x2C
#define LIS3DSH_OUT_Z_H						0x2D


//Read & Write cmds
#define LIS3DSH_READ						0x80		//bit7 = 1
#define LIS3DSH_WRITE						0x00		//bit7 = 0
#define LIS3DSH_MULTI_BYTE					0x40		//bit6 = 1


// other macros
#define LIS3DSH_DUMMY_DATA					0xFF
#define LIS3DSH_SENSITIVITY					60  // (0.06 * 1000) microg

// tilt threshold, based on a set of initial readings
#define X_OFFSET  1500
#define Y_OFFSET  238000



static uint8_t spi1PinSet[];
static GPIO_Handle_t spi1Pins, spi1Css;
extern USART_Handle_t usart2;
static GPIO_Handle_t ledGreen, ledOrange, ledRed, ledBlue;
extern uint8_t buffer[6]; // to hold the low & high values of x,y,z

//for spi comm.
uint8_t lis3dsh_Read_Reg(uint8_t regName);
void lis3dsh_Read_WHO_AM_I();
void lis3dsh_Write_Reg(uint8_t regName, uint8_t val);


//implement this only after WHO_AM_I is confirmed working
void lis3dsh_Init();
void lis3dsh_Read_XYZ();


void lis3dsh_INT1_Init();

#endif /* INC_LIS3DSH_H_ */
