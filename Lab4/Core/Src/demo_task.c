#include "main.h"
#include "scheduler.h"
#include "uart_print.h"
#include "demo_task.h"

uint32_t id_task_10ms = NO_TASK_ID;
static uint32_t count_10ms = 0;

void Task_10ms(void)
{
  print_time("task 10 ms");
  count_10ms++;
  if (count_10ms == 100) {
    SCH_Delete_Task(id_task_10ms);
    print_time("delete task 10 ms");
  }
}

void OneShot_1200ms(void)
{
  print_time("one-shot 1200 ms");
}

void OneShot_3000ms(void)
{
  print_time("one-shot 3000 ms");
}

void OneShot_4500ms(void)
{
  print_time("one-shot 4500 ms");
}
