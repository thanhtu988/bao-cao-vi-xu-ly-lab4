#include <stdio.h>
#include <string.h>
#include "main.h"
#include "scheduler.h"
#include "uart_print.h"

extern UART_HandleTypeDef huart2;

void print_text(const char *text)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)text, strlen(text), 100);
}

void print_time(const char *text)
{
  char buf[64];
  int n = snprintf(buf, sizeof buf, "[%6lu ms] %s\r\n", (unsigned long)get_time(), text);
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, n, 100);
}
