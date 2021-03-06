#include <stdbool.h>

#include "em_core.h"

#include <bsp.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <StatusInd.h>
#include "StatusInd_int.h"

#include <common.h>

#include <CommandFromUART.h>


static volatile Status status;

void StatusIndInit(void)
{
	if(!CommandFromUARTGet_status(&status))
		status = UNKNOWN;
	xTaskCreate(ledHandler, "StatusInd_ledHandler", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);
	xTimerStart(xTimerCreate("StatusInd_statusGet_timer", configTICK_RATE_HZ / 10, pdTRUE, NULL, statusGet), 0);
}

static void ledHandler(void* pvParam)
{
	Status st;
	while(true)
	{
		CORE_CRITICAL_SECTION(
				st = status;
		);
		TickType_t delay;// = configTICK_RATE_HZ / 2;

		switch(st){
			case INIT:
				BSP_LedClear(1);
				delay = configTICK_RATE_HZ / 1000;
				break;

			case UNCONNECTED:
				BSP_LedToggle(1);
				delay = configTICK_RATE_HZ / 2;
				break;

			case CONNECTED:
				BSP_LedSet(1);
				delay = configTICK_RATE_HZ / 10;
				break;

			case UNKNOWN:
				BSP_LedToggle(1);
				delay = configTICK_RATE_HZ / 20;
				break;

			default:
				while(1); //unexpected status
				break;
		}

		vTaskDelay(delay);	//sleep
	}
}

static void statusGet(TimerHandle_t xTimer)
{
	if(!CommandFromUARTGet_status(&status))
		status = UNKNOWN;
}
