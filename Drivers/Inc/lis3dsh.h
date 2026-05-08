/*
 * lis3dsh.h
 *
 *  Created on: May 5, 2026
 *      Author: 49157
 */

#ifndef INC_LIS3DSH_H_
#define INC_LIS3DSH_H_

#include "stm32f4xx.h"
#include <stdint.h>

#define LIS3DSH_WHO_AM_I					0x0F
#define LIS3DSH_CTRL_REG4					0x20
#define LIS3DSH_CTRL_REG3					0x23
#define LIS3DSH_CTRL_REG5					0x24
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

//for spi comm.
int8_t lis3dsh_Read_Reg(uint8_t regName);
void lis3dsh_Read_WHO_AM_I();
void lis3dsh_Write_Reg(uint8_t regName, uint8_t val);


//implement this only after WHO_AM_I is confirmed working
void lis3dsh_Init();
void lis3dsh_Read_XYZ();

#endif /* INC_LIS3DSH_H_ */
