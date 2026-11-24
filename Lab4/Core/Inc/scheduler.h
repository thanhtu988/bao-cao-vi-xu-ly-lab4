#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_TICK_MS                    10
#define SCH_MAX_TASKS                  40
#define NO_TASK_ID                     0

#define RETURN_NORMAL                  0
#define RETURN_ERROR                   1

#define ERROR_SCH_TOO_MANY_TASKS       1
#define ERROR_SCH_CANNOT_DELETE_TASK   2

typedef struct {
  void (*pTask)(void);
  uint32_t Delay;
  uint32_t Period;
  uint8_t RunMe;
  uint32_t TaskID;
  uint8_t Next;
} sTask;

extern sTask SCH_tasks_G[SCH_MAX_TASKS];
extern unsigned char Error_code_G;

void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
uint32_t get_time(void);
void SCH_Report_Status(void);
void SCH_Go_To_Sleep(void);

#endif
