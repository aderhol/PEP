#ifndef MESCONCENTRATION_INT_H
#define MESCONCENTRATION_INT_H

#include <stdint.h>

#include <FreeRTOS.h>
#include <timers.h>

static void MesConcentration(void *pvParam);
static void Timer(TimerHandle_t xTimer);
static float PPM_Of_CO2();
static void ADC_Single_Conv(uint32_t *result);
static void Init_ADC();


#endif
