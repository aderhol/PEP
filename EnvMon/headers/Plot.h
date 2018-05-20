#ifndef HEADERS_PLOT_H_
#define HEADERS_PLOT_H_

#include <stdbool.h>

bool PlotInit(pthread_t* threads, int* count, int maxCount);
bool sendToPlot(char const* dat);


#endif /* HEADERS_PLOT_H_ */
