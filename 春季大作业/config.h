#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <reg51.h>
#include <intrins.h>

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

// ===== ???? =====
#define MAP_ROWS        5       // ???(5?)
#define MAP_COLS        8       // ????
#define MAX_ENEMIES     8       // ?????
#define MAX_BULLETS     8       // ?????
#define MAX_PLANTS      (MAP_ROWS * MAP_COLS)  // ?????

// ===== ???? (??T0 10ms??) =====
#define GAME_TICK_MS    10
#define ENEMY_MOVE_TICK 10      // ??????(100ms)
#define BULLET_MOVE_TICK 5      // ??????(50ms)
#define SUN_GEN_TICK    50      // ????????(500ms)
#define PLANT_SHOOT_TICK 20     // ??????(200ms)

// ===== ???? =====
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define GRID_ROW_HEIGHT 10      // ??????(??)
#define GRID_COL_WIDTH  14      // ??????(??)
#define STATUS_BAR_Y    0       // ????(?0-1)
#define GAME_AREA_Y     2       // ???????

// ===== EEPROM?? =====
#define EE_ADDR_HIGHSCORE  0x00
#define EE_ADDR_MAGIC      0x0A   // ??,????????

#endif