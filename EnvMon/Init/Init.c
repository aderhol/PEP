#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>

#include <common.h>

#include <Init.h>
#include <Error.h>
#include <Serial.h>
#include <Plot.h>

bool Init(pthread_t* threads, int* count, int maxCount)
{
	int cnt = 0, ret_cnt, rem = maxCount;
	
	if(!ErrorInit(threads + cnt, &ret_cnt, rem)){
		printf("Error initError\n");
		cnt += ret_cnt;
		*count = cnt;
		return false;
	}
	cnt += ret_cnt;
	
	if(!Init_SerialPort(threads + cnt, &ret_cnt, rem)){
		printf("Serial initError\n");
		cnt += ret_cnt;
		*count = cnt;
		return false;
	}
	cnt += ret_cnt;
	

	*count = cnt;
	return true;
}

