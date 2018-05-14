#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>



#include <retargetserial.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>

#include <CommandFromUART.h>
#include "CommandFromUART_int.h"

#include <common.h>

void CommandFromUARTInit(void)
{
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(1);
	xTaskCreate(read, "CommandFromUART_read_task", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);
	//xTimerStart(xTimerCreate("MessengerUART_messenger_timer", configTICK_RATE_HZ / 100, pdTRUE, NULL, messenger_timer), 0);
}

static void read(void* pvParam)
{
	/*
	while(true){
		char input[100];
		if(gets(input) != NULL){
			volatile int a = 1;
		}
		vTaskDelay(configTICK_RATE_HZ / 10);
	}*/
	vTaskDelay(configTICK_RATE_HZ / 10);
	vTaskDelete(NULL);
}
