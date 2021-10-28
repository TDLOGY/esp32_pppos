#ifndef PPPOS_TASK_H_
#define PPPOS_TASK_H_

#include <stdbool.h>

void pppos_reset(void);
bool pppos_is_connected(void);
void pppos_task_init(void);

char* pppos_get_imei(void);
char* pppos_get_imsi(void);

#endif