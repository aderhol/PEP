#ifndef INIT_INIT_H_
#define INIT_INIT_H_

#include <stdbool.h>

#include <pthread.h>

bool Init(pthread_t* threads, int* count, int maxCount);

#endif /* INIT_INIT_H_ */
