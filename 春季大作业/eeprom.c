// eeprom.c (????)
#include "eeprom.h"
#include <intrins.h>

#define EEPROM_ADDR 0xA0

static void I2C_Delay(void) {
    _nop_(); _nop_(); _nop_(); _nop_();
}

void I2C_Start(void) {
    SDA = 1; SCL = 1; I2C_Delay();
    SDA = 0; I2C_Delay();
    SCL = 0;
}

void I2C_Stop(void) {
    SDA = 0; SCL = 1; I2C_Delay();
    SDA = 1; I2C_Delay();
}

void I2C_SendByte(u8 dat) {
    u8 i;
    for(i = 0; i < 8; i++) {
        SDA = (dat & 0x80) >> 7;
        dat <<= 1;
        SCL = 1; I2C_Delay();
        SCL = 0; I2C_Delay();
    }
    SDA = 1; SCL = 1; I2C_Delay();
    SCL = 0;
}

u8 I2C_ReceiveByte(void) {
    u8 i, dat = 0;
    SDA = 1;
    for(i = 0; i < 8; i++) {
        dat <<= 1;
        SCL = 1; I2C_Delay();
        dat |= SDA;
        SCL = 0; I2C_Delay();
    }
    return dat;
}

void EEPROM_WriteByte(u8 addr, u8 dat) {
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_WaitAck();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_SendByte(dat);
    I2C_WaitAck();
    I2C_Stop();
}

u8 EEPROM_ReadByte(u8 addr) {
    u8 dat;
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_WaitAck();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR | 0x01);
    I2C_WaitAck();
    dat = I2C_ReceiveByte();
    I2C_Stop();
    return dat;
}

void EEPROM_SaveScore(u16 score) {
    EEPROM_WriteByte(EE_ADDR_HIGHSCORE, score >> 8);
    EEPROM_WriteByte(EE_ADDR_HIGHSCORE + 1, score & 0xFF);
    EEPROM_WriteByte(EE_ADDR_MAGIC, 0x5A);  // ????
}