#ifndef __SOUND_H__
#define __SOUND_H__

#include "config.h"

sbit BEEP = P3^4;

// ????
typedef enum {
    SOUND_PLANT,    // ??
    SOUND_SHOOT,    // ??
    SOUND_HIT,      // ??
    SOUND_ENEMY_DIE,// ????
    SOUND_GAME_OVER,// ????
    SOUND_NONE
} SoundType;

void Sound_Init(void);
void Sound_Play(SoundType type);
void Sound_Stop(void);
void Sound_Tick(void);  // ?????????

#endif