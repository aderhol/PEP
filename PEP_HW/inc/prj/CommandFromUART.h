#ifndef COMMANDFROMUART_H
#define COMMANDFROMUART_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	INIT,
	UNCONNECTED,
	CONNECTED,
	UNKNOWN
}Status;

void CommandFromUARTInit(void);
bool CommandFromUARTGet_status(Status* status);
bool CommandFromUARTGet_reply(char* reply, int_fast16_t maxLength);	//sets the reply to "\0" and returns with false, if maxLength is too small (maxLength is with the terminating \0)

#endif
