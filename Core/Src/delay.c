#include "delay.h"
#include "stm32_hal_legacy.h"

#define USE_HAL_LEGACY
//#define Timebase_Source_is_SysTick 1	//当Timebase Source为SysTick时改为1
#define Timebase_Source_is_SysTick 0	//当使用FreeRTOS，Timebase Source为其他定时器时改为0

#if	(!Timebase_Source_is_SysTick)
	extern TIM_HandleTypeDef htim4;		//当使用FreeRTOS，Timebase Source为其他定时器时，修改为对应的定时器
	#define Timebase_htim htim4

	#define Delay_GetCounter()		__HAL_TIM_GetCounter(&Timebase_htim)
	#define Delay_GetAutoreload()	__HAL_TIM_GetAutoreload(&Timebase_htim)
#else
	#define Delay_GetCounter()		(SysTick->VAL)
	#define Delay_GetAutoreload()	(SysTick->LOAD)
#endif

static uint16_t fac_us = 0;
static uint32_t fac_ms = 0;


/*初始化*/
void delay_init(void)
{
#if	(!Timebase_Source_is_SysTick)
	fac_ms = 1000000;				//作为时基的计数器时钟频率在HAL_InitTick()中被设为了1MHz
	fac_us = fac_ms / 1000;
#else
	fac_ms = SystemCoreClock / 1000;
	fac_us = fac_ms / 1000;
#endif
}


/*微秒级延时*/
void delay_us(uint32_t nus)
{
	uint32_t ticks = 0;
	uint32_t told = 0;
	uint32_t tnow = 0;
	uint32_t tcnt = 0;
	uint32_t reload = 0;

	reload = Delay_GetAutoreload();

	ticks = nus * fac_us;

	told = Delay_GetCounter();

	while (1)
	{
		tnow = Delay_GetCounter();

		if (tnow != told)
		{
			if (tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if (tcnt >= ticks)
			{
				break;
			}
		}
	}
}

/*毫秒级延时*/
void delay_ms(uint32_t nms)
{
	uint32_t ticks = 0;
	uint32_t told = 0;
	uint32_t tnow = 0;
	uint32_t tcnt = 0;
	uint32_t reload = 0;

	reload = Delay_GetAutoreload();

	ticks = nms * fac_ms;

	told = Delay_GetCounter();

	while (1)
	{
		tnow = Delay_GetCounter();

		if (tnow != told)
		{
			if (tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if (tcnt >= ticks)
			{
				break;
			}
		}
	}
}




#include "tim.h"

#define MY_DELAY_TIM htim3
#define P_MY_DELAY_TIM (&htim3)


/**
 * ************************************************************************
 * @brief 基于16位定时器构造的微秒延时函数
 * 
 * @param[in] us  微秒值
 * @note 选择32位定时器，可防止计数值溢出，避免加大运算量
 * 
 * ************************************************************************
 */
void my_delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(P_MY_DELAY_TIM, 0);
    
    HAL_TIM_Base_Start(P_MY_DELAY_TIM);
    
    while (__HAL_TIM_GET_COUNTER(P_MY_DELAY_TIM) != us);
    
    HAL_TIM_Base_Stop(P_MY_DELAY_TIM);
}
/**
 * ************************************************************************
 * @brief 基于32位定时器构造的毫秒延时函数
 * 
 * @param[in] ms  毫秒值
 * 
 * ************************************************************************
 */
void my_delay_ms(uint16_t ms)
{
    my_delay_us(ms * 1000);
}




