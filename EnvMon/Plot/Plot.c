#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pthread.h>
#include <time.h>

static void* plot(void* args);

typedef struct datP{
	double dat;
	char date[100];
	uint32_t dateN;
	struct datP* prv;
	struct datP* nxt;
}datP;
static datP* datAdd(datP* list, datP new);
static void deleteList(datP* listEl, bool fromBackToFront);

int main(void)
{
	datP d = {4.5, "2015", 555, NULL, NULL};
	datP* listhead = datAdd(NULL, d);
	d.dateN = 600;
	d.dat = 99.7;
	listhead = datAdd(NULL, d);
	return 0;
}

bool PlotInit(pthread_t* threads, int* count, int maxCount)
{
	
}

bool sendToPlot(char* dat)
{
	
}

static void* plot(void* args)
{
	
}

static datP* datAdd(datP* listEl, datP new){
	if(listEl == NULL){
		datP * const toBeRet = (datP*)malloc(sizeof(datP));
		toBeRet->prv = NULL;
		toBeRet->nxt = NULL;
		strcpy(toBeRet->date, new.date);
		toBeRet->dateN = new.dateN;
		toBeRet->dat = new.dat;
		return toBeRet;
	}
	
	while(listEl->prv != NULL)
		listEl = listEl->prv;
	
	while(listEl->dateN > new.dateN)
		listEl = listEl->nxt;
	
	datP * const up = listEl->prv;
	listEl->prv = (datP*)malloc(sizeof(datP));
	listEl->prv->nxt = listEl;
	listEl = listEl->prv;
	listEl->prv = up;
	listEl->dat = new.dat;
	strcpy(listEl->date, new.date);
	listEl->dateN = new.dateN;
	
	while(listEl->prv != NULL)
			listEl = listEl->prv;
	return listEl;
}

static void deleteList(datP* listEl, bool fromBackToFront)
{
	if(fromBackToFront){
		while(listEl->nxt != NULL)
				listEl = listEl->nxt;
		while(listEl != NULL){
			datP* prv = listEl->prv;
			free(listEl);
			listEl = prv;
		}
	}
	else
	{
		while(listEl->prv != NULL)
				listEl = listEl->prv;
		while(listEl != NULL){
			datP* nxt = listEl->nxt;
			free(listEl);
			listEl = nxt;
		}
	}
}
