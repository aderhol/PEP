#include <Error.h>


#include <stdbool.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <common.h>

static void* error(void*);

static bool isInit = false;
static pthread_mutex_t init_mut, buff_mut;


#define strLen 1000
#define buffLen 100
static char buffers[2][buffLen][strLen];
static int buffIndex = 0;
static int buffCnt[2];

bool ErrorInit(pthread_t* threads, int* count, int maxCount)
{
	if(maxCount < 1){
		*count = 0;
		return false;
	}
	if(0 != pthread_mutex_init(&init_mut, NULL)){
		*count = 0;
		return false;
	}
	if(0 != pthread_mutex_init(&buff_mut, NULL)){
		*count = 0;
		return false;
	}
	if(0 != pthread_create(threads + 0, NULL, error, NULL)){
		*count = 0;
		return false;
	}
	*count = 1;
	pthread_mutex_lock(&init_mut);
	isInit = true;
	pthread_mutex_unlock(&init_mut);
	return true;
}

static void* error(void* args)
{
	struct timespec delay = {0, 1000*1000};
	while(!isStopRequested()){
		pthread_mutex_lock(&buff_mut);
		int index = buffIndex;
		buffIndex = (buffIndex == 0) ? 1 : 0;
		pthread_mutex_unlock(&buff_mut);
		
		for(int i = 0; i < buffCnt[index]; i++){
			fprintf(stderr, "%s\n", buffers[index][i]);
		}
		buffCnt[index] = 0;		
		
		nanosleep(&delay, NULL);
	}
	sleep(1);	

	pthread_mutex_lock(&init_mut);
	isInit = false;
	pthread_mutex_unlock(&init_mut);
	
	pthread_mutex_lock(&buff_mut);
	
	for(int i = 0; i < buffCnt[0]; i++)
		fprintf(stderr, "%s\n", buffers[0][i]);
	
	for(int i = 0; i < buffCnt[1]; i++)
		fprintf(stderr, "%s\n", buffers[1][i]);
	
	pthread_mutex_destroy(&buff_mut);
	fprintf(stderr,"Err Out!\n");
	fflush(stderr);
	struct timespec delay2 = {0, 1000*1000};
	nanosleep(&delay2, NULL);
	return NULL;
}

bool sendToError(char* e)
{
	pthread_mutex_lock(&init_mut);
	bool initS = isInit;
	pthread_mutex_unlock(&init_mut);
	if(!initS)
		return false;
	if(strlen(e) + 1 > strLen)
		return false;
	pthread_mutex_lock(&buff_mut);
	if(buffCnt[buffIndex] < buffLen){
		strcpy(buffers[buffIndex][buffCnt[buffIndex]++], e);
		pthread_mutex_unlock(&buff_mut);
		return true;
	}else
	{
		pthread_mutex_unlock(&buff_mut);
		return false;
	}
}
