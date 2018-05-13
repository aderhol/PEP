#include <stdint.h>
#include <stdbool.h>

#include "em_rtc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_core.h"

#include <SystemClock.h>

#include <common.h>

static volatile uint64_t time_ms = 0;								//the system time, incremented in every millisecond

static bool isInit = false;

//initialize the system time
void SystemClockInit(void)
{
	/* Ensure LE modules are accessible */
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* Enable LFRCO as LFACLK in CMU (will also enable oscillator if not enabled) */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

	/* RTC Prescaler  */
	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_8);

	CMU_ClockEnable(cmuClock_RTC, true);							//turn on the clock for the RTC

	RTC_CompareSet(0, CMU_ClockFreqGet(cmuClock_RTC) / 1000);		//set RTC's comparator such, that interrupts occur in every millisecond
	RTC_IntClear(0b111);											//clear all potentially pending RTC interrupts
	RTC_IntEnable(0b10);											//enable interrupts raised by RTC's comparator (CMP0)
	RTC_Init_TypeDef init = RTC_INIT_DEFAULT;						//initializer object: RTC with default setup
	RTC_Init(&init);												//RTC initialized (with default setup) and started
	NVIC_EnableIRQ(RTC_IRQn);										//enable interrupts raised by RTC
	isInit = true;
}

//RTC interrupt service routine
void RTC_IRQHandler(void)
{
	RTC_IntClear(0b111);											//clear all RTC interrupts
	CORE_CRITICAL_SECTION(
			time_ms++;												//increment system time
	);
}

bool SystemClockGet_ms(uint32_t* time)
{
	if(!isInit)
		return false;

	CORE_CRITICAL_SECTION(
			*time = time_ms;
	);
	return true;
}
