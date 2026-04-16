#ifndef __GAME_H__
#define __GAME_H__

#include "config.h"
#include "lcd12864.h"
#include "sound.h"

// ?????
typedef struct {
    u8 row;         // ???(0-4)
    u8 col;         // ???(???????)
    u8 hp;          // ??
    u8 type;        // ????
    u8 speed;       // ????
    u8 attackTimer; // ????
    u8 active;      // ????
} Enemy;

// ?????
typedef struct {
    u8 row;
    u8 x;           // ??X??
    u8 damage;
    u8 speed;
    u8 active;
} Bullet;

// ?????
typedef struct {
    u8 type;        // 0:? 1:?? 2:??
    u8 hp;
    u8 shootTimer;
} Plant;

// ????
typedef enum {
    GAME_MENU,
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

// ??????
extern GameState gameState;
extern u16 sunAmount;
extern u16 score;
extern u8 baseHP;
extern u8 cursorRow, cursorCol;
extern u8 selectedPlant;

// ????
void Game_Init(void);
void Game_Process(void);
void Game_Draw(void);
void Enemy_Spawn(void);
void Enemy_Update(void);
void Bullet_Update(void);
void Bullet_Create(u8 row, u8 x, u8 damage);
void Collision_Check(void);
void Plant_Place(u8 row, u8 col, u8 type);
void Plant_Shoot(void);  // ??????????
#endif