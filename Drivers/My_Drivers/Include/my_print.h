#ifndef __MY_PRINT_H__
#define __MY_PRINT_H__

/* myPrint.h */
#include <stdarg.h>
void myPrint(const char *format, ...);
void send_char(char ch);  // 需用户实现的底层发送函数（如串口发送）

#endif