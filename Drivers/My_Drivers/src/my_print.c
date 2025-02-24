#include "my_print.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

// 轻量级格式化核心（支持%d, %u, %x, %s, %c）
void myPrint(const char *format, ...) {

    va_list args;
    va_start(args, format);
	
	//taskENTER_CRITICAL();
    while (*format) {
        if (*format == '%') {
            switch (*++format) {
                case 'd': {
                    int num = va_arg(args, int);
                    // 整数转字符串处理
                    char buffer[12];
                    char *p = &buffer[11];
                    *p = '\0';
                    int sign = num < 0 ? -1 : 1;
                    num *= sign;
                    do {
                        *--p = '0' + num % 10;
                        num /= 10;
                    } while (num);
                    if (sign < 0) *--p = '-';
                    while (*p) send_char(*p++);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    while (*str) send_char(*str++);
                    break;
                }
                case 'c': {
                    char ch = va_arg(args, int);
                    send_char(ch);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char hex[] = "0123456789abcdef";
                    char buffer[9];
                    int i = 8;
                    buffer[i--] = '\0';
                    do {
                        buffer[i--] = hex[num & 0xF];
                        num >>= 4;
                    } while (i >= 0 && num);
                    while (buffer[++i]) send_char(buffer[i]);
                    break;
                }
                default:
                    send_char('%');
                    send_char(*format);
            }
        } else {
            send_char(*format);
        }
        format++;
    }
    va_end(args);
	//taskEXIT_CRITICAL();
}

/* 用户需要实现的底层发送函数（示例：HAL库串口发送） */
void send_char(char ch) {
    // 示例：通过串口1发送
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 10);
}