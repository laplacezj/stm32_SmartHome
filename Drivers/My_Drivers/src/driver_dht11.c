#include "driver_dht11.h"
#include "main.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_oled.h"
#include "stdio.h"
#include "my_print.h"

#if 1
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/

/* 私有变量 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
static void DHT11_Mode_IPU(void);
static void DHT11_Mode_Out_PP(void);
static uint8_t DHT11_ReadByte(void);
unsigned char   ple[]="0123456789";
extern DHT11_Data_TypeDef DHT11_Data;
#define Bit_RESET 0
#define Bit_SET   1

#if 0
//等待us级别
void delay_us(unsigned long i)
{
	unsigned long j;
	for(;i>0;i--)
	{
			for(j=12;j>0;j--);
	}
}
#endif

/**
  * 函数功能: DHT11 初始化函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void DHT11_Init ( void )
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	DHT11_Mode_Out_PP();
	
	DHT11_Dout_HIGH();  // 拉高GPIO
}

/**
  * 函数功能: 使DHT11-DATA引脚变为上拉输入模式
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void DHT11_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
}

/**
  * 函数功能: 使DHT11-DATA引脚变为推挽输出模式
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void DHT11_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
}

/**
  * 函数功能: 从DHT11读取一个字节，MSB先行
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static uint8_t DHT11_ReadByte ( void )
{
	uint8_t i, temp=0;
	
	for(i=0;i<8;i++)    
	{	 
		/*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/  
		while(DHT11_Data_IN()==Bit_RESET);

		/*DHT11 以26~28us的高电平表示“0”，以70us高电平表示“1”，
		 *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时 
		 */
		my_delay_us(40); //延时x us 这个延时需要大于数据0持续的时间即可	   	  

		if(DHT11_Data_IN()==Bit_SET)/* x us后仍为高电平表示数据“1” */
		{
			/* 等待数据1的高电平结束 */
			while(DHT11_Data_IN()==Bit_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //把第7-i位置1，MSB先行 
		}
		else	 // x us后为低电平表示数据“0”
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //把第7-i位置0，MSB先行
		}
	}
	return temp;
}

/**
  * 函数功能: 一次完整的数据传输为40bit，高位先出
  * 输入参数: DHT11_Data:DHT11数据类型
  * 返 回 值: ERROR：  读取出错
  *           SUCCESS：读取成功
  * 说    明：8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和 
  */
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint8_t temp;
  uint16_t humi_temp;
  
	/*输出模式*/
	DHT11_Mode_Out_PP();
	/*主机拉低*/
	DHT11_Dout_LOW();
	/*延时18ms*/
	my_delay_ms(20);

	/*总线拉高 主机延时30us*/
	DHT11_Dout_HIGH(); 

	my_delay_us(30);   //延时30us

	/*主机设为输入 判断从机响应信号*/ 
	DHT11_Mode_IPU();
	my_delay_us(30);   //延时30us
	/*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/   
	if(DHT11_Data_IN()==Bit_RESET)     
	{
    /*轮询直到从机发出 的80us 低电平 响应信号结束*/  
    while(DHT11_Data_IN()==Bit_RESET);

    /*轮询直到从机发出的 80us 高电平 标置信号结束*/
    while(DHT11_Data_IN()==Bit_SET);

    /*开始接收数据*/   
    DHT11_Data->humi_high8bit= DHT11_ReadByte();
    DHT11_Data->humi_low8bit = DHT11_ReadByte();
    DHT11_Data->temp_high8bit= DHT11_ReadByte();
    DHT11_Data->temp_low8bit = DHT11_ReadByte();
    DHT11_Data->check_sum    = DHT11_ReadByte();

    /*读取结束，引脚改为输出模式*/
    DHT11_Mode_Out_PP();
    /*主机拉高*/
    DHT11_Dout_HIGH();
    
    /* 对数据进行处理 */
    humi_temp=DHT11_Data->humi_high8bit*100+DHT11_Data->humi_low8bit;
    DHT11_Data->humidity =(float)humi_temp/100;
    
    humi_temp=DHT11_Data->temp_high8bit*100+DHT11_Data->temp_low8bit;
    DHT11_Data->temperature=(float)humi_temp/100;    
    
    /*检查读取的数据是否正确*/
    temp = DHT11_Data->humi_high8bit + DHT11_Data->humi_low8bit + 
           DHT11_Data->temp_high8bit+ DHT11_Data->temp_low8bit;
    if(DHT11_Data->check_sum==temp)
    { 
		myPrint("dht11 get seccess\n");
		return SUCCESS;
    }
    else
	{
		myPrint("dht11 error111\n");
		return ERROR;
	}
    
	}	
	else
	{
		myPrint("dht11 error222\n");
		return ERROR;
	}
		
}


