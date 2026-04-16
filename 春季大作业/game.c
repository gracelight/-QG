#include "game.h"
#include "keypad.h"
#include "eeprom.h"
#include "font.h"

// ????
// ?????(???ROM?)
typedef struct {
    u8 cost;
    u8 maxHP;
    u8 damage;
    u8 shootInterval;
} PlantAttr;

code PlantAttr plantDB[] = {
    {0,  0,  0, 0},     // ??0:??
    {50, 30, 0, 0},     // ??1:???
    {100,15, 2, 20}     // ??2:????
};
GameState gameState = GAME_MENU;
u16 sunAmount = 100;
u16 score = 0;
u8 baseHP = 3;
u8 cursorRow = 2, cursorCol = 4;
u8 selectedPlant = 1;  // 1:?? 2:??

// ??????
Plant xdata map[MAP_ROWS][MAP_COLS];
Enemy xdata enemies[MAX_ENEMIES];
Bullet xdata bullets[MAX_BULLETS];

// ?????(?ROM)
code u8 plantCost[] = {0, 50, 100};
code u8 plantMaxHP[] = {0, 30, 15};
code u8 plantDamage[] = {0, 0, 2};

// ???????
static u8 enemyMoveTick = 0;
static u8 bulletMoveTick = 0;
static u8 sunGenTick = 0;
static u8 shootTick = 0;

// ????
void Bullet_Create(u8 row, u8 x, u8 damage) {
    u8 i;
    for(i = 0; i < MAX_BULLETS; i++) {
        if(!bullets[i].active) {
            bullets[i].row = row;
            bullets[i].x = x;
            bullets[i].damage = damage;
            bullets[i].speed = 2;
            bullets[i].active = 1;
            break;
        }
    }
}

