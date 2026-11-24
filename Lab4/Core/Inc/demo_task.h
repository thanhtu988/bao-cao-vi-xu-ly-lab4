#ifndef INC_DEMO_TASK_H_
#define INC_DEMO_TASK_H_

#include <stdint.h>

extern uint32_t id_task_10ms;

void Task_10ms(void);
void OneShot_1200ms(void);
void OneShot_3000ms(void);
void OneShot_4500ms(void);

#endif
