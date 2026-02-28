#include <REGX51.H>
#include "Delay.h"
#include "LCD1602.h"
#include "Timer.h"
#include "MatrixKey.h"

unsigned char sec,ms,min;
unsigned char Num;

void main()
{
		
		LCD_Init();
		Timer0Init();
	P2=~0x00;
	LCD_ShowString(1,1,"Clock");
	LCD_ShowString(2,3,":  :");
	while(1)
	{

	Num=MatrixKey(); //É¨Ãè¾ØÕó¼üÅÌ
	if(Num==1)
	{
		 float total_ms = sec*100+ms;
	  if(total_ms>=950&&total_ms<=1050)
			P2=~0x01;
		else if(total_ms<950&&total_ms>=850)
			P2=~0x03;	  
		else if(total_ms>1050&&total_ms<=1150)
			P2=~0x03;
		else if(total_ms>1150||total_ms<850)
			P2=~0x00;
	  else
			P2=~0x00;
	}
		
		if(ms==100)
	{ms=0;
		sec++;
	}
	if(sec==60)
	{sec=0;
		min++;
	}
	if(min==60)
		min=0;
LCD_ShowNum(2,7,ms,2);
	LCD_ShowNum(2,4,sec,2);
	LCD_ShowNum(2,1,min,2);
	}
	
}

void Timer0_Routine() interrupt 1
{
	static unsigned int T0Count;
	TL0=0x66;		//??????
	TH0=0xFC;   //??????
	T0Count++;
	if(T0Count>=10)
	{
		T0Count=0;
		ms++;
	}
}