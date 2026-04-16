#include <reg51.h>
#include <intrins.h>
#include <string.h>

// ====================== 引脚定义 ======================
sbit LCD_RS  = P2^0;
sbit LCD_RW  = P2^1;
sbit LCD_E   = P2^2;
sbit LCD_CS1 = P2^3;
sbit LCD_CS2 = P2^4;
#define LCD_DATA P0

// 4x3矩阵键盘
sbit KEY_ROW0 = P1^0;
sbit KEY_ROW1 = P1^1;
sbit KEY_ROW2 = P1^2;
sbit KEY_ROW3 = P1^3;
sbit KEY_COL0 = P1^4;
sbit KEY_COL1 = P1^5;
sbit KEY_COL2 = P1^6;

sbit BUZZER = P3^7;

/******************** 类型定义 ********************/
#define u8  unsigned char
#define u16 unsigned int

typedef enum {
    STATE_MENU,        
    STATE_DIFF_SELECT,
    STATE_MAP_SELECT, 
    STATE_GAME_RUN,   
    STATE_GAME_PAUSE, 
    STATE_GAME_OVER,  
    STATE_HISTORY
} GameState;

typedef enum {
    TOWER_NONE = 0,
    TOWER_ARROW,   
    TOWER_TURRET,  
    TOWER_MAGIC,   
    TOWER_ENERGY   
} TowerType;

typedef enum {
    ENEMY_NONE = 0,
    ENEMY_SQUARE,  
    ENEMY_TRIANGLE 
} EnemyType;

typedef struct {
    u8 start_delay;   
    u8 init_energy;   
    u8 total_enemy;   
    u8 square_num;    
    u8 triangle_num;  
    u8 spawn_interval;
} DifficultyConfig;

typedef struct {
    u8 atk;           
    u8 hp;            
    u8 bullet_speed;  
    u8 range;         
    u8 range_type;    
    u8 cooldown;      
    u8 cost;          
    u8 energy_add_cd; 
} TowerAttr;

typedef struct {
    u8 atk;           
    u8 hp;            
    u8 move_speed;    
    u8 atk_range;     
    u8 atk_cooldown;  
} EnemyAttr;

typedef struct {
    TowerType type;
    u8 x;            
    u8 y;            
    u8 current_hp;    
    u8 current_cd;    
    u8 energy_cnt;    
} Tower;

typedef struct {
    EnemyType type;
    u8 x;            
    u8 y;            
    u8 current_hp;    
    u8 current_cd;    
    u8 move_cnt;      
    u8 is_alive;      
} Enemy;

typedef struct {
    u8 is_active;     
    u8 x;            
    u8 y;            
    u8 atk;          
    u8 speed;        
    u8 target_y;     
    u8 move_cnt;     
} Bullet;

typedef struct {
    u8 difficulty;    
    u8 base_hp_left;  
    u16 play_time;    
    u8 is_win;        
    u8 kill_count;    
} GameRecord;

/******************** 常量配置 ********************/
code TowerAttr tower_attr[5] = {
    {0,0,0,0,0,0,0,0},                     
    {10,20,4,5,0,10,1,0},                 
    {20,30,5,3,1,10,2,0},                 
    {15,30,7,4,2,15,3,0},                 
    {0,40,0,0,0,0,2,20}                    
};

code EnemyAttr enemy_attr[3] = {
    {0,0,0,0,0},                           
    {10,50,8,1,10},                        
    {20,30,10,1,10}                        
};

code DifficultyConfig diff_config[3] = {
    {4,5,10,5,5,20},                      
    {3,4,13,7,6,15},                      
    {2,3,16,9,7,12}                       
};

/******************** 宏定义 ********************/
#define MAX_TOWER     10     
#define MAX_ENEMY     10     
#define MAX_BULLET    8      
#define BASE_MAX_HP   80     
#define ENERGY_MAX    9      
#define GRID_ROW      5      
#define GRID_COL      6      

/******************** 全局变量 ********************/
GameState game_state = STATE_MENU;        
u8 current_diff = 0;                      
u8 current_map = 0;                       
u8 menu_cursor = 0;                       
u8 select_cursor_x = 1;                   
u8 select_cursor_y = 2;                   
u8 base_hp = BASE_MAX_HP;                 
u8 current_energy = 0;                    
u16 game_tick = 0;                        
u16 game_time = 0;                        
u8 enemy_spawn_cnt = 0;                   
u8 spawned_enemy = 0;                     
u8 killed_enemy = 0;                      
u8 game_result = 0;                       

