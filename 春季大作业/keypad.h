#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "config.h"

// 4?3?????
sbit ROW1 = P1^0;
sbit ROW2 = P1^1;
sbit ROW3 = P1^2;
sbit ROW4 = P1^3;
sbit COL1 = P1^4;
sbit COL2 = P1^5;
sbit COL3 = P1^6;

// ????
#define KEY_UP      0
#define KEY_DOWN    1
#define KEY_LEFT    2
#define KEY_RIGHT   3
#define KEY_OK      4
#define KEY_CANCEL  5
#define KEY_PLANT1  6   // ????
#define KEY_PLANT2  7   // ????
#define KEY_PAUSE   8
#define KEY_MENU    9
#define KEY_NONE    0xFF

u8 Key_Scan(void);
u8 Key_GetPressed(void);

#endif