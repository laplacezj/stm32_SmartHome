#include "uart_mana.h"
#include "usart.h"
#include "my_print.h"

//extern uint8_t Voice_Data;
uint8_t Voice_Data;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern unsigned char  esp8266_buf[512];
extern unsigned short esp8266_cnt, esp8266_cntPre;

int fputc(int ch, FILE *f)
{
	while( (USART1->SR & 0x40) == 0 );
	USART1->DR = (uint8_t) ch;
	return ch;
}

void setup_uart_interrupt()
{
	HAL_UART_Receive_IT(&huart3, &Voice_Data, 1);
  	HAL_UART_Receive_IT(&huart2, &esp8266_buf[0], 1);
}

uint8_t receivedByte2;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	unsigned int timeout=0;
  	unsigned int maxDelay=0x1FFFF;
	
	if (huart->Instance == USART3) // 确保是期望的UART实例
	{
		HAL_UART_Receive_IT(huart, &Voice_Data, 1);
		myPrint("Voice: %d\r\n", Voice_Data);
	}
	
	if(huart->Instance == USART2)
	{
		while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY)//等待就绪
		{
			timeout++;//超时处理
			if(timeout > maxDelay) break;		
		}
		timeout=0;
		while(HAL_UART_Receive_IT(&huart2, &receivedByte2, 1)!=HAL_OK)
		{
			timeout++; //超时处理
			if(timeout > maxDelay) break;	
		}
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
			esp8266_buf[esp8266_cnt++] = receivedByte2;
		
	}
}




