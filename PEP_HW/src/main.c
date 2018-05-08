#include <FreeRTOS.h>
#include <task.h>

#include <Init.h>


void main(void)
{
	Init();	//initializes the system

	vTaskStartScheduler();	//starts the OS

	//only gets here, if the system is out of RAM
	while(1)
	{
		//out of RAM fault
	}
}
