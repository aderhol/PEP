#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include <pthread.h>
#include <time.h>

#include <Plot.h>

static void* plot(void* args);

typedef struct datP{
	double dat;
	char date[101];
}datP;

#define DataSetLength (2000)
typedef struct DataSet{
	datP set[DataSetLength];
	size_t first;
	size_t count;
	pthread_mutex_t mutex;
}DataSet;

static DataSet tempSet, concSet;

static bool isInit = false;

static pthread_mutex_t time_mut;
static bool getTime(const int64_t datTime_ms, char* datTime, size_t maxStrLength);
static void addToSet(DataSet* set, datP dat);
static bool getFromSet(DataSet* set, size_t index, datP* dat);
#define setIndex(set, ind)	(((set).first + (ind)) % DataSetLength)

bool PlotInit(pthread_t* threads, int* count, int maxCount)
{
	pthread_mutex_init(&time_mut, NULL);
	pthread_mutex_init(&(tempSet.mutex), NULL);
	tempSet.first = 0;
	tempSet.count = 0;
	pthread_mutex_init(&(concSet.mutex), NULL);
	concSet.first = 0;
	concSet.count = 0;
	
	
	
	isInit = true;
	return true;
}

bool sendToPlot(char const* dat)
{
	if(!isInit)
		return false;
	
	if(strlen(dat) > 100)
		return false;
	
	char datC[101];
	strcpy(datC, dat);
	
	char* tokens[4];
	char* strtokState;
	for(int i = 0; i < 4; i++){
		tokens[i] = strtok_r(((i == 0) ? datC : NULL), ",", &strtokState);
		if(tokens[i] == NULL)
			return false;
	}
	if(strtok_r(NULL, "", &strtokState) != NULL)
		return false;
	
	char* strtodSt;
	double temp = strtod(tokens[0], &strtodSt);
	if('\0' != *strtodSt)
		return false;
	double conc = strtod(tokens[2], &strtodSt);
	if('\0' != *strtodSt)
		return false;
	
	if(!isnan(temp)){
		char* strtolSt;
		int64_t time = strtol(tokens[1], &strtolSt, 10);
		if('\0' != *strtolSt)
			return false;
		datP dat;
		dat.dat = temp;
		if(!getTime(time, dat.date, (sizeof(dat.date) / sizeof(dat.date[0]))))
			return false;
		addToSet(&tempSet, dat);
	}
	
	if(!isnan(conc)){
		char* strtolSt;
		int64_t time = strtol(tokens[3], &strtolSt, 10);
		if('\0' != *strtolSt)
			return false;
		datP dat;
		dat.dat = conc;
		if(!getTime(time, dat.date, (sizeof(dat.date) / sizeof(dat.date[0]))))
			return false;
		addToSet(&concSet, dat);
	}

	return true;
}

static void* plot(void* args)
{
	
	return NULL;
}

static bool getTime(const int64_t datTime_ms, char* datTime, size_t maxStrLength)
{
	if(!isInit)
		return false;
	
	static int64_t datTime_offset_ms = -1;
	static uint64_t refTime_ms;
	
	pthread_mutex_lock(&time_mut);
	if(datTime_offset_ms == -1){
		struct timespec currTime;
		clock_gettime(CLOCK_REALTIME, &currTime);
		refTime_ms = currTime.tv_sec * 1000 + (uint64_t)round(currTime.tv_nsec / (1000 * 1000.0));
		datTime_offset_ms = datTime_ms;
	}
	uint64_t numDatTime_ms = refTime_ms  + (datTime_ms - datTime_offset_ms);
	pthread_mutex_unlock(&time_mut);
	

	struct tm localDatTime_s;
	time_t numDateTime_s = numDatTime_ms / 1000;
	localtime_r(&numDateTime_s, &localDatTime_s);
	size_t timeOutLen = strftime(datTime, maxStrLength, "%F %T.", &localDatTime_s);
	if(maxStrLength - timeOutLen < 3 || timeOutLen == 0){
		return false;
	}
	char ms[10];
	sprintf(ms, "%.3d", (int)(numDatTime_ms % 1000));
	strcat(datTime, ms);
	return true;
}

static void addToSet(DataSet* set, datP dat)
{
	if(!isInit)
		return;

	pthread_mutex_lock(&(set->mutex));
		
	set->set[setIndex(*set, set->count)] = dat;
	
	if(set->count == DataSetLength){
		set->first = setIndex(*set, (set->count + 1));
	}
	else
		set->count++;
	
	pthread_mutex_unlock(&(set->mutex));		
}

static bool getFromSet(DataSet* set, size_t index, datP* dat)
{
	if(!isInit)
		return false;
	
	pthread_mutex_lock(&(set->mutex));
	
	if(index >= set->count)
		return false;
	
	*dat = set->set[setIndex(*set, index)];
	pthread_mutex_unlock(&(set->mutex));
	return true;
}
