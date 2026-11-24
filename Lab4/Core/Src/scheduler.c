#include "main.h"
#include "scheduler.h"

#define NO_INDEX  0xFF

sTask SCH_tasks_G[SCH_MAX_TASKS];
unsigned char Error_code_G = 0;

static uint8_t head;
static uint8_t free_head;
static uint8_t ready[SCH_MAX_TASKS];
static uint8_t ready_in;
static uint8_t ready_out;
static uint8_t ready_count;
static uint32_t next_id;
static volatile uint32_t tick_count;

static void ready_push(uint8_t index)
{
  ready[ready_in] = index;
  ready_in = (ready_in + 1) % SCH_MAX_TASKS;
  ready_count++;
}

static uint8_t ready_pop(void)
{
  uint8_t index = ready[ready_out];
  ready_out = (ready_out + 1) % SCH_MAX_TASKS;
  ready_count--;
  return index;
}

static void list_insert(uint8_t index, uint32_t delay)
{
  uint8_t prev = NO_INDEX;
  uint8_t cur = head;

  while (cur != NO_INDEX && SCH_tasks_G[cur].Delay <= delay) {
    delay -= SCH_tasks_G[cur].Delay;
    prev = cur;
    cur = SCH_tasks_G[cur].Next;
  }
  SCH_tasks_G[index].Delay = delay;
  SCH_tasks_G[index].Next = cur;
  if (cur != NO_INDEX) SCH_tasks_G[cur].Delay -= delay;
  if (prev == NO_INDEX) head = index;
  else SCH_tasks_G[prev].Next = index;
}

static uint8_t list_remove(uint8_t index)
{
  uint8_t prev = NO_INDEX;
  uint8_t cur = head;

  while (cur != NO_INDEX && cur != index) {
    prev = cur;
    cur = SCH_tasks_G[cur].Next;
  }
  if (cur == NO_INDEX) return 0;
  if (SCH_tasks_G[cur].Next != NO_INDEX)
    SCH_tasks_G[SCH_tasks_G[cur].Next].Delay += SCH_tasks_G[cur].Delay;
  if (prev == NO_INDEX) head = SCH_tasks_G[cur].Next;
  else SCH_tasks_G[prev].Next = SCH_tasks_G[cur].Next;
  return 1;
}

static void slot_free(uint8_t index)
{
  SCH_tasks_G[index].pTask = 0;
  SCH_tasks_G[index].Delay = 0;
  SCH_tasks_G[index].Period = 0;
  SCH_tasks_G[index].RunMe = 0;
  SCH_tasks_G[index].TaskID = NO_TASK_ID;
  SCH_tasks_G[index].Next = free_head;
  free_head = index;
}

void SCH_Init(void)
{
  free_head = NO_INDEX;
  for (int i = SCH_MAX_TASKS - 1; i >= 0; i--) {
    slot_free((uint8_t)i);
  }
  head = NO_INDEX;
  ready_in = 0;
  ready_out = 0;
  ready_count = 0;
  next_id = 0;
  tick_count = 0;
  Error_code_G = 0;
}

void SCH_Update(void)
{
  tick_count++;
  if (head == NO_INDEX) return;
  if (SCH_tasks_G[head].Delay > 0) SCH_tasks_G[head].Delay--;
  while (head != NO_INDEX && SCH_tasks_G[head].Delay == 0) {
    uint8_t index = head;
    head = SCH_tasks_G[index].Next;
    SCH_tasks_G[index].Delay = tick_count;
    SCH_tasks_G[index].RunMe = 1;
    ready_push(index);
  }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
  uint32_t id;

  __disable_irq();
  if (free_head == NO_INDEX || pFunction == 0) {
    __enable_irq();
    Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
    return NO_TASK_ID;
  }
  uint8_t index = free_head;
  free_head = SCH_tasks_G[index].Next;

  next_id++;
  if ((next_id << 8) == 0) next_id = 1;
  id = (next_id << 8) | index;

  SCH_tasks_G[index].pTask = pFunction;
  SCH_tasks_G[index].Period = PERIOD;
  SCH_tasks_G[index].TaskID = id;
  if (DELAY == 0) {
    SCH_tasks_G[index].Delay = tick_count;
    SCH_tasks_G[index].RunMe = 1;
    ready_push(index);
  } else {
    SCH_tasks_G[index].RunMe = 0;
    list_insert(index, DELAY);
  }
  __enable_irq();
  return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
  uint8_t index = (uint8_t)(taskID & 0xFF);

  if (taskID == NO_TASK_ID || index >= SCH_MAX_TASKS) {
    Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
    return RETURN_ERROR;
  }
  __disable_irq();
  if (SCH_tasks_G[index].pTask == 0 || SCH_tasks_G[index].TaskID != taskID) {
    __enable_irq();
    Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
    return RETURN_ERROR;
  }
  if (SCH_tasks_G[index].RunMe) {
    SCH_tasks_G[index].pTask = 0;
    SCH_tasks_G[index].TaskID = NO_TASK_ID;
  } else {
    list_remove(index);
    slot_free(index);
  }
  __enable_irq();
  return RETURN_NORMAL;
}

void SCH_Dispatch_Tasks(void)
{
  while (1) {
    __disable_irq();
    if (ready_count == 0) {
      __enable_irq();
      break;
    }
    uint8_t index = ready_pop();
    __enable_irq();

    void (*task)(void) = SCH_tasks_G[index].pTask;
    if (task != 0) task();

    __disable_irq();
    SCH_tasks_G[index].RunMe = 0;
    if (SCH_tasks_G[index].pTask == 0 || SCH_tasks_G[index].Period == 0) {
      slot_free(index);
    } else {
      uint32_t late = tick_count - SCH_tasks_G[index].Delay;
      uint32_t period = SCH_tasks_G[index].Period;
      if (late >= period) {
        SCH_tasks_G[index].Delay = tick_count;
        SCH_tasks_G[index].RunMe = 1;
        ready_push(index);
      } else {
        list_insert(index, period - late);
      }
    }
    __enable_irq();
  }
  SCH_Report_Status();
  SCH_Go_To_Sleep();
}

uint32_t get_time(void)
{
  return tick_count * SCH_TICK_MS;
}

void SCH_Report_Status(void)
{
}

void SCH_Go_To_Sleep(void)
{
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) {
    SCH_Update();
  }
}
