#include "main.h"
#include "led_task.h"
#include "uart_print.h"

void LED1_Toggle(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  print_time("LED1 - 0.5 s");
}

void LED2_Toggle(void)
{
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  print_time("LED2 - 1 s");
}

void LED3_Toggle(void)
{
  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  print_time("LED3 - 1.5 s");
}

void LED4_Toggle(void)
{
  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  print_time("LED4 - 2 s");
}

void LED5_Toggle(void)
{
  HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
  print_time("LED5 - 2.5 s");
}
