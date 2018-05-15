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
#define I2C_RXBUFFER_SIZE 2 //2 b�jtot olvasunk ( 16bites a h�m�rs�klet regiszter )

static QueueHandle_t tempQueue;
static SemaphoreHandle_t tempSemaphore;

void MesTemperatureInit(void){
    Init_I2C();	//I2C inicializ�l�s
    tempQueue = xQueueCreate(10, sizeof(Temperature));
    tempSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(MesTemperature, "MesTemperature_MesTemperature_task", configMINIMAL_STACK_SIZE, NULL, BASE_PRIORITY, NULL);       //task l�trehoz�s
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
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;                       //default inicializ�l� strukt�ra
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
	I2C_Init(I2C1, &i2cInit);                                         //I2C inicializ�l�s
}

static void I2C_Receive( uint8_t* temperature, uint16_t len)    //temperature v�ltoz�ba ker�l az eredm�ny
{
	 I2C_TransferSeq_TypeDef transfer;                  //�tviteli strukt�ra
	 transfer.addr =  I2C_ADDRESS;		                //erre a c�mre �runk AAAAAAAx alak�
	 transfer.flags = I2C_FLAG_READ;                    //S+ADDR(R)+DATA0+P buffer0-ba j�n az adat
	 transfer.buf[0].len = len;                         // ennyi byte-ot olvasunk
	 transfer.buf[0].data = temperature;                //ennyi byte-ot olvasunk


	 I2C_TransferReturn_TypeDef ret;	                // olvas�s visszat�r�si �rt�ke (pl: m�g tart)

	 ret = I2C_TransferInit(I2C1, &transfer);
	 while (ret == i2cTransferInProgress)               //am�g folyamatban az olvas�s
	 {
	   ret = I2C_Transfer(I2C1);			            //olvas�s
	 }

}

static void MesTemperature(void *pvParam){                  //task ami m�ri a h�m�rs�kletet
                                                     //16 bit-es a h�m�rs�klet regiszer
	uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE];     //I2C buffer
	uint32_t time;                               //id� v�ltoz�
	Temperature temp;
	
	while(1)
	{
		xSemaphoreTake(tempSemaphore, portMAX_DELAY);		
		
		I2C_Receive(i2c_rxBuffer, I2C_RXBUFFER_SIZE);     //I2C olvas�sa
		if(SystemClockGet_ms(&time))                     //id� lek�rdez�se ( nem fog kelleni )
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


