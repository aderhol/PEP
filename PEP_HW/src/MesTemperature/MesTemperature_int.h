#ifndef MESTEMPERATURE_INT_H
#define MESTEMPERATURE_INT_H

#include <stdint.h>

#include <FreeRTOS.h>
#include <timers.h>

static void Init_I2C(void);
static void I2C_Receive( uint8_t* temperature, uint16_t len);
static void MesTemperature(void *pvParam);
static void Timer(TimerHandle_t xTimer);

#endif
