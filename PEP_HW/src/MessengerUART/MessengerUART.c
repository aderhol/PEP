#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>



#include <retargetserial.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>

#include <MessengerUART.h>
#include "MessengerUart_int.h"

#include <common.h>

#include <CommandFromUART.h>
#include <MesConcentration.h>
#include <MesTemperature.h>

static SemaphoreHandle_t messengerSem;

void MessengerUARTInit(void)
{
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(1);
	messengerSem = xSemaphoreCreateBinary();
	xTaskCreate(messenger, "MessengerUART_messenger_task", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);
	//xTimerStart(xTimerCreate("MessengerUART_messenger_timer", configTICK_RATE_HZ / 100, pdTRUE, NULL, messenger_timer), 0);
}

static void messenger(void* pvParam)
{
	char message[101];

	while(true){
		//xSemaphoreTake(messengerSem, portMAX_DELAY);	//wait for signal to run
		while(CommandFromUARTGet_reply(message, 100)){
			while(*message == '\0'); //spin if the message was to long
			printf("%s", message);
		}

		bool isConc, isTemp;
		Temperature temperature;
		Concentration concentration;
		isConc = MesConcentrationGet_concentration(&concentration);
		isTemp = MesTemperatureGet_temperature(&temperature);
		while(isConc || isTemp){
			if(isConc && isTemp)
				printf("$DAT,%.2f,%lu,%.2f,%lu\r\n", temperature.temperature, temperature.time, concentration.concentration, concentration.time);
			else if(isTemp)
				printf("$DAT,%.2f,%lu,,\r\n", temperature.temperature, temperature.time);
			else
				printf("$DAT,,,%.2f,%lu\r\n", concentration.concentration, concentration.time);
			
			isConc = MesConcentrationGet_concentration(&concentration);
			isTemp = MesTemperatureGet_temperature(&temperature);
		}
		vTaskDelay(configTICK_RATE_HZ / 100);
	}
}

static void messenger_timer(TimerHandle_t xTimer)
{
	xSemaphoreGive(messengerSem);
}

bool __attribute__((weak)) MesConcentrationGet_concentration(Concentration* con)
{
	static uint32_t time = 0;
	static int count = 0;
	time++;
	count = (count + 1) % 20;
	if(count != 0)
		return false;
	con->concentration = 123.456;
	con->time = time;
	return true;
}

bool __attribute__((weak)) MesTemperatureGet_temperature(Temperature* temp)
{
	static uint32_t time = 0;
	static int count = 0;
	count = (count + 1) % 10;
	time++;
	if(count != 0)
		return false;
	temp->temperature = 12.345;
	temp->time = time;
	return true;
}

bool __attribute__((weak)) CommandFromUARTGet_reply(char* ms, int_fast16_t maxLength)
{
	static int count = 0;
	count = (count + 1) % 100;
	if(count != 0)
		return false;
	strcpy(ms, "$ALIVE\r\n");
	return true;
}

