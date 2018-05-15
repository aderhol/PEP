
#ifndef COMMANDFROMUART_INT_H
#define COMMANDFROMUART_INT_H

#include <stdbool.h>
#include <timers.h>

static void read(void* pvParam);
static void read_timer(TimerHandle_t xTimer);
static bool connectionResp(int ID);
static int connectionReq(int ID);
static void connection_timer(TimerHandle_t xTimer);
static void connection_pinger(void* pvParam);
static void connection_pinger_timer(TimerHandle_t xTimer);

#endif
