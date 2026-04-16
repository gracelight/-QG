#ifndef __LCD12864_H__
#define __LCD12864_H__

#include "config.h"

// ??????
#define LCD_DATA    P0

sbit LCD_RS  = P2^0;  // H:??, L:??
sbit LCD_RW  = P2^1;  // H:?, L:?
sbit LCD_E   = P2^2;  // ??
sbit LCD_CS1 = P2^3;  // ?????(0??)
sbit LCD_CS2 = P2^4;  // ?????(0??)

// ????
#define LCD_ON      0x3F
#define LCD_OFF     0x3E
#define PAGE_ADD    0xB8
#define COL_ADD     0x40
#define START_LINE  0xC0

// ????
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetPage(u8 page);
void LCD_SetCol(u8 col);
void LCD_SelectScreen(u8 screen);  // 1:? 2:? 3:?
void LCD_WriteCmd(u8 cmd);
void LCD_WriteData(u8 dat);
void LCD_DrawBitmap(u8 x, u8 y, u8 w, u8 h, u8 *bitmap);
void LCD_DrawBitmap16x16(u8 x, u8 y, u8 *bitmap);
void LCD_DrawFillRect(u8 x, u8 y, u8 w, u8 h, u8 pattern);
void LCD_DisplayNum(u8 x, u8 y, u16 num, u8 len);
void LCD_DisplayString(u8 x, u8 y, u8 *str);
#endif