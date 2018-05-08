#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#include <stdbool.h>
#include <stdint.h>

extern void SystemClockInit(void);
extern bool SystemClockGet_ms(uint32_t* time_ms);

#endif