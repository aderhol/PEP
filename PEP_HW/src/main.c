#include "em_chip.h"

#include <FreeRTOS.h>
#include <task.h>

#include <common.h>

#include <Init.h>

#include <stdio.h>

int main(void)
{
	/* Chip errata */
	CHIP_Init();
	Init();	//initializes the system

	vTaskStartScheduler();	//starts the OS

	//only gets here, if the system is out of RAM
	while(1)
	{
		//out of RAM fault
	}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
	printf("%s", pcTaskName);
}
