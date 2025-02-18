#ifndef __DRIVER_DHT11_H__
#define __DRIVER_DHT11_H__

#include "stdint.h"
#if 1
/* 类型定义 ------------------------------------------------------------------*/
/************************ DHT11 数据类型定义******************************/
typedef struct
{
	uint8_t  humi_high8bit;		//原始数据：湿度高8位
	uint8_t  humi_low8bit;	 	//原始数据：湿度低8位
	uint8_t  temp_high8bit;	 	//原始数据：温度高8位
	uint8_t  temp_low8bit;	 	//原始数据：温度高8位
	uint8_t  check_sum;	 	    //校验和
  float    humidity;        //实际湿度
  float    temperature;     //实际温度  
} DHT11_Data_TypeDef;

/* 宏定义 -------------------------------------------------------------------*/
/***********************   DHT11 连接引脚定义  **************************/
#define DHT11_Dout_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOB_CLK_ENABLE()
#define DHT11_Dout_PORT                           GPIOB
#define DHT11_Dout_PIN                            GPIO_PIN_15

/***********************   DHT11 函数宏定义  ****************************/
#define DHT11_Dout_LOW()                          HAL_GPIO_WritePin(DHT11_Dout_PORT, DHT11_Dout_PIN, GPIO_PIN_RESET)
#define DHT11_Dout_HIGH()                         HAL_GPIO_WritePin(DHT11_Dout_PORT, DHT11_Dout_PIN, GPIO_PIN_SET)
#define DHT11_Data_IN()	                          HAL_GPIO_ReadPin(DHT11_Dout_PORT,DHT11_Dout_PIN)

/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
void DHT11_Init( void );
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef * DHT11_Data);

void DHT11_Test(void);







#else
#include "stdint.h"

void DHT11_OUT(void );
void DHT11_IN(void );
void DHT11_Reset(void );
uint8_t DHT11_Check(void );
uint8_t DHT11_Read_Bit(void );
uint8_t DHT11_Read_Byte(void );
uint8_t DHT11_Read_Data(uint8_t* humi,uint8_t* temp);

void DHT11_Test(void);
#endif

#endif
