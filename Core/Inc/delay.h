#ifndef DELAY_H
#define DELAY_H

#include "main.h"

extern void delay_init(void);
extern void delay_us(uint32_t nus);
extern void delay_ms(uint32_t nms);


void my_delay_us(uint16_t us);
void my_delay_ms(uint16_t ms);

#endif