xdata Tower tower_list[MAX_TOWER];        
xdata Enemy enemy_list[MAX_ENEMY];        
xdata Bullet bullet_list[MAX_BULLET];     
xdata GameRecord record_list[2];          
u8 record_cnt = 0;                        

u8 key_value = 0xFF;                      
u8 refresh_flag = 0;                      
u8 select_tower_type = TOWER_ARROW;       

/******************** 8x16标准字模 ********************/
code unsigned char char_table[][16] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 空格
    {0x00,0x00,0x7E,0x81,0x81,0x81,0x7E,0x00,0x00,0x7E,0x81,0x81,0x81,0x7E,0x00,0x00}, // O
    {0x00,0x00,0x81,0x42,0x24,0x18,0x42,0x81,0x00,0x81,0x42,0x24,0x18,0x42,0x81,0x00}, // X
    {0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // -
    {0x00,0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // =
    {0x00,0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00}, // #
    {0x00,0x00,0x22,0x14,0x7F,0x14,0x22,0x00,0x00,0x22,0x14,0x7F,0x14,0x22,0x00,0x00}, // *
    {0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00}, // .
    {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00,0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00}, // 0
    {0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00}, // 1
    {0x00,0x71,0x49,0x49,0x49,0x46,0x00,0x00,0x00,0x46,0x49,0x49,0x49,0x71,0x00,0x00}, // 2
    {0x00,0x49,0x49,0x49,0x49,0x7F,0x00,0x00,0x00,0x7F,0x49,0x49,0x49,0x49,0x00,0x00}, // 3
    {0x00,0x0F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x0F,0x00,0x00}, // 4
    {0x00,0x4F,0x49,0x49,0x49,0x79,0x00,0x00,0x00,0x79,0x49,0x49,0x49,0x4F,0x00,0x00}, // 5
    {0x00,0x7F,0x49,0x49,0x49,0x79,0x00,0x00,0x00,0x79,0x49,0x49,0x49,0x7F,0x00,0x00}, // 6
    {0x00,0x01,0x01,0x01,0x01,0x7F,0x00,0x00,0x00,0x7F,0x01,0x01,0x01,0x01,0x00,0x00}, // 7
    {0x00,0x7F,0x49,0x49,0x49,0x7F,0x00,0x00,0x00,0x7F,0x49,0x49,0x49,0x7F,0x00,0x00}, // 8
    {0x00,0x4F,0x49,0x49,0x49,0x7F,0x00,0x00,0x00,0x7F,0x49,0x49,0x49,0x4F,0x00,0x00}, // 9
    {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00}, // H
    {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00}, // L
    {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00}, // M
    {0x00,0x7F,0x01,0x01,0x01,0x7F,0x00,0x00,0x00,0x7F,0x01,0x01,0x01,0x7F,0x00,0x00}, // N
    {0x00,0x7F,0x41,0x41,0x41,0x7F,0x00,0x00,0x00,0x7F,0x41,0x41,0x41,0x7F,0x00,0x00}, // O
    {0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00,0x00,0x06,0x09,0x09,0x09,0x7F,0x00,0x00}, // P
    {0x00,0x7F,0x01,0x01,0x01,0x7F,0x00,0x00,0x00,0x7F,0x41,0x41,0x41,0x7F,0x00,0x00}, // Q
    {0x00,0x7F,0x09,0x09,0x09,0x76,0x00,0x00,0x00,0x76,0x09,0x09,0x09,0x7F,0x00,0x00}, // R
    {0x00,0x4F,0x49,0x49,0x49,0x79,0x00,0x00,0x00,0x79,0x49,0x49,0x49,0x4F,0x00,0x00}, // S
    {0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00}, // T
    {0x00,0x7F,0x40,0x40,0x40,0x7F,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x7F,0x00,0x00}, // U
    {0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x1F,0x00,0x00}, // V
    {0x00,0x7F,0x40,0x30,0x40,0x7F,0x00,0x00,0x00,0x7F,0x20,0x10,0x20,0x7F,0x00,0x00}, // W
    {0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00,0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00}, // X
    {0x00,0x07,0x08,0x78,0x08,0x07,0x00,0x00,0x00,0x07,0x08,0x78,0x08,0x07,0x00,0x00}, // Y
    {0x00,0x71,0x49,0x45,0x43,0x41,0x00,0x00,0x00,0x41,0x43,0x45,0x49,0x71,0x00,0x00}, // Z
    {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00,0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00}, // D
    {0x00,0x7F,0x41,0x41,0x41,0x41,0x00,0x00,0x00,0x7F,0x41,0x41,0x41,0x41,0x00,0x00}, // E
    {0x00,0x7F,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x7F,0x01,0x01,0x01,0x01,0x00,0x00}, // F
    {0x00,0x7F,0x41,0x49,0x49,0x49,0x00,0x00,0x00,0x49,0x49,0x49,0x41,0x7F,0x00,0x00}, // G
    {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00}, // I
    {0x00,0x40,0x40,0x40,0x40,0x7F,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00}, // J
    {0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00,0x00,0x41,0x22,0x14,0x08,0x7F,0x00,0x00}, // K
    {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00}, // C
    {0x00,0x7F,0x41,0x41,0x41,0x7F,0x00,0x00,0x00,0x7F,0x41,0x41,0x41,0x7F,0x00,0x00}, // A
    {0x00,0x7F,0x49,0x49,0x49,0x7F,0x00,0x00,0x00,0x7F,0x49,0x49,0x49,0x7F,0x00,0x00}, // B
    {0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00}, // :
    {0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00}, // >
};

/******************** 函数声明 ********************/
void delay_ms(u16 ms);
void delay_10us(u8 t);
void lcd_select_screen(u8 screen);
void lcd_write_cmd(u8 cmd);
void lcd_write_data(u8 dat);
void lcd_init(void);
void lcd_set_pos(u8 x, u8 y);
void lcd_clear(void);
void lcd_show_char(u8 x, u8 y, u8 c);
void lcd_show_str(u8 x, u8 y, u8 *str);
void key_scan(void);
void timer0_init(void);
u8 rand_num(u8 min, u8 max);
u8 find_empty_tower(void);
u8 find_empty_enemy(void);
u8 find_empty_bullet(void);
u8 check_tower_at(u8 x, u8 y);
u8 check_enemy_at(u8 x, u8 y);
void save_record(void);
void game_init(void);
void enemy_spawn(void);
void tower_attack(void);
void bullet_update(void);
void enemy_update(void);
void game_over_check(void);
void draw_menu(void);
void draw_diff_select(void);
void draw_map_select(void);
void draw_game_screen(void);
void draw_game_pause(void);
void draw_game_over(void);
void draw_history(void);
void menu_key_handle(void);
void diff_select_key_handle(void);
void map_select_key_handle(void);
void game_run_key_handle(void);
void game_pause_key_handle(void);
void game_over_key_handle(void);
void history_key_handle(void);

/******************** 基础延时函数 ********************/
void delay_ms(u16 ms) {
    u16 i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 114; j++);
}

