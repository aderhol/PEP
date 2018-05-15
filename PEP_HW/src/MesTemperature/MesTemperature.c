#include <stdbool.h>
#include <stdint.h>

#include <em_i2c.h>
#include "em_gpio.h"
#include "em_cmu.h"

#include <MesTemperature.h>
#include "MesTemperature_int.h"

#include "FreeRTOS.h"
#include "queue.h"
#include <timers.h>
#include <semphr.h>
#include "task.h"
#include "common.h"     
#include "SystemClock.h"


#define I2C_ADDRESS 0x90  //0x48<<1 1001000x
#define I2C_RXBUFFER_SIZE 2 //2 bájtot olvasunk ( 16bites a hõmérséklet regiszter )

static QueueHandle_t tempQueue;
static SemaphoreHandle_t tempSemaphore;

void MesTemperatureInit(void){
    Init_I2C();	//I2C inicializálás
    tempQueue = xQueueCreate(10, sizeof(Temperature));
    tempSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(MesTemperature, "MesTemperature_MesTemperature_task", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);       //task létrehozás
    xTimerStart(xTimerCreate("temperature_timer", configTICK_RATE_HZ, pdTRUE, NULL, Timer), 0);
}

bool MesTemperatureGet_temperature(Temperature *temp){
	
    if(pdTRUE == xQueueReceive(tempQueue, temp, 0))
    {    	
    	return true;
    }
    else
    {
    	return false;
    }
}

static void Init_I2C(void)
{
	//CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_I2C1, true);
	I2C1->ROUTE |= I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN;
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;                       //default inicializáló struktúra
	/*
	true,                    enable
	true,                    Set to master mode
	0,                       Use currently configured reference clock
	I2C_FREQ_STANDARD_MAX,   Set to standard rate assuring being
				   within I2C spec
	i2cClockHLRStandard      Set to use 4:4 low/high duty cycle
	*/
	
	/*PC4 (SDA) PC5 (SCL) */
	
	GPIO_PinModeSet(gpioPortC, 5, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(gpioPortC, 4, gpioModeWiredAndPullUpFilter, 1);
	I2C_Init(I2C1, &i2cInit);                                         //I2C inicializálás
}

static void I2C_Receive( uint8_t* temperature, uint16_t len)    //temperature változóba kerül az eredmény
{
	 I2C_TransferSeq_TypeDef transfer;                  //átviteli struktúra
	 transfer.addr =  I2C_ADDRESS;		                //erre a címre írunk AAAAAAAx alakú
	 transfer.flags = I2C_FLAG_READ;                    //S+ADDR(R)+DATA0+P buffer0-ba jön az adat
	 transfer.buf[0].len = len;                         // ennyi byte-ot olvasunk
	 transfer.buf[0].data = temperature;                //ennyi byte-ot olvasunk


	 I2C_TransferReturn_TypeDef ret;	                // olvasás visszatérési értéke (pl: még tart)

	 ret = I2C_TransferInit(I2C1, &transfer);
	 while (ret == i2cTransferInProgress)               //amíg folyamatban az olvasás
	 {
	   ret = I2C_Transfer(I2C1);			            //olvasás
	 }

}

static void MesTemperature(void *pvParam){                  //task ami méri a hõmérsékletet
                                                     //16 bit-es a hõmérséklet regiszer
	uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE];     //I2C buffer
	uint32_t time;                               //idõ változó
	Temperature temp;
	
	while(1)
	{
		xSemaphoreTake(tempSemaphore, portMAX_DELAY);		
		
		I2C_Receive(i2c_rxBuffer, I2C_RXBUFFER_SIZE);     //I2C olvasása
		if(SystemClockGet_ms(&time))                     //idõ lekérdezése ( nem fog kelleni )
		{
			temp.temperature = (int8_t)i2c_rxBuffer[0] + ((i2c_rxBuffer[1] >> 7) / 2.0);
			temp.time = time;
			xQueueSendToBack(tempQueue, (void *) &temp, 0);
		}
		else
		{
			while(true);
		}
	}
}

static void Timer(TimerHandle_t xTimer)
{
	xSemaphoreGive(tempSemaphore);
}


