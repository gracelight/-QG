// eeprom.h
#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "config.h"

sbit SDA = P3^5;
sbit SCL = P3^6;

void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(u8 dat);
u8 I2C_ReceiveByte(void);
u8 I2C_WaitAck(void);

void EEPROM_WriteByte(u8 addr, u8 dat);
u8 EEPROM_ReadByte(u8 addr);
void EEPROM_SaveScore(u16 score);
u16 EEPROM_LoadScore(void);

#endif