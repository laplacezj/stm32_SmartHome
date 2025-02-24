#ifndef __DEIVER_ESP8266_H
#define __DEIVER_ESP8266_H

#include "main.h"

//#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
//#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
//#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
//#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
//	
////MQTT连接服务器
//extern uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);
////MQTT消息订阅
//extern uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);
////MQTT消息发布
//extern uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos);
////MQTT发送心跳包
//extern void MQTT_SentHeart(void);

//extern void MQTT_Subscribe(char *topic_name, int QoS);

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define  USART_DEBUG DEBUG_USARTx //调试用串口

void ESP8266_Init(void);
//void ESP8266_Usartx_Init(unsigned int baud);
void ESP8266_Clear(void);
void ESP8266_SendData(unsigned char *data, unsigned short len);
//unsigned char *ESP8266_GetIPD(unsigned short timeOut);
//void Send_Data_To_Cloud(void);
//void Get_Data_From_Cloud(void);
#endif
