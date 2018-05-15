#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <em_core.h>

#include <retargetserial.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include <queue.h>

#include <CommandFromUART.h>
#include "CommandFromUART_int.h"

#include <common.h>

static SemaphoreHandle_t readSem;

static QueueHandle_t replyQueue;

static TimerHandle_t connection_timer_handle;
static SemaphoreHandle_t conSem;
static volatile Status status = INIT;
static volatile int sentIDs[3] = {0};
static SemaphoreHandle_t sentIDs_mutex;

#define maxReplyLength	(100 + 1)

void CommandFromUARTInit(void)
{
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(1);
	readSem = xSemaphoreCreateBinary();
	xTaskCreate(read, "CommandFromUART_read_task", configMINIMAL_STACK_SIZE * 2, NULL, BASE_PRIORITY, NULL);
	xTaskCreate(connection_pinger, "CommandFromUART_pinger_task", configMINIMAL_STACK_SIZE * 2, NULL, BASE_PRIORITY, NULL);
	conSem = xSemaphoreCreateBinary();
	sentIDs_mutex = xSemaphoreCreateMutex();
	xTimerStart(xTimerCreate("CommandFromUART_read_timer", configTICK_RATE_HZ / 100, pdTRUE, NULL, read_timer), 0);
	xTimerStart(connection_timer_handle = xTimerCreate("CommandFromUART_connection_timer", configTICK_RATE_HZ, pdTRUE, NULL, connection_timer), 0);
	xTimerStart(xTimerCreate("CommandFromUART_pinger_timer", configTICK_RATE_HZ / 10, pdTRUE, NULL, connection_pinger_timer), 0);
	replyQueue = xQueueCreate(10, sizeof(char) * maxReplyLength);
}

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define errorTooLong_header ("$ERROR,receive,Too-Long,")
#define errorFormat_header ("$ERROR,receive,Format,")
#define errorEmpty_header ("$ERROR,receive,Empty")
#define errorNumber_header ("$ERROR,receive,NumberOfTokens,")
#define errorUsage_header ("$ERROR,receive,Usage,")
#define errorInvalid_header ("$ERROR,receive,Invalid,")
#define errorAlive_header ("$ERROR,receive,Alive,")
#define errorTooLong_length (sizeof(errorTooLong_header) / sizeof(char) - 1)
#define errorFormat_length (sizeof(errorFormat_header) / sizeof(char) - 1)
#define errorEmpty_length (sizeof(errorEmpty_header) / sizeof(char) - 1)
#define errorNumber_length (sizeof(errorNumber_header) / sizeof(char) - 1)
#define errorUsage_length (sizeof(errorUsage_header) / sizeof(char) - 1)
#define errorInvalid_length (sizeof(errorInvalid_header) / sizeof(char) - 1)
#define errorAlive_length (sizeof(errorAlive_header) / sizeof(char) - 1)
#define error_header_maxLength (MAX(errorAlive_length, MAX(errorInvalid_length, MAX(errorUsage_length, MAX(errorNumber_length, MAX(errorEmpty_length ,MAX(errorTooLong_length, errorFormat_length)))))))
#define maxInputLength (maxReplyLength - (1 + 1) - error_header_maxLength)

#define maxNumberOfTokens	15

