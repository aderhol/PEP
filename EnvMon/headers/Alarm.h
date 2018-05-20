#ifndef ALARM_H
#define ALARM_H

#include <pthread.h>
#include <stdbool.h>

bool sendToAlarm(char const* data);
bool AlarmInit(pthread_t* threads, int *count, int countMax);

#endif