void DHT11_Test(void)
{
	DHT11_Data_TypeDef DHT11_Data;
	while (1)
	{
		HAL_Delay(5000);
		taskENTER_CRITICAL();
		DHT11_Read_TempAndHumidity(&DHT11_Data);
		taskEXIT_CRITICAL();
		OLED_ShowString(2,1,"HUMI:  .  ");
		OLED_ShowNum(2,6,DHT11_Data.humi_high8bit,2);
		OLED_ShowNum(2,9,0,2);

		OLED_ShowString(3,1,"TEMP:  .  ");
		OLED_ShowNum(3,6,DHT11_Data.temp_high8bit,2);
		OLED_ShowNum(3,9,0,2);
	}
	
}





#else
#define DHT11_HIGH HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET)
#define DHT11_LOW  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET)
#define Read_Data  HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin)

#include "tim.h"


//输出
void DHT11_OUT(void ){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}
//输入
void DHT11_IN(void ){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}
//复位DHT11
void DHT11_Reset(void ){
	myPrint("DHT11_Reset\r\n");
    DHT11_OUT();                                          //设置为输出
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);     //拉低引脚
    my_delay_ms(20);                                             //延迟20ms
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);       //拉高引脚
    my_delay_us(30);
}
//等待DHT11回应
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IN();                                           //设置为输入
    while(GPIO_PIN_SET==HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) && retry<100)
    {
        retry++;
        my_delay_us(1);
    }
    if(retry>=100)
        return 1;
    else
        retry=0;

    while(GPIO_PIN_RESET==HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) && retry<100)
    {
        retry++;
        my_delay_us(1);
    }
    if(retry>=100)
        return 1;
    return 0;
}
//从DHT11读取一个位
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(GPIO_PIN_SET==HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) && retry<100)
    {
        retry++;
        my_delay_us(1);
    }
    retry=0;

    while(GPIO_PIN_RESET==HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) && retry<100)
    {
        retry++;
        my_delay_us(1);
    }
    my_delay_us(40);

    if(GPIO_PIN_SET==HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin))
        return 1;
    else
        return 0;
}

//从DHT11读取一个字节
uint8_t DHT11_Read_Byte(void)
{
    uint8_t dat=0;
    for(uint8_t i=0;i<8;i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}
//从DHT11读取一次数据
uint8_t DHT11_Read_Data(uint8_t* humi,uint8_t* temp)
{
    uint8_t buf[5];
    DHT11_Reset();
    if(DHT11_Check() == 0)
    {
        for(uint8_t i=0;i<5;i++)
            buf[i]=DHT11_Read_Byte();
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
            *humi=buf[0];       //这里省略小数部分
            *temp=buf[2];
        }
    }
    else
        return 1;
    return 0;
}


void DHT11_Test(void)
{
	uint8_t humi, temp;
	while (1)
	{
		HAL_Delay(5000);
		taskENTER_CRITICAL();
		DHT11_Read_Data(&humi, &temp);
		taskEXIT_CRITICAL();
		OLED_ShowString(2,1,"HUMI:  .  ");
		OLED_ShowNum(2,6,humi,2);
		OLED_ShowNum(2,9,0,2);

		OLED_ShowString(3,1,"TEMP:  .  ");
		OLED_ShowNum(3,6,temp,2);
		OLED_ShowNum(3,9,0,2);
	}
	
}

#endif