void delay_10us(u8 t) {
    while(t--) _nop_();
}

/****************===== 核心修复1：正确的AMPIRE128X64驱动 =====****************/
void lcd_select_screen(u8 screen)
{
    if(screen == 0) {
        LCD_CS1 = 0; // 左半屏：低电平有效
        LCD_CS2 = 1; // 右半屏关闭
    } else if(screen == 1) {
        LCD_CS1 = 1; // 左半屏关闭
        LCD_CS2 = 0; // 右半屏：低电平有效
    } else if(screen == 2) {
        LCD_CS1 = 0; // 全屏：左右都打开
        LCD_CS2 = 0;
    }
}

void lcd_write_cmd(u8 cmd)
{
    LCD_E  = 0;
    delay_10us(1);
    LCD_RS = 0;
    LCD_RW = 0;
    delay_10us(1);
    LCD_DATA = cmd;
    delay_10us(2);
    LCD_E = 1;
    delay_10us(5);
    LCD_E = 0;
    delay_10us(2);
}

void lcd_write_data(u8 dat)
{
    LCD_E  = 0;
    delay_10us(1);
    LCD_RS = 1;
    LCD_RW = 0;
    delay_10us(1);
    LCD_DATA = dat;
    delay_10us(2);
    LCD_E = 1;
    delay_10us(5);
    LCD_E = 0;
    delay_10us(2);
}

void lcd_init(void) {
    LCD_CS1 = 1;
    LCD_CS2 = 0;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E = 0;
    delay_ms(50);

    lcd_select_screen(2);
    lcd_write_cmd(0xC0);  // 设置显示起始行
    lcd_write_cmd(0x3F);  // 开显示
    lcd_clear();
}

void lcd_set_pos(u8 x, u8 y) {
    u8 col = x * 8;
    if(col < 64) {
        lcd_select_screen(0);
    } else {
        lcd_select_screen(1);
        col -= 64;
    }
    lcd_write_cmd(0xB8 | y);
    lcd_write_cmd(0x40 | col);
}

void lcd_clear(void) {
    u8 i, j;
    lcd_select_screen(2);
    for(i=0; i<8; i++) {
        lcd_write_cmd(0xB8 | i);
        lcd_write_cmd(0x40);
        for(j=0; j<64; j++) {
            lcd_write_data(0x00);
        }
    }
}

