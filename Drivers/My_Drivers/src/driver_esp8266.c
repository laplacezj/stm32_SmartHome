#include "driver_esp8266.h"
#include "my_print.h"
#include "string.h"

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"111\",\"12345678\"\r\n"   //连接的Wifi名 密码
#define MQTTUSERCFG  "AT+MQTTUSERCFG=0,1,\"NULL\",\"STM32&k1h2hJkoTA7\",\"25940a10a11126eb67377bed21b8983b595b5c96429664f9519baeaa5f91dd58\",0,0,\"\"\r\n"
#define MQTTCLIENTID "AT+MQTTCLIENTID=0,\"k1h2hJkoTA7.STM32|securemode=2\\,signmethod=hmacsha256\\,timestamp=1721780638696|\"\r\n"
#define MQTTCONN "AT+MQTTCONN=0,\"iot-06z00jlxn39wea0.mqtt.iothub.aliyuncs.com\",1883,1\r\n"



unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

extern   UART_HandleTypeDef huart2;
#define  ESP8266_USART      (&huart2)
/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	USARTx：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Usart_SendString(UART_HandleTypeDef *USARTx, unsigned char *str, unsigned short len)
{
	unsigned short count = 0;
	
	if (str == NULL || USARTx == NULL) 
	{
			return;
	}
	for (; count < len; count++) 
	{
		if (str[count] == '\0') 
		{
				break;
		}
		HAL_UART_Transmit(USARTx, (uint8_t *)(str + count), 1, 10);
	}
}

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	return REV_WAIT;								//返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;
	
	myPrint("\r\nWifiTx = %s, strlen: %d\n", cmd,strlen((const char *)cmd));
	HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen((const char *)cmd), 0xffff);
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			myPrint("[len: %d]esp8266_buf : %s\n",strlen(esp8266_buf), esp8266_buf);
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
			else
			{
				ESP8266_Clear();
			}
		}
		HAL_Delay(10);
	}
	return 1;
}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		//Usart_SendString(ESP8266_USART, data, len);		//发送设备连接请求数据
		HAL_UART_Transmit(ESP8266_USART, (uint8_t *)data, len, 0xffff);
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
	char *ptrIPD = NULL;
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			myPrint("%s\n", esp8266_buf);
			ptrIPD = strstr((char *)esp8266_buf, "+MQTTSUBRECV:");				//搜索“IPD”头
			if(ptrIPD != NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				ptrIPD = strchr(ptrIPD, '{');							//找到':'
				if(ptrIPD != NULL)
				{
					return (unsigned char *)(ptrIPD);
				}
			}
		}
		HAL_Delay(1);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针
}

//unsigned char *ESP8266_GetIPD(unsigned short timeOut)
//{

