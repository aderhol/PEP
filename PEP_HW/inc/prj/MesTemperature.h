#ifndef MESTEMPERATURE_H
#define MESTEMPERATURE_H

#include <stdbool.h>
#include <stdint.h>

void MesTemperatureInit(void);

typedef struct{
	float temperature;
	uint32_t time;
}Temperature;
bool MesTemperatureGet_temperature(Temperature*);

#endif
