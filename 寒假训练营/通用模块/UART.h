#ifndef __UART_H__
#define __UART_H__

void UART_Init();
void UART_SendByte(unsigned char Byte);
void UART_SendString(unsigned char *str);
	
#endif