// ????
void Game_Init(void) {
    u8 i, j;
    
    // ????
    for(i = 0; i < MAP_ROWS; i++) {
        for(j = 0; j < MAP_COLS; j++) {
            map[i][j].type = 0;
            map[i][j].hp = 0;
            map[i][j].shootTimer = 0;
        }
    }
    
    // ???????
    for(i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
    for(i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    
    sunAmount = 100;
    score = 0;
    baseHP = 3;
    gameState = GAME_RUNNING;
    
    LCD_Clear();
    Game_Draw();
}

// ???????(??????10ms??)
void Game_Process(void) {
    u8 key;
    
    if(gameState != GAME_RUNNING) return;
    
    // ????
    key = Key_GetPressed();
    switch(key) {
        case KEY_UP:    if(cursorRow > 0) cursorRow--; break;
        case KEY_DOWN:  if(cursorRow < MAP_ROWS-1) cursorRow++; break;
        case KEY_LEFT:  if(cursorCol > 0) cursorCol--; break;
        case KEY_RIGHT: if(cursorCol < MAP_COLS-1) cursorCol++; break;
        case KEY_OK:
            if(map[cursorRow][cursorCol].type == 0 && 
               sunAmount >= plantCost[selectedPlant]) {
                Plant_Place(cursorRow, cursorCol, selectedPlant);
            }
            break;
        case KEY_PLANT1: selectedPlant = 1; break;
        case KEY_PLANT2: selectedPlant = 2; break;
        case KEY_PAUSE:  gameState = GAME_PAUSED; break;
    }
    
    // ??????
    if(++enemyMoveTick >= ENEMY_MOVE_TICK) {
        enemyMoveTick = 0;
        Enemy_Update();
    }
    if(++bulletMoveTick >= BULLET_MOVE_TICK) {
        bulletMoveTick = 0;
        Bullet_Update();
    }
    if(++sunGenTick >= SUN_GEN_TICK) {
        sunGenTick = 0;
        sunAmount += 5;  // ??????
    }
    if(++shootTick >= PLANT_SHOOT_TICK) {
        shootTick = 0;
        // ??????
        Plant_Shoot();
    }
    
    // ????
    Collision_Check();
    
    // ????
    Enemy_Spawn();
    
    // ??????
    if(baseHP == 0) {
        gameState = GAME_OVER;
        Sound_Play(SOUND_GAME_OVER);
        EEPROM_SaveScore(score);
    }
    
    // ????
    Game_Draw();
}

// ????
void Plant_Place(u8 row, u8 col, u8 type) {
    if(sunAmount >= plantCost[type]) {
        sunAmount -= plantCost[type];
        map[row][col].type = type;
        map[row][col].hp = plantMaxHP[type];
        map[row][col].shootTimer = 0;
        Sound_Play(SOUND_PLANT);
    }
}

// ????
void Enemy_Spawn(void) {
    static u8 spawnTimer = 0;
    u8 i;
    
    if(++spawnTimer < 30) return;  // ?300ms????
    spawnTimer = 0;
    
    for(i = 0; i < MAX_ENEMIES; i++) {
        if(!enemies[i].active) {
            enemies[i].row = i % MAP_ROWS;  // ?????
            enemies[i].col = MAP_COLS * GRID_COL_WIDTH;  // ???
            enemies[i].hp = 10;
            enemies[i].type = 0;
            enemies[i].speed = 1;
            enemies[i].attackTimer = 0;
            enemies[i].active = 1;
            break;
        }
    }
}

// ????
void Enemy_Update(void) {
    u8 i;
    u8 gridCol;  // ????
    
    for(i = 0; i < MAX_ENEMIES; i++) {
        if(!enemies[i].active) continue;
        
        gridCol = enemies[i].col / GRID_COL_WIDTH;
        
        if(gridCol == 0) {
            baseHP--;
            enemies[i].active = 0;
            continue;
        }
        
        if(gridCol < MAP_COLS && map[enemies[i].row][gridCol-1].type != 0) {
            if(++enemies[i].attackTimer >= 10) {
                enemies[i].attackTimer = 0;
                map[enemies[i].row][gridCol-1].hp--;
                if(map[enemies[i].row][gridCol-1].hp == 0) {
                    map[enemies[i].row][gridCol-1].type = 0;
                }
            }
        } else {
            enemies[i].col -= enemies[i].speed;
        }
    }
}

void Bullet_Update(void) {
    u8 i, j;
    u8 enemyX;  // ????
    
    for(i = 0; i < MAX_BULLETS; i++) {
        if(!bullets[i].active) continue;
        
        bullets[i].x += bullets[i].speed;
        
        if(bullets[i].x >= 128) {
            bullets[i].active = 0;
            continue;
        }
        
        for(j = 0; j < MAX_ENEMIES; j++) {
            if(!enemies[j].active) continue;
            if(enemies[j].row != bullets[i].row) continue;
            
            enemyX = enemies[j].col;
            if(bullets[i].x >= enemyX - 4 && bullets[i].x <= enemyX + 8) {
                enemies[j].hp -= bullets[i].damage;
                bullets[i].active = 0;
                Sound_Play(SOUND_HIT);
                
                if(enemies[j].hp == 0) {
                    enemies[j].active = 0;
                    score += 10;
                    Sound_Play(SOUND_ENEMY_DIE);
                }
                break;
            }
        }
    }
}
void Plant_Shoot(void) {
    u8 i, j;
    for(i = 0; i < MAP_ROWS; i++) {
        for(j = 0; j < MAP_COLS; j++) {
            if(map[i][j].type == 2) {  // ????
                if(++map[i][j].shootTimer >= plantDB[2].shootInterval) {
                    map[i][j].shootTimer = 0;
                    // ??????
                    Bullet_Create(i, j * GRID_COL_WIDTH + 8, plantDB[2].damage);
                }
            }
        }
    }
}
// ????(?????)

// ????
void Collision_Check(void) {
    u8 i;
    for(i = 0; i < MAX_ENEMIES; i++) {
        if(!enemies[i].active) continue;
        // ??Enemy_Update???
    }
}

// ????
void Game_Draw(void) {
    // ???(???????) 
    LCD_Clear();
    // ????
    LCD_DisplayString(0, 0, "Sun: ");
    // ????
    LCD_DisplayString(60, 0, "Score: ");
    // ????
    LCD_DisplayString(0, 6, "HP: ");
}