void lcd_show_char(u8 x, u8 y, u8 c) {
    u8 i, idx = 0;
    if(c >= '0' && c <= '9') idx = c - '0' + 8;
    else if(c >= 'A' && c <= 'Z') idx = c - 'A' + 18;
    else {
        switch(c) {
            case 'O': idx=1; break; case 'X': idx=2; break; case '-': idx=3; break;
            case '=': idx=4; break; case '#': idx=5; break; case '*': idx=6; break;
            case '.': idx=7; break; case '>': idx=44; break; case ':': idx=43; break;
            default: idx=0; break;
        }
    }

    lcd_set_pos(x, y);
    for(i=0; i<8; i++) lcd_write_data(char_table[idx][i]);
    lcd_set_pos(x, y+1);
    for(i=8; i<16; i++) lcd_write_data(char_table[idx][i]);
}

void lcd_show_str(u8 x, u8 y, u8 *str) {
    while(*str && x<16) {
        lcd_show_char(x++, y, *str++);
    }
}

/****************===== 核心修复2：正确的键盘扫描，有return =====****************/
void key_scan(void) {
    u8 row, col;
    u8 key_map[4][3] = {{0,1,2}, {3,4,5}, {6,7,8}, {9,10,11}};
    key_value = 0xFF;

    for(row=0; row<4; row++) {
        KEY_ROW0 = 1; KEY_ROW1 = 1; KEY_ROW2 = 1; KEY_ROW3 = 1;
        switch(row) {
            case 0: KEY_ROW0 = 0; break;
            case 1: KEY_ROW1 = 0; break;
            case 2: KEY_ROW2 = 0; break;
            case 3: KEY_ROW3 = 0; break;
        }
        delay_ms(5);

        for(col=0; col<3; col++) {
            u8 col_val = 1;
            switch(col) {
                case 0: col_val = KEY_COL0; break;
                case 1: col_val = KEY_COL1; break;
                case 2: col_val = KEY_COL2; break;
            }
            if(col_val == 0) {
                delay_ms(20);
                if(col_val == 0) {
                    key_value = key_map[row][col];
                    while(col_val == 0) {
                        switch(col) {
                            case 0: col_val = KEY_COL0; break;
                            case 1: col_val = KEY_COL1; break;
                            case 2: col_val = KEY_COL2; break;
                        }
                    }
                    return; // 必须return，否则按键失灵
                }
            }
        }
    }
}

