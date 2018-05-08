#include <Init.h>

#include <common.h>

#include <MesConcentration.h>
#include <MesTemperature.h>
#include <StatusInd.h>
#include <SystemClock.h>
#include <CommandFromUART.h>
#include <MessengerUART.h>
#include <AlarmLED.h>

static void defaultInit(void);

void Init(void)
{
	//initialization of the system
	MesConcentrationInit();
	MesTemperatureInit();
	StatusIndInit();
	SystemClockInit();
	CommandFromUARTInit();
	MessengerUARTInit();
	AlarmLEDInit();
}

//default initialization routine
static void defaultInit(void)
{
	return;
}


//weak defines for the initialization routines

void __attribute__((weak)) 	MesConcentrationInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	MesTemperatureInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	StatusIndInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	SystemClockInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	CommandFromUARTInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	MessengerUARTInit(void)
{
	defaultInit();
}

void __attribute__((weak)) 	AlarmLEDInit(void)
{
	defaultInit();
}
