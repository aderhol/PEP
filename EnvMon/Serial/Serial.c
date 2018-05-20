#include <stdlib.h>
#include <pthread.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "Serial.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include <stdbool.h>
#include <Error.h>
#include <stdint.h>
#include <unistd.h>

#include <Plot.h>

static pthread_t ser;												// szál
static int serialfd;												// fájl azonosító
static struct termios SerialSettings;								// termios struktúra a sorosport kezelésére

static void Close_SerialPort();
static void TokenPing(char *str, char* out);
static bool StartTransfer(char* str);
static void *Serial_Thread(void* arg); 

bool __attribute__((weak)) sendToPlot(char const* str)
{
	char a[1000];
	sprintf(a,"Plot: %s", str);
	return sendToError(a);
}

static void *Serial_Thread(void* arg)
{
    char readChar;
	char buffToSend[200]; 					                        // $DAT,temp,temp_time,conc,conc_time\r egy teljes adat tárolására
	int readeBytes;							                        // olvasott bájtok száma
    int i = 0;
    
	    while(!isStopRequested()){                                  // ammíg le nem állítja a main
      
		    while(0 == (readeBytes = read(serialfd,&readChar,1)))
		    	usleep(8);	            // olvasás a sorosportról ( 1db karakter )
		    if(readeBytes == -1)									// olvasási hiba
		    {
                sendToError("Serial port read error!");
		    }
		    else
		    {
		        if(readChar == '\n')
                {
                    if(i != 0 && buffToSend[i-1] == '\r')
                    {
                        buffToSend[i-1] = '\0';
                    }
                    else
                    {
                        buffToSend[i] = '\0';
                    }
                    StartTransfer(buffToSend);
                    i = 0;
                }
                else
                {
                	if(i >= sizeof(buffToSend) / sizeof(char)){
                		sendToError("Serial buffer overflow!");
                		while(true);
                	}
                    buffToSend[i++] = readChar;
                }
		    }
        }
        Close_SerialPort(&serialfd);								// soros port bezárása
        
        sendToError("Serial Out!");
        
        return NULL;
}

bool Init_SerialPort(pthread_t *threads, int * count, int maxCount)
{
	
	char port[100], portNum[100];
	printf("Specify the serial port number: ");
	fgets(portNum, 99, stdin);
	sscanf(portNum, "%s", portNum);
	sprintf(port, "/dev/ttyACM%s", portNum);
	*count = 0;
	serialfd = open(port, O_RDWR | O_NOCTTY);  	 //O_RDWR: olvasásre és írásra nyitjuk ki ttyAMC0-át
 	 	 	 	 	 	 	 	 	 	 	         	     //O_NOCTTY a terminál nem vezérelheti a portot
	if((serialfd)==-1)
	{
		printf("Error opening serial port!\n");
		return false;
	}

	tcgetattr(serialfd,&SerialSettings);  						//aktuális beállítás lekérdezése

	cfsetispeed(&SerialSettings,B115200);					    //bemeneti sebesség beállítása
	cfsetospeed(&SerialSettings,B115200);					    //kimeneti sebesség beállítása

	SerialSettings.c_cflag &= ~PARENB;						    //nincs paritás
	SerialSettings.c_cflag &= ~CSTOPB;						    //1 stop bit
	SerialSettings.c_cflag &= ~CSIZE;
	SerialSettings.c_cflag |= CS8;							    //8bit adat (8N1)
	SerialSettings.c_cflag &= ~CRTSCTS;					    // output hardware flow control
	SerialSettings.c_cflag |= CREAD | CLOCAL;				    //olvasás engedélyezése és helyi kapcsolat

	SerialSettings.c_iflag &= ~(IXON | IXOFF | IXANY);			//egyéb dolgok beállítása amit nem vágok :D
	SerialSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);		//non canonic mód


	SerialSettings.c_oflag &= ~OPOST;

	SerialSettings.c_cc[VMIN] = 0;								// 1 karakter olvasásos polling
	SerialSettings.c_cc[VTIME] = 0;							// timeout ameddig a karakter meg nem érkezik ennyit vár


	if(tcsetattr(serialfd,TCSANOW,&SerialSettings)!=0)		    // beállítások véglegesítése
	{
		printf("Error setting up serial port!\n");
		return false;
	}

	tcflush(serialfd,TCIFLUSH);	

	if(pthread_create(&ser,NULL,Serial_Thread,(void*)0))	//sorosport thread létrehozása
	{
		printf("Error creating serial thread!\n");
		return false;
	}

	if(maxCount >= 1)
	{
		threads[0] = ser;
		*count = 1;
		return true;
	}
	else
	{
		return false;
	}
}

static void Close_SerialPort()
{
	close(serialfd);
}

static bool StartTransfer(char* str)
{
	char* strtokState;
	if(str[0] != '$')						// nem valódi adat
	{
		sendToError("Invalid serial data ($ is missing).");
        return false;
	}
	else
	{
		char* token;					// command kiolvasás
		str++;	//$ levágása
        token = strtok_r(str,",", &strtokState);
		
			if(strcmp(token,"DAT") == 0)			// adat érkezett
			{
				sendToPlot(strtok_r(NULL, "", &strtokState));
                return true;
			}
			else if(strcmp(token,"PING") == 0)							// ping érkezett
			{
				TokenPing(strtok_r(NULL, "", &strtokState), str);  
                write(serialfd, (void*)str, strlen(str));
                return true;     
			}
			else if(strcmp(token, "ERROR") == 0)
			{
				char message[200];
				sprintf(message, "Serial error ($ERROR): %s", strtok_r(NULL, "", &strtokState));
				sendToError(message);
				return true;
			}
			else
			{
				char message[200];
				sprintf(message, "Invalid serial message ($%s): %s",token, strtok_r(NULL, "", &strtokState));
				sendToError(message);
				return false;
			}
	}
}

static void TokenPing(char *str, char* out)
{	
	char* strtokState;
	char* ID_str = strtok_r(str, ",", &strtokState);
	
	if(strtok_r(NULL, ",", &strtokState) != NULL){
		sendToError("Invalid serial message: $PING has too many arguments.");
		out[0] = '\0'; //send nothing
		return;
	}
	
	int32_t ID = atoi(ID_str);
	
	if(0 == ID){
		sendToError("Invalid ID in $PING.");
		out[0] = '\0'; 
		return;
	}
	
    sprintf(out, "$ALIVE,%d\n", ID - 1);
}
   
