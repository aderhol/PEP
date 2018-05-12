#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#include <stdbool.h>
#include <stdint.h>

void SystemClockInit(void);
bool SystemClockGet_ms(uint32_t* time_ms);

#endif
