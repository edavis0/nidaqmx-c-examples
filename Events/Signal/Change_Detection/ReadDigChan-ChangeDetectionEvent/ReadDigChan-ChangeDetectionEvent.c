/*********************************************************************
*
* ANSI C Example program:
*    ReadDigChan-ChangeDetectionEvent.c
*
* Example Category:
*    Events
*
* Description:
*    This example demonstrates how to use Change Detection events to
*    read values from one or more digital input channels. The Change
*    Detection events indicate when new values should be read.
*
* Instructions for Running:
*    1. Select the digital lines on the DAQ device to be read.
*    2. Select the rising and falling edge lines on which to perform
*       change detection.
*    3. Select the number of samples per read.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Input channel. Use one channel for all
*       lines. You can alternatively use one channel for each line,
*       but then use a different version of the DAQmx Read function.
*    3. Setup the Change Detection timing for the acquisition. The
*       timing is set to read continuously. The Rising Edge Lines and
*       Falling Edge Lines specify the digital line transitions that
*       will cause a sample to be read.
*    4. Register a callback to receive the Change Detection event.
*       Change Detection events occurs based on the specified Change
*       Detection timing configuration. The callback contains code to
*       read the values of the digital lines.
*    5. Call the Start function to start the task.
*    6. Receive Change Detection events until the stop button is
*       pressed or an error occurs.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminals match the Lines I/O
*    Control. In this case wire your digital signals to the first
*    eight digital lines on your DAQ Device.
*
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static TaskHandle	taskHandle;
static uInt32		numLines;
static uInt8		cachedData[200];

int32 CVICALLBACK ChangeDetectionCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);
void Cleanup (void);

int main(void)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(taskHandle,"Dev1/port0/line0:7","",DAQmx_Val_ChanPerLine));
	DAQmxErrChk (DAQmxCfgChangeDetectionTiming(taskHandle,"Dev1/port0/line0:7","Dev1/port0/line0:7",DAQmx_Val_ContSamps,1));
	DAQmxErrChk (DAQmxRegisterSignalEvent(taskHandle,DAQmx_Val_ChangeDetectionEvent,0,ChangeDetectionCallback,NULL));
	DAQmxErrChk (DAQmxGetTaskNumChans(taskHandle,&numLines));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	puts("Continuously reading. Press Enter key to interrupt\n");

	puts("Timestamp                 Data read   Changed Lines");

	getchar();

Error:
	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		Cleanup();
		printf("DAQmx Error: %s\n",errBuff);
	}
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}

int32 CVICALLBACK ChangeDetectionCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	int32   error=0;
	uInt8   data[200]={0};
	int32   numRead;
	uInt32  i=0;
	char    buff[512], *buffPtr;
	char    errBuff[2048]={'\0'};
	char	*timeStr;
	time_t	currTime;

	if( taskHandle ) {
		time (&currTime);
		timeStr = ctime(&currTime);
		timeStr[strlen(timeStr)-1]='\0';  // Remove trailing newline.

		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadDigitalLines(taskHandle,1,10.0,DAQmx_Val_GroupByScanNumber,data,8,&numRead,NULL,NULL));

		if( numRead ) {
			buffPtr = buff;
			strcpy(buff, timeStr);

			strcat(buff,"  ");
			buffPtr = buff + strlen(buff);
			for(;i<numLines;++i) {
				sprintf(buffPtr,"%d",data[i]);
				buffPtr++;
			}

			strcat(buff,"    ");
			buffPtr = buff + strlen(buff);
			for(i=0;i<numLines;++i) {
				sprintf(buffPtr,"%c",data[i]==cachedData[i]?'-':'X');
				buffPtr++;
				cachedData[i] = data[i];
			}
			puts(buff);
			fflush(stdout);
		}
	}
	return 0;

Error:
	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		Cleanup();
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

void Cleanup (void)
{
	if( taskHandle!=0 ) 
	{
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		taskHandle = 0;
	}
}
