/*********************************************************
* 工程：51单片机塔防/小游戏主程序
* 硬件：AMPIRE128x64 LCD、矩阵键盘、蜂鸣器、EEPROM
* 功能：菜单显示、游戏状态管理、定时器中断、按键处理
**********************************************************/

// 包含各模块驱动头文件
#include "config.h"       // 系统配置文件（引脚、宏定义）
#include "lcd12864.h"     // AMPIRE12864液晶驱动
#include "keypad.h"       // 矩阵键盘驱动
#include "sound.h"        // 蜂鸣器/音效驱动
#include "game.h"         // 游戏逻辑核心
#include "eeprom.h"       // 掉电存储（高分记录）
#include "font.h"         // 字模库（中文/英文点阵）

// 全局系统节拍计数器（volatile防止编译器优化，中断中使用）
volatile u8 sysTick = 0;

/*********************************************************
* 函数名：Timer0_ISR
* 功能  ：定时器0中断服务函数，10ms定时中断
* 输入  ：无
* 输出  ：无
* 备注  ：12MHz晶振，定时10ms，用于系统计时+音效驱动
**********************************************************/
void Timer0_ISR(void) interrupt 1 {
    // 重装定时器初值，保证10ms精准定时
    TH0 = (65536 - 10000) / 256;
    TL0 = (65536 - 10000) % 256;
    
    sysTick++;  // 系统节拍+1（每10ms加1）
    Sound_Tick();  // 蜂鸣器节拍驱动（处理音效播放）
}

/*********************************************************
* 函数名：Menu_Draw
* 功能  ：绘制主菜单界面（LCD初始化显示）
* 输入  ：无
* 输出  ：无
**********************************************************/
void Menu_Draw(void) {
    LCD_Clear();  // 清屏
    
    // ===== 第一行：显示游戏标题 "TFGAME" =====
    LCD_DrawBitmap16x16(10, 0, (u8*)custom_chars[0]);  // 显示字符 T
    LCD_DrawBitmap16x16(30, 0, (u8*)custom_chars[1]);  // 显示字符 F
    LCD_DrawBitmap16x16(50, 0, (u8*)custom_chars[2]);  // 显示字符 G
    LCD_DrawBitmap16x16(70, 0, (u8*)custom_chars[3]);  // 显示字符 A
    LCD_DrawBitmap16x16(90, 0, (u8*)custom_chars[4]);  // 显示字符 M
    LCD_DrawBitmap16x16(110, 0, (u8*)custom_chars[5]); // 显示字符 E
    
    // ===== 第二~四行：显示菜单选项 =====
    LCD_DisplayString(10, 2, "1.Start Game");    // 选项1：开始游戏
    LCD_DisplayString(10, 3, "2.High Score");    // 选项2：最高分记录
    LCD_DisplayString(10, 4, "3.Difficulty");    // 选项3：难度设置
    
    // ===== 显示中文：历史记录 =====
    LCD_DrawBitmap16x16(10, 40, (u8*)history_chars[0]);  // 历
    LCD_DrawBitmap16x16(30, 40, (u8*)history_chars[1]);  // 史
    LCD_DrawBitmap16x16(50, 40, (u8*)history_chars[2]);  // 记
    LCD_DrawBitmap16x16(70, 40, (u8*)history_chars[3]);  // 录
    
    // ===== 显示中文：游戏说明 =====
    LCD_DrawBitmap16x16(10, 56, (u8*)game_desc_chars[0]); // 游
    LCD_DrawBitmap16x16(30, 56, (u8*)game_desc_chars[1]); // 戏
    LCD_DrawBitmap16x16(50, 56, (u8*)game_desc_chars[2]); // 说
    LCD_DrawBitmap16x16(70, 56, (u8*)game_desc_chars[3]); // 明
}

/*********************************************************
* 函数名：Menu_Process
* 功能  ：菜单界面按键处理（上下选择+确认）
* 输入  ：无
* 输出  ：无
**********************************************************/
void Menu_Process(void) {
    u8 key = Key_GetPressed();               // 读取按键值
    static u8 menuItem = 0;                  // 静态变量：记录当前选中的菜单项
    
    if(key == KEY_UP && menuItem > 0)        // 按下上键：选项上移（不越界）
        menuItem--;
    if(key == KEY_DOWN && menuItem < 2)      // 按下下键：选项下移（不越界）
        menuItem++;
    if(key == KEY_OK) {                      // 按下确认键：执行对应功能
        switch(menuItem) {
            case 0: Game_Init(); break;       // 选中第1项：初始化并开始游戏
            case 1: break;                   // 选中第2项：查看最高分（待完善）
            case 2: break;                   // 选中第3项：设置难度（待完善）
        }
    }
}

/*********************************************************
* 函数名：Pause_Process
* 功能  ：游戏暂停状态处理
* 输入  ：无
* 输出  ：无
**********************************************************/
void Pause_Process(void) {
    // 按下暂停键：恢复游戏运行状态
    if(Key_GetPressed() == KEY_PAUSE) {
        gameState = GAME_RUNNING;
    }
}

/*********************************************************
* 函数名：GameOver_Process
* 功能  ：游戏结束状态处理
* 输入  ：无
* 输出  ：无
**********************************************************/
void GameOver_Process(void) {
    u8 key = Key_GetPressed();
    if(key == KEY_OK) {              // 按下确认键：返回主菜单
        gameState = GAME_MENU;
        Menu_Draw();                 // 重新绘制菜单界面
    } else if(key == KEY_CANCEL) {
        // 按下取消键：功能待扩展
    }
}

/*********************************************************
* 函数名：main
* 功能  ：主函数，系统入口
* 输入  ：无
* 输出  ：无
**********************************************************/
void main(void) {
    // 定时器初始化
    TMOD = 0x21;  // T0工作模式1（16位定时），T1工作模式2
    // 定时器0初值（10ms）
    TH0 = (65536 - 10000) / 256;
    TL0 = (65536 - 10000) % 256;
    ET0 = 1;      // 使能定时器0中断
    TR0 = 1;      // 启动定时器0
    EA  = 1;      // 开启总中断
    
    // 外设初始化
    LCD_Init();    // AMPIRE12864液晶初始化
    Sound_Init();  // 蜂鸣器初始化
    
    Menu_Draw();          // 绘制主菜单
    gameState = GAME_MENU;// 设置初始状态：菜单模式
    
    // 主循环：根据游戏状态执行对应逻辑
    while(1) {
        switch(gameState) {
            case GAME_MENU:     // 菜单状态
                Menu_Process();
                break;
            case GAME_RUNNING:  // 游戏运行状态
                Game_Process();
                break;
            case GAME_PAUSED:   // 游戏暂停状态
                Pause_Process();
                break;
            case GAME_OVER:     // 游戏结束状态
                GameOver_Process();
                break;
        }
    }
}