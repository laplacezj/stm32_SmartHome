#ifndef __DEIVER_ESP8266_H
#define __DEIVER_ESP8266_H

#include "main.h"
#include "stdio.h"
#include <stdbool.h>
#include "string.h"
#include "stdint.h"





#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define  USART_DEBUG DEBUG_USARTx //调试用串口

void ESP8266_Init(void);
//void ESP8266_Usartx_Init(unsigned int baud);
void ESP8266_Clear(void);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);
#endif