/******************** 定时器初始化 ********************/
void timer0_init(void) {
    TMOD |= 0x01;
    TH0 = (65536 - 20000) / 256;
    TL0 = (65536 - 20000) % 256;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

/******************** 游戏辅助函数 ********************/
u8 rand_num(u8 min, u8 max) {
    static u16 seed = 0;
    if(seed == 0) seed = game_tick;
    seed = seed * 1103515245 + 12345;
    return (seed % (max - min + 1)) + min;
}

u8 find_empty_tower(void) {
    u8 i;
    for(i = 0; i < MAX_TOWER; i++) {
        if(tower_list[i].type == TOWER_NONE) return i;
    }
    return 0xFF;
}

u8 find_empty_enemy(void) {
    u8 i;
    for(i = 0; i < MAX_ENEMY; i++) {
        if(enemy_list[i].is_alive == 0) return i;
    }
    return 0xFF;
}

u8 find_empty_bullet(void) {
    u8 i;
    for(i = 0; i < MAX_BULLET; i++) {
        if(bullet_list[i].is_active == 0) return i;
    }
    return 0xFF;
}

u8 check_tower_at(u8 x, u8 y) {
    u8 i;
    for(i = 0; i < MAX_TOWER; i++) {
        if(tower_list[i].type != TOWER_NONE && tower_list[i].x == x && tower_list[i].y == y) {
            return i;
        }
    }
    return 0xFF;
}

u8 check_enemy_at(u8 x, u8 y) {
    u8 i;
    for(i = 0; i < MAX_ENEMY; i++) {
        if(enemy_list[i].is_alive && enemy_list[i].x == x && enemy_list[i].y == y) {
            return i;
        }
    }
    return 0xFF;
}

void save_record(void) {
    record_list[1] = record_list[0];
    record_list[0].difficulty = current_diff;
    record_list[0].base_hp_left = base_hp;
    record_list[0].play_time = game_time;
    record_list[0].is_win = game_result;
    record_list[0].kill_count = killed_enemy;
    
    if(record_cnt < 2) record_cnt++;
}

void game_init(void) {
    u8 i;
    for(i = 0; i < MAX_TOWER; i++) {
        tower_list[i].type = TOWER_NONE;
        tower_list[i].current_hp = 0;
        tower_list[i].current_cd = 0;
        tower_list[i].energy_cnt = 0;
    }
    for(i = 0; i < MAX_ENEMY; i++) {
        enemy_list[i].is_alive = 0;
        enemy_list[i].current_hp = 0;
    }
    for(i = 0; i < MAX_BULLET; i++) {
        bullet_list[i].is_active = 0;
        bullet_list[i].move_cnt = 0;
    }

    base_hp = BASE_MAX_HP;
    current_energy = diff_config[current_diff].init_energy;
    game_tick = 0;
    game_time = 0;
    enemy_spawn_cnt = 0;
    spawned_enemy = 0;
    killed_enemy = 0;
    select_cursor_x = 1;
    select_cursor_y = 2;
    select_tower_type = TOWER_ARROW;
}

/******************** 游戏逻辑 ********************/
void enemy_spawn(void) {
    u8 enemy_id, enemy_type, spawn_y;
    if(game_tick < (u16)diff_config[current_diff].start_delay * 50) return;
    if(spawned_enemy >= diff_config[current_diff].total_enemy) return;

    enemy_spawn_cnt++;
    if(enemy_spawn_cnt < diff_config[current_diff].spawn_interval) return;
    enemy_spawn_cnt = 0;

    enemy_id = find_empty_enemy();
    if(enemy_id == 0xFF) return;

    if(spawned_enemy < diff_config[current_diff].square_num) 
        enemy_type = ENEMY_SQUARE;
    else 
        enemy_type = ENEMY_TRIANGLE;

    if(current_map == 0) 
        spawn_y = rand_num(0, 4);
    else 
        spawn_y = rand_num(1, 3);

    enemy_list[enemy_id].type = enemy_type;
    enemy_list[enemy_id].x = 5;
    enemy_list[enemy_id].y = spawn_y;
    enemy_list[enemy_id].current_hp = enemy_attr[enemy_type].hp;
    enemy_list[enemy_id].current_cd = 0;
    enemy_list[enemy_id].move_cnt = 0;
    enemy_list[enemy_id].is_alive = 1;

    spawned_enemy++;
}

void tower_attack(void) {
    u8 i, j, bullet_id, dis;
    for(i = 0; i < MAX_TOWER; i++) {
        if(tower_list[i].type == TOWER_NONE) continue;
        
        if(tower_list[i].current_cd > 0) tower_list[i].current_cd--;

        if(tower_list[i].type == TOWER_ENERGY) {
            tower_list[i].energy_cnt++;
            if(tower_list[i].energy_cnt >= 20) {
                tower_list[i].energy_cnt = 0;
                if(current_energy < ENERGY_MAX) current_energy++;
            }
            continue;
        }

        if(tower_list[i].current_cd > 0) continue;

        for(j = 0; j < MAX_ENEMY; j++) {
            if(enemy_list[j].is_alive == 0) continue;
            dis = (enemy_list[j].x > tower_list[i].x) ? 
                  (enemy_list[j].x - tower_list[i].x) : (tower_list[i].x - enemy_list[j].x);
            if(dis > 5 || enemy_list[j].y != tower_list[i].y) continue;

            bullet_id = find_empty_bullet();
            if(bullet_id == 0xFF) break;

            bullet_list[bullet_id].is_active = 1;
            bullet_list[bullet_id].x = tower_list[i].x + 1;
            bullet_list[bullet_id].y = tower_list[i].y;
            bullet_list[bullet_id].atk = tower_attr[tower_list[i].type].atk;
            bullet_list[bullet_id].speed = 4;
            bullet_list[bullet_id].target_y = enemy_list[j].y;
            bullet_list[bullet_id].move_cnt = 0;

            tower_list[i].current_cd = 10;
            break;
        }
    }
}

void bullet_update(void) {
    u8 i, enemy_id;
    for(i = 0; i < MAX_BULLET; i++) {
        if(bullet_list[i].is_active == 0) continue;
        bullet_list[i].move_cnt++;
        if(bullet_list[i].move_cnt >= 2) {
            bullet_list[i].move_cnt = 0;
            bullet_list[i].x++;
        }
        if(bullet_list[i].x >= GRID_COL) {
            bullet_list[i].is_active = 0;
            continue;
        }
        enemy_id = check_enemy_at(bullet_list[i].x, bullet_list[i].target_y);
        if(enemy_id != 0xFF) {
            enemy_list[enemy_id].current_hp -= bullet_list[i].atk;
            bullet_list[i].is_active = 0;
            if(enemy_list[enemy_id].current_hp <= 0) {
                enemy_list[enemy_id].is_alive = 0;
                killed_enemy++;
            }
        }
    }
}

void enemy_update(void) {
    u8 i, tower_id;
    for(i = 0; i < MAX_ENEMY; i++) {
        if(enemy_list[i].is_alive == 0) continue;
        if(enemy_list[i].current_cd > 0) enemy_list[i].current_cd--;

        tower_id = check_tower_at(enemy_list[i].x - 1, enemy_list[i].y);
        if(tower_id != 0xFF) {
            if(enemy_list[i].current_cd == 0) {
                tower_list[tower_id].current_hp -= enemy_attr[enemy_list[i].type].atk;
                enemy_list[i].current_cd = 10;
                if(tower_list[tower_id].current_hp <= 0) {
                    tower_list[tower_id].type = TOWER_NONE;
                }
            }
            continue;
        }

        enemy_list[i].move_cnt++;
        if(enemy_list[i].move_cnt >= 4) {
            enemy_list[i].move_cnt = 0;
            enemy_list[i].x--;
            if(enemy_list[i].x == 0) {
                base_hp -= 20;
                if(base_hp < 0) base_hp = 0;
                enemy_list[i].is_alive = 0;
                killed_enemy++;
            }
        }
    }
}

void game_over_check(void) {
    if(base_hp <= 0) {
        base_hp = 0;
        game_result = 0;
        game_state = STATE_GAME_OVER;
        save_record();
        draw_game_over();
    }
    if(spawned_enemy >= diff_config[current_diff].total_enemy && killed_enemy >= spawned_enemy) {
        game_result = 1;
        game_state = STATE_GAME_OVER;
        save_record();
        draw_game_over();
    }
}

/******************** 界面绘制 ********************/
void draw_menu(void) {
    lcd_clear();
    lcd_show_str(1, 0, "TOWER DEFENSE");
    lcd_show_str(0, 2, (menu_cursor == 0) ? ">START" : " START");
    lcd_show_str(0, 3, (menu_cursor == 1) ? ">DIFF INFO" : " DIFF INFO");
    lcd_show_str(0, 4, (menu_cursor == 2) ? ">HISTORY" : " HISTORY");
    lcd_show_str(0, 6, (menu_cursor == 3) ? ">EXIT" : " EXIT");
}

void draw_diff_select(void) {
    lcd_clear();
    lcd_show_str(3, 0, "DIFFICULTY");
    lcd_show_str(0, 2, (menu_cursor == 0) ? ">EASY" : " EASY");
    lcd_show_str(5, 2, (menu_cursor == 1) ? ">MID" : " MID");
    lcd_show_str(10, 2, (menu_cursor == 2) ? ">HARD" : " HARD");
}

void draw_map_select(void) {
    lcd_clear();
    lcd_show_str(3, 0, "SELECT MAP");
    lcd_show_str(0, 2, (menu_cursor == 0) ? ">5 LINE" : " 5 LINE");
    lcd_show_str(8, 2, (menu_cursor == 1) ? ">3 LINE" : " 3 LINE");
}

void draw_game_screen(void) {
    u8 i, j, lcd_x, lcd_y, tower_id, enemy_id, b, cursor_x, cursor_y;
    lcd_clear();
    
    // 最上方一行为防守方装备，4种，为固定背景
    lcd_show_str(0, 0, "* - # =");
    lcd_show_str(0, 1, "E A M T");
    
    // 最左边一列为大本营，显示大本营字体与剩余血量
    lcd_show_str(0, 2, "BASE");
    lcd_show_str(0, 4, "HP:");
    lcd_show_char(0, 5, (base_hp/10)+'0');
    lcd_show_char(1, 5, (base_hp%10)+'0');
    
    // 显示能量和时间
    lcd_show_str(10, 0, "EN:");
    lcd_show_char(13, 0, current_energy+'0');
    lcd_show_str(11, 1, "T:");
    lcd_show_char(13, 1, (game_time/10)+'0');
    lcd_show_char(14, 1, (game_time%10)+'0');
    
    // 绘制5条横线和6条竖线
    for(j=0; j<GRID_ROW; j++) {
        for(i=0; i<GRID_COL; i++) {
            lcd_x = i*2 + 2; // 从第2列开始，留出左边的大本营
            lcd_y = j+2;
            if(lcd_x >= 16) break;
            
            // 绘制网格背景
            lcd_show_char(lcd_x, lcd_y, ' ');
            
            // 绘制塔
            tower_id = check_tower_at(i, j);
            if(tower_id != 0xFF) {
                switch(tower_list[tower_id].type) {
                    case TOWER_ARROW:  lcd_show_char(lcd_x, lcd_y, '-'); break;
                    case TOWER_TURRET: lcd_show_char(lcd_x, lcd_y, '='); break;
                    case TOWER_MAGIC:  lcd_show_char(lcd_x, lcd_y, '#'); break;
                    case TOWER_ENERGY: lcd_show_char(lcd_x, lcd_y, '*'); break;
                    default: break;
                }
                continue;
            }
            
            // 绘制敌人
            enemy_id = check_enemy_at(i, j);
            if(enemy_id != 0xFF) {
                if(enemy_list[enemy_id].type == ENEMY_SQUARE) 
                    lcd_show_char(lcd_x, lcd_y, 'O');
                else 
                    lcd_show_char(lcd_x, lcd_y, 'X');
                continue;
            }
            
            // 绘制子弹
            for(b=0; b<MAX_BULLET; b++) {
                if(bullet_list[b].is_active && bullet_list[b].x == i && bullet_list[b].y == j) {
                    lcd_show_char(lcd_x+1, lcd_y, '.');
                    break;
                }
            }
        }
    }
    
    // 绘制选择光标
    cursor_x = select_cursor_x*2 + 2; // 从第2列开始，留出左边的大本营
    cursor_y = select_cursor_y+2;
    if(cursor_x < 16) {
        lcd_show_char(cursor_x, cursor_y, '#');
    }
}

void draw_game_pause(void) {
    lcd_clear();
    lcd_show_str(5, 0, "PAUSE");
    lcd_show_str(2, 2, "*=CONTINUE");
    lcd_show_str(2, 4, "5=BACK MENU");
}

void draw_game_over(void) {
    lcd_clear();
    if(game_result == 1) 
        lcd_show_str(6, 0, "WIN!");
    else 
        lcd_show_str(5, 0, "LOSE!");
    
    lcd_show_str(0, 2, "KILL:");
    lcd_show_char(5, 2, (killed_enemy/10)+'0');
    lcd_show_char(6, 2, (killed_enemy%10)+'0');
    
    lcd_show_str(0, 3, "HP:");
    lcd_show_char(3, 3, (base_hp/10)+'0');
    lcd_show_char(4, 3, (base_hp%10)+'0');
    
    lcd_show_str(0, 5, "5=BACK MENU");
}

void draw_history(void) {
    lcd_clear();
    lcd_show_str(4, 0, "HISTORY");
    if(record_cnt == 0) {
        lcd_show_str(3, 3, "NO RECORD");
    } else {
        u8 i;
        for(i=0; i<record_cnt; i++) {
            u8 line = 2 + i*2;
            lcd_show_str(0, line, record_list[i].is_win ? "WIN " : "LOSE");
            lcd_show_str(5, line, record_list[i].difficulty==0 ? "EASY" : (record_list[i].difficulty==1 ? "MID " : "HARD"));
        }
    }
    lcd_show_str(0, 7, "*=BACK");
}

/******************** 按键处理 ********************/
void menu_key_handle(void) {
    if(key_value == 0xFF) return;
    switch(key_value) {
        case 0: if(menu_cursor>0) menu_cursor--; draw_menu(); break;
        case 1: if(menu_cursor<3) menu_cursor++; draw_menu(); break;
        case 4: 
            switch(menu_cursor){
                case 0: menu_cursor=0; game_state=STATE_DIFF_SELECT; draw_diff_select(); break;
                case 1: lcd_clear(); lcd_show_str(1, 0, "DIFF INFO");
                        lcd_show_str(0, 2, "EASY: 5 EN, 10 ENEMY");
                        lcd_show_str(0, 3, "MID: 4 EN, 13 ENEMY");
                        lcd_show_str(0, 4, "HARD: 3 EN, 16 ENEMY");
                        lcd_show_str(0, 6, "5=BACK");
                        while(key_value != 4) {
                            key_scan();
                            delay_ms(10);
                        }
                        key_value = 0xFF;
                        draw_menu();
                        break;
                case 2: game_state=STATE_HISTORY; draw_history(); break;
                case 3: lcd_clear(); lcd_show_str(3, 0, "EXIT"); while(1); break;
            }
            break;
    }
    key_value=0xFF;
}

void diff_select_key_handle(void) {
    if(key_value==0xFF) return;
    switch(key_value){
        case 0: if(menu_cursor>0) menu_cursor--; draw_diff_select(); break;
        case 1: if(menu_cursor<2) menu_cursor++; draw_diff_select(); break;
        case 4: current_diff=menu_cursor; menu_cursor=0; game_state=STATE_MAP_SELECT; draw_map_select(); break;
        case 9: menu_cursor=0; game_state=STATE_MENU; draw_menu(); break;
    }
    key_value=0xFF;
}

void map_select_key_handle(void) {
    if(key_value==0xFF) return;
    switch(key_value){
        case 0: if(menu_cursor>0) menu_cursor--; draw_map_select(); break;
        case 1: if(menu_cursor<1) menu_cursor++; draw_map_select(); break;
        case 4: current_map=menu_cursor; game_init(); game_state=STATE_GAME_RUN; draw_game_screen(); break;
        case 9: menu_cursor=0; game_state=STATE_DIFF_SELECT; draw_diff_select(); break;
    }
    key_value=0xFF;
}

void game_run_key_handle(void) {
    u8 tower_slot;
    if(key_value==0xFF) return;
    switch(key_value){
        case 0: if(select_cursor_y>0) select_cursor_y--; refresh_flag=1; break;
        case 1: if(select_cursor_y<4) select_cursor_y++; refresh_flag=1; break;
        case 2: if(select_cursor_x>0) select_cursor_x--; refresh_flag=1; break;
        case 3: if(select_cursor_x<5) select_cursor_x++; refresh_flag=1; break;
        case 4: select_tower_type = (select_tower_type % 4) + 1; break;
        case 5: 
            if(current_map == 1 && (select_cursor_y == 0 || select_cursor_y == 4)) break;
            if(check_tower_at(select_cursor_x, select_cursor_y) != 0xFF) break;
            if(current_energy < tower_attr[select_tower_type].cost) break;
            tower_slot=find_empty_tower();
            if(tower_slot==0xFF) break;
            tower_list[tower_slot].type=select_tower_type;
            tower_list[tower_slot].x=select_cursor_x;
            tower_list[tower_slot].y=select_cursor_y;
            tower_list[tower_slot].current_hp = tower_attr[select_tower_type].hp;
            tower_list[tower_slot].current_cd = 0;
            current_energy -= tower_attr[select_tower_type].cost;
            refresh_flag=1;
            break;
        case 9: game_state=STATE_GAME_PAUSE; draw_game_pause(); break;
    }
    key_value=0xFF;
}

void game_pause_key_handle(void) {
    if(key_value == 0xFF) return;
    switch(key_value) {
        case 9: game_state=STATE_GAME_RUN; refresh_flag=1; break;
        case 4: game_state=STATE_MENU; menu_cursor=0; draw_menu(); break;
    }
    key_value=0xFF;
}

void game_over_key_handle(void) {
    if(key_value == 4) {
        game_state=STATE_MENU;
        menu_cursor=0;
        draw_menu();
    }
    key_value=0xFF;
}

void history_key_handle(void) {
    if(key_value == 9) {
        game_state=STATE_MENU;
        menu_cursor=0;
        draw_menu();
    }
    key_value=0xFF;
}

/******************** 中断服务函数 ********************/
void timer0_isr(void) interrupt 1 {
    TH0 = (65536 - 20000) / 256;
    TL0 = (65536 - 20000) % 256;
    game_tick++;
    if(game_tick%50==0) game_time++;
    if(game_state==STATE_GAME_RUN){
        enemy_spawn();
        tower_attack();
        bullet_update();
        enemy_update();
        game_over_check();
        refresh_flag=1;
    }
}

/****************===== 核心修复3：主函数，确保蜂鸣器关闭 =====****************/
void main(void) {
    // 初始化所有IO口
    P0=0xFF;
    P1=0xFF;
    P2=0xFF;
    P3=0xFF;
    BUZZER=1; // 确保蜂鸣器关闭，不会乱响

    // 初始化外设
    lcd_init();
    timer0_init();
    delay_ms(1000); // 上电等待稳定

    // 进入主菜单
    game_state=STATE_MENU;
    draw_menu();

    // 主循环
    while(1){
        key_scan();
        switch(game_state){
            case STATE_MENU:        menu_key_handle(); break;
            case STATE_DIFF_SELECT: diff_select_key_handle(); break;
            case STATE_MAP_SELECT:  map_select_key_handle(); break;
            case STATE_GAME_RUN:    
                game_run_key_handle(); 
                if(refresh_flag){
                    draw_game_screen();
                    refresh_flag=0;
                } 
                break;
            case STATE_GAME_PAUSE:  game_pause_key_handle(); break;
            case STATE_GAME_OVER:   game_over_key_handle(); break;
            case STATE_HISTORY:     history_key_handle(); break;
        }
        delay_ms(10);
    }
}