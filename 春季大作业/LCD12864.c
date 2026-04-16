#include "config.h"
#include "lcd12864.h"
#include "font.h"

// ?????
static u8 LCD_Busy(void) {
    // 在Proteus仿真中，忙信号检测可能不可靠，使用延时替代
    return 0;
}

// ????
static void LCD_WaitFree(void) {
    // 添加简单的延时
    u8 i;
    for(i = 0; i < 10; i++) {
        _nop_();
    }
}

void LCD_WriteCmd(u8 cmd) {
    LCD_WaitFree();
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E  = 1;
    LCD_DATA = cmd;
    _nop_(); _nop_();
    LCD_E  = 0;
}

void LCD_WriteData(u8 dat) {
    LCD_WaitFree();
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E  = 1;
    LCD_DATA = dat;
    _nop_(); _nop_();
    LCD_E  = 0;
}

void LCD_SetPage(u8 page) {
    LCD_WriteCmd(PAGE_ADD | (page & 0x07));
}

void LCD_SetCol(u8 col) {
    LCD_WriteCmd(COL_ADD | (col & 0x3F));
}

void LCD_SelectScreen(u8 screen) {
    switch(screen) {
        case 1: LCD_CS1 = 0; LCD_CS2 = 1; break;
        case 2: LCD_CS1 = 1; LCD_CS2 = 0; break;
        case 3: LCD_CS1 = 0; LCD_CS2 = 0; break;
        default: LCD_CS1 = 1; LCD_CS2 = 1; break;
    }
}

void LCD_Init(void) {
    LCD_CS1 = 1; LCD_CS2 = 1;
    LCD_E = 0;
    LCD_RS = 0; LCD_RW = 0;  // 设置为命令模式
    LCD_WriteCmd(LCD_OFF);
    LCD_WriteCmd(START_LINE);
    LCD_WriteCmd(LCD_ON);
    LCD_Clear();
}

void LCD_Clear(void) {
    u8 i, j;
    for(i = 0; i < 8; i++) {
        LCD_SelectScreen(1);
        LCD_SetPage(i);
        LCD_SetCol(0);
        for(j = 0; j < 64; j++) LCD_WriteData(0x00);
        
        LCD_SelectScreen(2);
        LCD_SetPage(i);
        LCD_SetCol(0);
        for(j = 0; j < 64; j++) LCD_WriteData(0x00);
    }
    LCD_SelectScreen(3);
}

// ????(????????,???????????)

// ????16x16????
void LCD_DrawBitmap16x16(u8 x, u8 y, u8 *bitmap) {
    u8 page, col, i, pixelX, dat;
    u8 startPage, endPage;
    
    startPage = y / 8;
    endPage = (y + 15) / 8;  // 16??
    
    for(page = startPage; page <= endPage && page < 8; page++) {
        for(col = 0; col < 16; col++) {
            pixelX = x + col;
            if(pixelX >= 128) continue;
            
            if(pixelX < 64) LCD_SelectScreen(1);
            else LCD_SelectScreen(2);
            
            LCD_SetPage(page);
            LCD_SetCol(pixelX % 64);
            
            dat = 0;
            for(i = 0; i < 8; i++) {
                u8 pixelY, byteIdx, bitIdx;
                pixelY = page * 8 + i;
                if(pixelY >= y && pixelY < y + 16) {
                    byteIdx = (pixelY - y) * 2 + col / 8;
                    bitIdx = 7 - (col % 8);
                    if(bitmap[byteIdx] & (1 << bitIdx))
                        dat |= (1 << i);
                }
            }
            LCD_WriteData(dat);
        }
    }
    LCD_SelectScreen(3);
}

// ????(????????,???????????)
void LCD_DrawBitmap(u8 x, u8 y, u8 w, u8 h, u8 *bitmap) {
    u8 page, col, i, pixelX, dat;
    u8 startPage, endPage, bytePerRow;
    
    bytePerRow = (w + 7) / 8;
    startPage = y / 8;
    endPage = (y + h - 1) / 8;
    
    for(page = startPage; page <= endPage && page < 8; page++) {
        for(col = 0; col < w; col++) {
            pixelX = x + col;
            if(pixelX >= 128) continue;
            
            if(pixelX < 64) LCD_SelectScreen(1);
            else LCD_SelectScreen(2);
            
            LCD_SetPage(page);
            LCD_SetCol(pixelX % 64);
            
            dat = 0;
            for(i = 0; i < 8; i++) {
                u8 pixelY, byteIdx, bitIdx;
                pixelY = page * 8 + i;
                if(pixelY >= y && pixelY < y + h) {
                    byteIdx = (pixelY - y) * bytePerRow + col / 8;
                    bitIdx = 7 - (col % 8);
                    if(bitmap[byteIdx] & (1 << bitIdx))
                        dat |= (1 << i);
                }
            }
            LCD_WriteData(dat);
        }
    }
    LCD_SelectScreen(3);
}

// ????
void LCD_DrawFillRect(u8 x, u8 y, u8 w, u8 h, u8 pattern) {
    u8 page, col, pixelX, dat;
    u8 startPage, endPage;
    
    startPage = y / 8;
    endPage = (y + h - 1) / 8;
    
    for(page = startPage; page <= endPage && page < 8; page++) {
        for(col = 0; col < w; col++) {
            pixelX = x + col;
            if(pixelX >= 128) continue;
            
            if(pixelX < 64) LCD_SelectScreen(1);
            else LCD_SelectScreen(2);
            
            LCD_SetPage(page);
            LCD_SetCol(pixelX % 64);
            
            dat = pattern;
            if(page == startPage && (y % 8) != 0) {
                dat = pattern << (y % 8);
            } else if(page == endPage && ((y + h) % 8) != 0) {
                dat = pattern >> (8 - ((y + h) % 8));
            }
            LCD_WriteData(dat);
        }
    }
    LCD_SelectScreen(3);
}

// ????
void LCD_DisplayNum(u8 x, u8 y, u16 num, u8 len) {
    // ???(???????)
}

// ?????
void LCD_DisplayString(u8 x, u8 y, u8 *str) {
    while(*str) {
        LCD_DrawBitmap(x, y, 8, 8, ascii_8x8[*str - 0x20]);
        x += 8;
        str++;
    }
}