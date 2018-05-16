#include <stdbool.h>

#include "em_core.h"
#include "em_cmu.h"
#include "em_gpio.h"

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <AlarmLED.h>

#include <common.h>

static bool alarm = false;

static void ledHandler(void* pvParam);
static void watcher(TimerHandle_t xTimer);

void AlarmLEDInit(void)
{
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);
	
	xTaskCreate(ledHandler, "AlarmLED_ledHandler", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);
	xTimerStart(xTimerCreate("AlarmLED_watcher_timer", configTICK_RATE_HZ, pdTRUE, NULL, watcher), 0);
}


static void watcher(TimerHandle_t xTimer)
{
	static int count = 0;
	
	if(GPIO_PinInGet(gpioPortD, 1) == 0)
		count += (count < 3) ? 1 : 0;
	else
		count = 0;
	
	if(count >= 3){
		CORE_CRITICAL_SECTION(
				alarm = true;
		);	
	}
	else{
		CORE_CRITICAL_SECTION(
				alarm = false;
		);			
	}
}


static void ledHandler(void* pvParam)
{
	while(true){
		bool isAlarm;
		CORE_CRITICAL_SECTION(
				isAlarm = alarm;
		);
		if(isAlarm){
			BSP_LedToggle(0);
		}
		else{
			BSP_LedClear(0);
		}
		
		vTaskDelay(configTICK_RATE_HZ/10);	
	}
}
