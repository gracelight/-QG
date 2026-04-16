#include "keypad.h"

static u8 keyState = KEY_NONE;
static u8 keyDebounce = 0;

// ?????(???)
u8 Key_Scan(void) {
    u8 row, col;
    u8 keyMap[4][3] = {
        {KEY_UP,    KEY_OK,     KEY_MENU},
        {KEY_DOWN,  KEY_CANCEL, KEY_PAUSE},
        {KEY_LEFT,  KEY_PLANT1, KEY_PLANT2},
        {KEY_RIGHT, 0xFF,       0xFF}
    };
    
    for(row = 0; row < 4; row++) {
        // ?????????
        P1 |= 0x0F;  // ??????
        P1 &= ~(0x01 << row);  // ?????
        
        _nop_(); _nop_(); _nop_(); _nop_();  // ??????
        
        for(col = 0; col < 3; col++) {
            if(!(P1 & (0x10 << col))) {  // ?????
                return keyMap[row][col];
            }
        }
    }
    return KEY_NONE;
}

// ????????
u8 Key_GetPressed(void) {
    u8 key = Key_Scan();
    if(key != KEY_NONE) {
        if(keyDebounce == 0) {
            keyDebounce = 5;  // ????
            keyState = key;
            return key;
        }
    } else {
        if(keyDebounce > 0) keyDebounce--;
    }
    return KEY_NONE;
}