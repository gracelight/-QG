#include "sound.h"

// 12MHz??????????????(???1,??2)
static u8 code freqTable[][2] = {
    {0xFC, 0x66},  // 262Hz (C4) - ???
    {0xFE, 0x33},  // 523Hz (C5) - ???
    {0xFD, 0x80},  // 349Hz (F4) - ???
    {0xFD, 0x34},  // 392Hz (G4) - ????
    {0xFB, 0xE8}   // 220Hz (A3) - ????
};

static u8 soundPlaying = 0;
static u8 soundTimer = 0;
static u8 soundDuration = 0;
static SoundType currentSound = SOUND_NONE;

void Sound_Init(void) {
    TMOD |= 0x20;  // T1??2(8?????)
    BEEP = 0;
}

void Sound_Play(SoundType type) {
    if(type >= sizeof(freqTable)/2) return;
    
    currentSound = type;
    soundPlaying = 1;
    soundDuration = 15;  // 150ms
    soundTimer = 0;
    
    // ????
    TH1 = freqTable[type][0];
    TL1 = freqTable[type][1];
    TR1 = 1;
    ET1 = 1;
    EA = 1;
}

void Sound_Stop(void) {
    TR1 = 0;
    ET1 = 0;
    BEEP = 0;
    soundPlaying = 0;
    currentSound = SOUND_NONE;
}

// ?T1??????????
void Timer1_ISR(void) interrupt 3 {
    BEEP = ~BEEP;
}

// ????????,??????
void Sound_Tick(void) {
    if(soundPlaying) {
        soundTimer++;
        if(soundTimer >= soundDuration) {
            Sound_Stop();
        }
    }
}