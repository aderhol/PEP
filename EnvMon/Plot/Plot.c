#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <float.h>

#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <common.h>

#include <Plot.h>

#include <Error.h>

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

static bool plotInit(void);
static FILE* gnuplotPipe;

#define graphMarginRel 0.05 //5%
static void* plot(void* args)
{
	while(!isStopRequested()){
		sleep(1);
		
		pthread_mutex_lock(&(tempSet.mutex));
		pthread_mutex_lock(&(concSet.mutex));
		if(!(tempSet.count > 0) || !(concSet.count > 0)){
			pthread_mutex_unlock(&(tempSet.mutex));
			pthread_mutex_unlock(&(concSet.mutex));
			continue;
		}
		pthread_mutex_unlock(&(tempSet.mutex));
		pthread_mutex_unlock(&(concSet.mutex));
		
		fprintf(gnuplotPipe, "plot '-' using 1:3 t \"\" with lines lt 1 axes x1y1, '-' using 1:3 t \"\" with lines lt 2 axes x1y2\n");
		
		pthread_mutex_lock(&(concSet.mutex));
		for(size_t i = 0; i < concSet.count; i++){
			datP dat;
			if(!getFromSet(&concSet, i, &dat)){
				sendToError("Fatal Error: Plot -> plot >> getFromSet");
				while(true);
			}
			fprintf(gnuplotPipe, "%s %f\n", dat.date, dat.dat);
		}
		pthread_mutex_unlock(&(concSet.mutex));
		fprintf(gnuplotPipe, "e\n");
		
		double tempMin = DBL_MAX, tempMax = DBL_MIN;
		pthread_mutex_lock(&(tempSet.mutex));
		for(size_t i = 0; i < tempSet.count; i++){
			datP dat;
			if(!getFromSet(&tempSet, i, &dat)){
				sendToError("Fatal Error: Plot -> plot >> getFromSet");
				while(true);
			}
			fprintf(gnuplotPipe, "%s %f\n", dat.date, dat.dat);
			tempMin = (dat.dat < tempMin) ? dat.dat : tempMin;
			tempMax = (dat.dat > tempMax) ? dat.dat : tempMax;
		}
		pthread_mutex_unlock(&(tempSet.mutex));
		fprintf(gnuplotPipe, "e\n");

		tempMin -= 0.5;
		tempMax += 0.5;
		fprintf(gnuplotPipe, "set y2range [%f : %f]\n", (((int)((tempMin - ((tempMax - tempMin) * graphMarginRel)) * 2)) / 2.0), (((int)((tempMax + ((tempMax - tempMin) * graphMarginRel)) * 2)) / 2.0));
		fprintf(gnuplotPipe, "set xrange [\"%s\" : \"%s\"]\n", tempSet.set[tempSet.first].date, tempSet.set[tempSet.count - 1].date);
		
		fflush(gnuplotPipe);
	}
	isInit = false;
	pclose(gnuplotPipe);
	pthread_mutex_destroy(&(tempSet.mutex));
	pthread_mutex_destroy(&(concSet.mutex));
	pthread_mutex_destroy(&time_mut);
	sendToError("Plot Out!");
	return NULL;
}

bool PlotInit(pthread_t* threads, int* count, int maxCount)
{
	*count = 0;
	if(maxCount < 1)
		return false;	
	
	pthread_mutex_init(&time_mut, NULL);
	pthread_mutex_init(&(tempSet.mutex), NULL);
	tempSet.first = 0;
	tempSet.count = 0;
	pthread_mutex_init(&(concSet.mutex), NULL);
	concSet.first = 0;
	concSet.count = 0;
	
	if(!plotInit())	
		return false;
	
	pthread_create(threads + (*count)++, NULL, plot, NULL);
	
	isInit = true;
	return true;
}

static bool plotInit(void)
{
	char * comm[] = {
			"set term wxt noraise font \"Helvetica,20\"",
			"set title \"Environment Monitor\" font \"Helvetica,45\" tc rgb \"0x0000CD\"",
			"set ytics tc lt 1",
			"set y2tics tc lt 2",
			"set xdata time",
			"set timefmt \"%Y-%m-%d %H:%M:%S\"",
			"set format x \"%H:%M:%S\"",
			"set grid",
			"set xlabel \"Date of Measurement\"",
			"set ylabel \"Concentration [ppm]\" tc lt 1",
			"set y2label \"Temperature [°C]\" tc lt 2",
			"set logscale y",
			"set offsets graph 0, 0, 0.05, 0.05"
	};
	
	if(NULL == (gnuplotPipe = popen("gnuplot 1> /dev/null 2> /dev/null", "w")))
			return false;

	for(int i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
		fprintf(gnuplotPipe, "%s\n", comm[i]);
	
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
	time_t numDateTime_s = (time_t)round(numDatTime_ms / 1000.0);
	localtime_r(&numDateTime_s, &localDatTime_s);
	if(strftime(datTime, maxStrLength, "%F %T", &localDatTime_s) == 0)
		return false;
	
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
	
	if(index >= set->count)
		return false;
	
	*dat = set->set[setIndex(*set, index)];
	return true;
}