//	char *ptrIPD = NULL;
//	
//	do
//	{
//		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
//		{
//			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索“IPD”头
//			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
//			{
//				//printf("\"IPD\" not found\r\n");
//			}
//			else
//			{
//				ptrIPD = strchr(ptrIPD, ':');							//找到':'
//				if(ptrIPD != NULL)
//				{
//					ptrIPD++;
//					return (unsigned char *)(ptrIPD);
//				}
//				else
//					return NULL;
//			}
//		}
////		delay_ms(5);													//延时等待
//		HAL_Delay(1);													//延时等待
//	} while(timeOut--);
//	
//	return NULL;														//超时还未找到，返回空指针
//}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Init(void)
{
	uint8_t log_line = 32;
	
	HAL_Delay(500);
	ESP8266_Clear();
	ESP8266_SendCmd("AT+RST\r\n", "OK");
	HAL_Delay(500);
	myPrint("[len: %d]esp8266_buf : %s\n",strlen(esp8266_buf), esp8266_buf);
#if 1
	myPrint("1. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK")) 
	{
		HAL_Delay(500);
		myPrint("Sending AT command failed, retrying...\r\n");		
	}
	//LCD_print_log(0, log_line*0, (uint8_t *)"1.AT");
	
	myPrint("2. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))  //配置为STA模式
	{
		HAL_Delay(500);
		myPrint("Setting CWMODE failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*1, (uint8_t *)"2.AT+CWMODE");
	
	myPrint("3. AT+CIPSNTPCFG\r\n");
	while(ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"\r\n", "OK")) 
	{
		HAL_Delay(500);
		myPrint("Setting CWDHCP failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*2, (uint8_t *)"3.AT+CIPSNTPCFG");
	
	myPrint("4. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP")) 
	{
		HAL_Delay(500);
		myPrint("Connecting to WiFi failed, retrying...\r\n");
	}
	myPrint("esp 3266 Connect to WIFI OK!!!\r\n");
	//LCD_print_log(0, log_line*3, (uint8_t *)"4.AT+CWJAP");
#endif
	#if 0
	//设备名、密码
	myPrint("5.AT+MQTTUSERCFG\r\n");
	while(ESP8266_SendCmd(MQTTUSERCFG, "OK")) 
	{
		HAL_Delay(500);
		//myPrint("Starting TCP connection failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*4, (uint8_t *)"5.AT+MQTTUSERCFG");
	
	myPrint("6.AT+MQTTCLIENTID\r\n");
	while(ESP8266_SendCmd(MQTTCLIENTID, "OK")) 
	{
		HAL_Delay(500);
		//myPrint("Starting TCP connection failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*5, (uint8_t *)"6.AT+MQTTCLIENTID");
	
	myPrint("7.AT+MQTTCONN\r\n");
	while(ESP8266_SendCmd(MQTTCONN, "OK")) 
	{
		HAL_Delay(500);
		myPrint("MQTTCONN failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*6, (uint8_t *)"7.AT+MQTTCONN");
	
	//LCD_Clear(WHITE); //清屏
	myPrint("8.AT+MQTTSUB\r\n");
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\"/sys/k1h2hJkoTA7/STM32/thing/service/property/set\",1\r\n", "OK")) 
	{
		HAL_Delay(500);
		myPrint("MQTTSUB failed, retrying...\r\n");
	}
	//LCD_print_log(0, log_line*0, (uint8_t *)"8.AT+MQTTSUB");
	//LCD_print_log(0, log_line*1, (uint8_t *)"Cloud connected");
	
	myPrint("Cloud connection successful\n");
	#endif
}

#if 0
void Get_Data_From_Cloud(void) 
{
    unsigned char *data = ESP8266_GetIPD(200);
    if (data != NULL) 
    {
        // 处理接收到的数据，例如：
        printf("Received data: %s\n", data);

        // 根据需要解析 JSON 数据
        cJSON *json = cJSON_Parse((char *)data);
        if (json != NULL) 
        {
            // 解析并处理 LED 数据
            cJSON *led = cJSON_GetObjectItem(json, "LED");
            if (cJSON_IsNumber(led)) 
            {
                printf("LED: %d\n", led->valueint);
								LED_TOGGLE(1);
                //在这里处理 LED 数据
            } 
            else 
            {
                printf("Error: LED is not a number\n");
            }

            // 解析并处理 door 数据
            cJSON *door = cJSON_GetObjectItem(json, "door");
            if (cJSON_IsNumber(door)) 
            {
                printf("door: %d\n", door->valueint);
								Toggle_Door();
                // 在这里处理 door 数据
            } 
            else 
            {
                printf("Error: door is not a number\n");
            }

            // 解析并处理 beep 数据
            cJSON *beep = cJSON_GetObjectItem(json, "beep");
            if (cJSON_IsNumber(beep)) 
            {
              printf("beep: %d\n", beep->valueint);
							BEEP_TOGGLE(1,100);
                // 在这里处理 beep 数据
            } 
            else 
            {
                printf("Error: beep is not a number\n");
            }

            cJSON_Delete(json);
        } 
        else 
        {
            printf("Error: Failed to parse JSON\n");
        }
    } 
    else 
    {
        //printf("Error: No data received\n");
    }
}


extern float Light_Value;
extern unsigned int rec_data[4];


void Send_Data_To_Cloud(void)
{
	char buf[256];
	int i = 0;
	
	//自定义云产品转发上传主题
	sprintf(buf, "AT+MQTTPUB=0,\"/k1h2hJkoTA7/STM32/user/Android_STM32\",\"{\\\"params\\\":{\\\"Temp\\\":%d}}\",1,0\r\n", rec_data[2]);
	printf("WifiTx = %s", buf);
	//ESP8266_SendCmd(buf, NULL);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, strlen((const char *)buf), 0xffff);
	memset(buf, 0, sizeof(buf));
	ESP8266_Clear();									//清空缓存
	HAL_Delay(100);
	
	sprintf(buf, "AT+MQTTPUB=0,\"/k1h2hJkoTA7/STM32/user/Android_STM32\",\"{\\\"params\\\":{\\\"Humi\\\":%d}}\",1,0\r\n", rec_data[0]);
	printf("WifiTx = %s", buf);
	//ESP8266_SendCmd(buf, NULL);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, strlen((const char *)buf), 0xffff);
	memset(buf, 0, sizeof(buf));
	ESP8266_Clear();									//清空缓存
	HAL_Delay(100);
	
	sprintf(buf, "AT+MQTTPUB=0,\"/k1h2hJkoTA7/STM32/user/Android_STM32\",\"{\\\"params\\\":{\\\"light\\\":%d}}\",1,0\r\n", (uint8_t)Light_Value);
	printf("WifiTx = %s", buf);
	//ESP8266_SendCmd(buf, NULL);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, strlen((const char *)buf), 0xffff);
	memset(buf, 0, sizeof(buf));
	ESP8266_Clear();									//清空缓存
	HAL_Delay(100);
	
	
}

void Send_Data_To_Cloud_Phy_model(void)
{
	//物模型上传主题
//	sprintf(buf, "AT+MQTTPUB=0,\"/sys/k1h2hJkoTA7/STM32/thing/event/property/post\",\"{\\\"params\\\":{\\\"Temp\\\":%d}}\",1,0\r\n", rec_data[2]);
//	//printf("%s", buf);
//	ESP8266_SendCmd(buf, NULL);
//	memset(buf, 0, sizeof(buf));
//	//HAL_Delay(1);
//	
//	sprintf(buf, "AT+MQTTPUB=0,\"/sys/k1h2hJkoTA7/STM32/thing/event/property/post\",\"{\\\"params\\\":{\\\"Humi\\\":%d}}\",1,0\r\n", rec_data[0]);
//	//printf("%s", buf);
//	ESP8266_SendCmd(buf, NULL);
//	memset(buf, 0, sizeof(buf));
//	//HAL_Delay(1);
//	
//	sprintf(buf, "AT+MQTTPUB=0,\"/sys/k1h2hJkoTA7/STM32/thing/event/property/post\",\"{\\\"params\\\":{\\\"light\\\":%d}}\",1,0\r\n", (uint8_t)Light_Value);
//	//printf("%s", buf);
//	ESP8266_SendCmd(buf, NULL);
//	memset(buf, 0, sizeof(buf));
}

#endif

