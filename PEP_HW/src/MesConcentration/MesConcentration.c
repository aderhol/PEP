
#include "MesConcentration.h"
#include "MesConcentration_int.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "common.h"     // ezt elvileg te csinálod meg
#include "SystemClock.h"

#include <timers.h>
#include <semphr.h>

#include "em_gpio.h"
#include "em_cmu.h"
#include "em_adc.h"
#include <math.h>


//BASE_PRIORITY (common-ban)
//configMINIMAL_STACK_SIZE (saját stack méret)

/*....define-ok amiket a karakterisztika illesztõje csinált...*/
#define MQ135_DEFAULTPPM (392)                //alap ppm CO2 kalibráláshoz
#define MQ135_DEFAULTRO (41763)               //alap ellenállás R0 az alap CO2 koncentrációhoz
#define MQ135_SCALINGFACTOR (116.6020682)     //függvény szorzótényezõ
#define MQ135_EXPONENT (-2.769034857)         //függvény hatványa
#define MQ135_MAXRSRO (2.428)                 //max R0
#define MQ135_MINRSRO (0.358)                 //min R0
/*............................................................*/

static QueueHandle_t concQueue;
static SemaphoreHandle_t concSemaphore;


 void MesConcentrationInit(void){
	
	xTimerStart(xTimerCreate("concentration_timer", configTICK_RATE_HZ, pdTRUE, NULL, Timer), 0);
	concQueue = xQueueCreate(10, sizeof(Concentration));
	concSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(MesConcentration,"",configMINIMAL_STACK_SIZE,NULL, BASE_PRIORITY,NULL);       //task létrehozás
    Init_ADC();                                                                               //ADC inicializálása
}

 bool MesConcentrationGet_concentration(Concentration *conc){
	if(pdTRUE == xQueueReceive(concQueue, conc, 0))
	    {    	
	    	return true;
	    }
	    else
	    {
	    	return false;
	    }
}

static void MesConcentration(void *pvParam){
   Concentration conc;
   uint32_t time;

    while(1){
    	
    		xSemaphoreTake(concSemaphore, portMAX_DELAY);	
    		
			if(SystemClockGet_ms(&time))                        //idõ lekérdezése ( nem fog kelleni az idõ )
			{
				 conc.time = time;                           //berakjuk a conc változóba a megfelelõ értékeket
				 conc.concentration = PPM_Of_CO2();
				 while(errQUEUE_FULL == xQueueSendToBack(concQueue, (void *) &conc, 0));
			}
			else
			{        
				  while(1);
			}
    }
}


static void Init_ADC()
{
	CMU_ClockEnable(cmuClock_ADC0,true);						//ADC0 engedélyezése
	GPIO_PinModeSet(gpioPortD, 0, gpioModeInput,0);
	ADC_Init_TypeDef   init  = ADC_INIT_DEFAULT; 				//ADC inicializáló struktúra default
	ADC_InitSingle_TypeDef  MyAdc = ADC_INITSINGLE_DEFAULT; 	//ADC_single inicializáló struktúra
																//default csatorna a PD0 (CH0) ADC0
	MyAdc.reference=adcRefVDD; 									//a default struktúrában 1.2v a referencia. Így 3.3v-lesz.
	/*
	{
	  adcPRSSELCh0,              // PRS ch0 (if enabled).
	  adcAcqTime1,               //1 ADC_CLK cycle acquisition time.
	  adcRef1V25,                //1.25V internal reference.
	  adcRes12Bit,               // 12 bit resolution.
	  adcSingleInpCh0,           // CH0 input selected.
	  false,                     // Single ended input.
	  false,                     // PRS disabled.
	  false,                     // Right adjust.
	  false                      // Deactivate conversion after one scan sequence.
	}
	 */

	ADC_Init(ADC0, &init); 									     //ADC0-ra inicializálunk
	ADC_InitSingle(ADC0, &MyAdc);								 //ADC_single inicializálás
}

static void ADC_Single_Conv(uint32_t *result)
{
	ADC_Start(ADC0,adcStartSingle );					         //konverzió kezdete
	while(!(ADC0->STATUS & (1 << 16))); 				         //varunk még befejezõdik a konverzió
	*result = ADC_DataSingleGet(ADC0); 					         // eredmény írása
}

static float PPM_Of_CO2()
{
	uint32_t adcResult;                                          //ADC eredménye max 4096
	double LSB = 3.3/4096;                                       //ADC karakterisztikának a meredeksége
	double adcVoltage;                                           //a mért feszültség
	double resvalue;                                             //a függvény által számított érték
	double ro=MQ135_DEFAULTRO;                                   //lásd define
	ADC_Single_Conv(&adcResult);
	adcVoltage = LSB*(double)adcResult;
	resvalue = 1000*((5/adcVoltage)-1);
	float ret = -1;
	float validinterval = 0;
		validinterval = resvalue / ro;                    //R0 ellenörzése, hogy megfelelõ határok között mozog-e
		/*if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
			ret = (double)(MQ135_SCALINGFACTOR) * (float)pow(validinterval, MQ135_EXPONENT);
		}*/
		return (double)(MQ135_SCALINGFACTOR) * (float)pow(validinterval, MQ135_EXPONENT);
}

static void Timer(TimerHandle_t xTimer)
{
	xSemaphoreGive(concSemaphore);
}