static void read(void* pvParam)
{
	int count = 0;
	char input[maxInputLength];
	while(true){
		xSemaphoreTake(readSem, portMAX_DELAY);	//wait for signal to run
		int chGot;
		while((chGot = getchar()) != EOF){
			char ch = (char)chGot;
			input[count++] = ch;			
			if('\n' == ch){
				if(input[count - 2] == '\r')
					count--;
				input[count - 1] = '\0';
				
				if(input[0] != '$'){	//not a proper message
					input[count - 1] = '\n';
					input[count++] = '\0';
					char reply[maxReplyLength];
					strcpy(reply, errorFormat_header);
					strcpy(reply + errorFormat_length, input);
					while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full					
				}
				else{
					int tkCount;
					char* tokens [maxNumberOfTokens + 1];
					char input_copy[maxInputLength];
					strcpy(input_copy, input);
					for(tkCount = 0; (maxNumberOfTokens >= tkCount) && (NULL != (tokens[tkCount] = strtok((tkCount == 0) ? (input_copy + 1) : NULL, ","))); tkCount++)
						for(int i = 0; '\0' != (tokens[tkCount][i] = (char)tolower((int)(tokens[tkCount][i]))); i++);	
					if(maxNumberOfTokens < tkCount){ //too many tokens
						input[count - 1] = '\n';
						input[count++] = '\0';
						char reply[maxReplyLength];
						strcpy(reply, errorNumber_header);
						strcpy(reply + errorNumber_length, input);
						while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
					}
					else{						
						if(tkCount == 0){ //0 tokens
							char reply[maxReplyLength];
							strcpy(reply, errorEmpty_header);
							reply[errorEmpty_length] = '\n';
							reply[errorEmpty_length + 1] = '\0';
							while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
						}
						else{
							if(strcmp(tokens[0], "alive") == 0){
								if(2 != tkCount){
									input[count - 1] = '\n';
									input[count++] = '\0';
									char reply[maxReplyLength];
									strcpy(reply, errorUsage_header);
									strcpy(reply + errorUsage_length, input);
									while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
								}
								else{
									char* end;
									int ID = (int)strtol(tokens[1], &end, 10);
									if(0 == ID || '\0' != *end){
										input[count - 1] = '\n';
										input[count++] = '\0';
										char reply[maxReplyLength];
										strcpy(reply, errorUsage_header);
										strcpy(reply + errorUsage_length, input);
										while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
									}
									else{
										if(!connectionResp(ID))
										{
											input[count - 1] = '\n';
											input[count++] = '\0';
											char reply[maxReplyLength];
											strcpy(reply, errorAlive_header);
											strcpy(reply + errorAlive_length, input);
											while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full								
										}
									}
								}
							}
							else if(strcmp(tokens[0], "ping") == 0){
								if(2 != tkCount){
									input[count - 1] = '\n';
									input[count++] = '\0';
									char reply[maxReplyLength];
									strcpy(reply, errorUsage_header);
									strcpy(reply + errorUsage_length, input);
									while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
								}
								else{
									char* end;
									int ID = (int)strtol(tokens[1], &end, 10);
									if(0 == ID || '\0' != *end){
										input[count - 1] = '\n';
										input[count++] = '\0';
										char reply[maxReplyLength];
										strcpy(reply, errorUsage_header);
										strcpy(reply + errorUsage_length, input);
										while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
									}
									else{
										char reply[maxReplyLength];
										sprintf(reply, "$ALIVE,%d\n", connectionReq(ID));
										while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full				
									}
								}
							}
							else{
								input[count - 1] = '\n';
								input[count++] = '\0';
								char reply[maxReplyLength];
								strcpy(reply, errorInvalid_header);
								strcpy(reply + errorInvalid_length, input);
								while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
							}
						}
					}
				}
				
				count = 0; //reset the buffer
			}
			else if(count == maxInputLength){
				input[count++] = '\n';
				input[count++] = '\0';
				char reply[maxReplyLength];
				strcpy(reply, errorTooLong_header);
				strcpy(reply + errorTooLong_length, input);
				while(errQUEUE_FULL == xQueueSendToBack(replyQueue, reply, 0));	//spins if the queue is full
				count = 0;
			}
		}
	}
}

static void read_timer(TimerHandle_t xTimer)
{
	xSemaphoreGive(readSem);
}

bool CommandFromUARTGet_reply(char* reply, int_fast16_t maxLength)
{
	char rp[maxReplyLength];
	if(pdTRUE == xQueueReceive(replyQueue, rp, 0)){
		if(strlen(rp) + 1 > maxLength){
			*reply = '\0'; //indicates that the receiving side can't receive the message, because it is too long
			return false;
		}
		strcpy(reply, rp); //copies the reply to the receiving string
		return true;
	}
	else{
		*reply = '\0' + 1;	//ensures that the reply is not an empty string, which would indicate that the message was too long for the receiving side
		return false;
	}
}


static bool connectionResp(int ID)
{
	BaseType_t res = xTimerReset(connection_timer_handle, 0);
	while(res != pdPASS );
	ID++;
	int i;
	xSemaphoreTake(sentIDs_mutex, portMAX_DELAY);
	for(i = 0; i < (sizeof(sentIDs) / sizeof(int)); i++)
		if(sentIDs[i] == ID)
			break;
	xSemaphoreGive(sentIDs_mutex);
	if(i == (sizeof(sentIDs) / sizeof(int)))
		return false;
	
	CORE_CRITICAL_SECTION(
			status = CONNECTED;
	);			
	return true;
}

static int connectionReq(int ID)
{
	return ID - 1;
}

bool CommandFromUARTGet_status(Status* st)
{
	CORE_CRITICAL_SECTION(
			*st = status;
	);	
	return true;
}

static void connection_timer(TimerHandle_t xTimer)
{
	if(global_IsInit){
		CORE_CRITICAL_SECTION(
				status = UNCONNECTED;
		);			
	}
	else{
		CORE_CRITICAL_SECTION(
				status = INIT;
		);			
	}
}

static void connection_pinger_timer(TimerHandle_t xTimer)
{
	xSemaphoreGive(conSem);
}

static void connection_pinger(void* pvParam)
{
	int ID;
	while(true){
		xSemaphoreTake(conSem, portMAX_DELAY);	//wait for signal to run
		while(true){
			while((ID = rand()) == 0); //ID cannot be 0
			int i;
			xSemaphoreTake(sentIDs_mutex, portMAX_DELAY);
			for(i = 0; i < (sizeof(sentIDs) / sizeof(int) - 1); i++)
				if(sentIDs[i] == ID)
					break;
			if(i == (sizeof(sentIDs) / sizeof(int) - 1))
				break;
			xSemaphoreGive(sentIDs_mutex);
		}		
		for(int i = 0; i < (sizeof(sentIDs) / sizeof(int) - 1); i++)
					sentIDs[i] = sentIDs[i + 1];		
		 sentIDs[sizeof(sentIDs) / sizeof(int) - 1] = ID;
		 xSemaphoreGive(sentIDs_mutex);
		 
		 char message[maxReplyLength];
		 sprintf(message, "$PING,%d\n", ID);
		 while(errQUEUE_FULL == xQueueSendToBack(replyQueue, message, 0));	//spins if the queue is full
	}
}
