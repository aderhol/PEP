#include <MessengerUART.h>

#include <common.h>

// Application specific includes
#include <stdio.h>
#include <retargetserial.h>

// FreeRTOS includes

#include "FreeRTOS.h"

#include "task.h"

static void prvTaskHello(void *pvParam);

void MessengerUARTInit(void)
{
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(1);
	xTaskCreate(prvTaskHello, "", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);
}

static void prvTaskHello(void *pvParam)
{
	while(true)	{
		printf("Hello FreeRTOS! :)\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
