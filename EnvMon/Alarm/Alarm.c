#include <Alarm.h>
#include <pthread.h>				//ehhez a foshoz kell library...
#include <string.h>
#include <stdlib.h> 				//atoi-hoz
#include <stdint.h>
#include "Error.h"
#include <common.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>     
#include <sys/stat.h>
#include <fcntl.h>

#define critical_conc 200
static pthread_mutex_t alarm_mut;
static bool AlarmFlag = false;	

static void* Alarm(void* param);


bool sendToAlarm(char const* mess)			//megnézi hogy van-e koncentráció adat és ha van akkor megvizsgálja a nagyságát
{
	char data[100];
	strcpy(data, mess);
	
	char* strtokState;
	
	char result[5][50];					//temp,temp_time,conc,conc_time 
	char *token;						//tokenizálás részeredménye ide kerül
	int i = 0;
	token = strtok_r(data, ",", &strtokState);
	
	while(token != NULL && i < 5)
	{
		strcpy(result[i], token);
		token = strtok_r(NULL, ",", &strtokState);		//belerakjuk a result tárolóba a tokeneket
		i++;
	}
	
	if(i == 5)
		return false;
	
	char* strtodSt;
	double conc = strtod(result[2], &strtodSt); //a második indexű elem (3. elem) a koncentráció
		if('\0' != *strtodSt)
			return false;		
		
	if(!isnan(conc))						
	{
		if(conc > critical_conc)
		{
			pthread_mutex_lock(&alarm_mut);	//kritikus koncentráció esetén AlarmFlag = true vagyis riasztunk
			AlarmFlag = true;
			pthread_mutex_unlock(&alarm_mut);
		}
		else
		{
			pthread_mutex_lock(&alarm_mut);
			AlarmFlag = false;
			pthread_mutex_unlock(&alarm_mut);
		}
	}
	return true;
}

static void* Alarm(void* param)
{
	int PID = 0;
	
	while(!isStopRequested()){
		bool al;
		pthread_mutex_lock(&alarm_mut);
		al = AlarmFlag;
		pthread_mutex_unlock(&alarm_mut);
		
		if(al){
			if(PID == 0){
				switch(PID = fork()){
				case 0:{
					
					//redirect stdout and stderr
					int abyss = open("/dev/null", O_WRONLY);
				    dup2(abyss, 1);
				    dup2(abyss, 2);
				    close(abyss);
				    
					execlp("speaker-test", "speaker-test", "-t", "wav", "-W", ".", "-w", "alarm.wav", "-l", "0", (char*) NULL);
				}
					break;
				default:
					break;
				}				
			}
		}
		else{
			if(PID != 0){
				kill(PID, 9);
				PID = 0;
			}
		}
		usleep(1000 * 100);
	}
	
	if(PID != 0)
		kill(PID, 9);
	
	sendToError("Alarm Out!");
	return NULL;
}

bool AlarmInit(pthread_t* threads, int* count, int countMax)
{	
	if(pthread_mutex_init(&alarm_mut, NULL) != 0)
	{
		sendToError("Error initializing mutex!");
		*count = 0;
		return false;
	}
	
	if(countMax < 1)
	{
		*count = 0;
		return false;
	}
	pthread_t al;
	if(pthread_create(&al, NULL, Alarm,(void*)NULL) != 0)
	{
		sendToError("Error creating Alarm thread!");
		*count = 0;
		return false;
	}
	*count = 1;
	threads[0] = al;
	return true;
}
