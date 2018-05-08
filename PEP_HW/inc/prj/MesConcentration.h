#ifndef MESCONCENTRATION_H
#define MESCONCENTRATION_H

#include <stdbool.h>
#include <stdint.h>

extern void MesConcentraitonInit(void);

typedef struct{
	float concentration;
	uint32_t time;
}Concentration;
extern bool MesConcentrationGet_concentration(Temperature*);

#endif
