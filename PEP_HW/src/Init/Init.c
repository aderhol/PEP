#include <stdint.h>
#include <stdlib.h>

#include <em_cmu.h>
#include <em_adc.h>

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

bool global_IsInit = false;

static void randomInit(void);

void Init(void)
{
	randomInit();
	
	//initialization of the system
	MesConcentrationInit();
	MesTemperatureInit();
	StatusIndInit();
	SystemClockInit();
	CommandFromUARTInit();
	MessengerUARTInit();
	AlarmLEDInit();

	global_IsInit = true;
}

//initializes the seed for rand() with a random number
static void randomInit(void)
{
	CMU_ClockEnable(cmuClock_ADC0, true);								//turns on the clock for the ADC
	ADC_Init_TypeDef initstruct = ADC_INIT_DEFAULT;						//creates an ADC single mode initialization structure for default setup
	ADC_Init(ADC0, &initstruct);										//initializes the ADC with default setup
	ADC_InitSingle_TypeDef initstruct_single = ADC_INITSINGLE_DEFAULT;	//creates an ADC single mode initialization structure for default setup
	initstruct_single.input = adcSingleInputTemp;						//sets the input channel for the output of the internal temperature sensor
	ADC_InitSingle(ADC0, &initstruct_single);							//initializes the ADC in single mode

	unsigned int seed = 1;												//the seed starts of being equal to 1

	//gets 3 random number between 1 and 100
	//and assigns their product to seed
	for(int i = 0; i < 3; i++){
		ADC_Start(ADC0, adcStartSingle);								//start the measurement
		while(!(ADC0->STATUS & ADC_STATUS_SINGLEDV));					//waits until the measurement is done, and the data is valid
		seed *= (unsigned int)((ADC_DataSingleGet(ADC0) % 100) + 1);	//multiplies the seeds current value with the random value of the last two decimal digits + 1 of the measured temperature sensor output
	}

	CMU_ClockEnable(cmuClock_ADC0, false);								//turns off the clock for the ADC (disables it)

	srand(seed);														//initializes the standard librariy's rand() with the seed
}

//default initialization routine
static void defaultInit(void)
{
	return;
}


//weak defines for the initialization routines

void __attribute__((weak)) 	AlarmLEDInit(void)
{
	defaultInit();
}
