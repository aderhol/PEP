#ifndef HEADERS_ERROR_H_
#define HEADERS_ERROR_H_

#include <stdbool.h>
#include <pthread.h>

bool ErrorInit(pthread_t* threads, int* count, int maxCount);
bool sendToError(char* e);

#endif /* HEADERS_ERROR_H_ */
