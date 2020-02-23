

#ifndef _EEPROM_H
#define _EEPROM_H

#include "stm32f1xx_hal.h"



/* 
 * EEPROM 8kb = 8192bit = 8192/8 B = 1024 B
 * 128 pages of 8 bytes each
 *
 * Device Address
 * 1 0 1 0 A2 P1 P0 R/W
 * 1 0 1 0 0  0  0  0 = 0XA0   Ð´     256byte
 * 1 0 1 0 0  0  0  1 = 0XA1   ¶Á
 
 * 1 0 1 0 0  0  1  0 = 0XA2   Ð´     256byte
 * 1 0 1 0 0  0  1  1 = 0XA3   ¶Á
 
 * 1 0 1 0 0  1  0  0 = 0XA4   Ð´     256byte
 * 1 0 1 0 0  1  0  1 = 0XA5   ¶Á

 * 1 0 1 0 0  1  0  0 = 0XA6   Ð´     256byte
 * 1 0 1 0 0  1  0  1 = 0XA7   ¶Á
 */
/* EEPROM Addresses defines */ 
#define DATA_ADDRESS             0xA0
#define DATA_ADDRESS2			 0xA2

#define CHK_ID								 0xA5A6AFA3


void               I2C_EEPROM_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value);
HAL_StatusTypeDef  I2C_EEPROM_WriteBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length);
uint8_t            I2C_EEPROM_ReadData(uint16_t Addr, uint8_t Reg);
HAL_StatusTypeDef  I2C_EEPROM_ReadBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length);
HAL_StatusTypeDef  I2C_EEPROM_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);

HAL_StatusTypeDef EE_DatasRead(uint16_t Addr,uint8_t *pBuffer, uint16_t Length);
HAL_StatusTypeDef EE_DatasWrite(uint16_t Addr,uint8_t *pBuffer, uint16_t Length);
#endif

