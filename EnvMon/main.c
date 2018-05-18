#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>     
#include <sys/stat.h>
#include <fcntl.h>



#include <errno.h>

#include <pthread.h>

#include <common.h>

#include <Init.h>

static bool isStopReq = false;
static pthread_mutex_t isStopReq_mut_;
static pthread_mutex_t* isStopReq_mut = &isStopReq_mut_;
int main(void)
{
	printf("Initializing...\n");
	
	if (pthread_mutex_init(isStopReq_mut, NULL) != 0)
    {
        fprintf(stderr, "\n mutex init failed\n");
    	printf("Exited.\n");
        return 1;
    }
	
	
	pthread_t threads[100];
	int threadCount;
	
	if(false == Init(threads, &threadCount, sizeof(threads) / sizeof(pthread_t))){
		printf("An error has occurred during initialization!\n");
	}
	else{
		printf("Initialized.\n");
		printf("Press ENTER to exit!\n");
				
		while('\n' != getchar());
	}
	
	pthread_mutex_lock(isStopReq_mut);
	isStopReq = true;
	pthread_mutex_unlock(isStopReq_mut);
	
	for(int i = 0; i < threadCount; i++){
		pthread_join(threads[i], NULL);
	}
	
	pthread_mutex_destroy(isStopReq_mut);
	
	printf("Exited.\n");
	//while('\n' != getchar());
	return 0;
}

bool isStopRequested()
{
	pthread_mutex_lock(isStopReq_mut);
	bool is = isStopReq;
	pthread_mutex_unlock(isStopReq_mut);
	
	return is;
}
