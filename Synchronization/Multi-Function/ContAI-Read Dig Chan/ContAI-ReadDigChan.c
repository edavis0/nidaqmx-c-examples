/*********************************************************************
*
* ANSI C Example program:
*    ContAI-ReadDigChan.c
*
* Example Category:
*    Sync
*
* Description:
*    This example demonstrates how to continuously acquire analog and
*    digital data at the same time, synchronized with one another on
*    the same device.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       analog signal is input on the DAQ device. Also, select the
*       channel to correspond to where your digital signal is input
*       on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the sample rate of the acquisition.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.

*    Note: This example requires two DMA channels to run. If your
*          hardware does not support two DMA channels, you need to
*          set the Data Transfer Mechanism attribute for the Digital
*          Input Task to use "Interrupts".
*
*    Refer to your device documentation to determine how many DMA
*    channels are supported for your hardware.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel. Also, create a
*       Digital Input channel.
*    3. Set the rate for the sample clocks. Additionally, define the
*       sample modes to be continuous.

*    3a. Call the GetTerminalNameWithDevPrefix function. This will
*    take a Task and a terminal and create a properly formatted
*    device + terminal name to use as the source of the digital
*    sample clock.
*    4. Call the Start function to arm the two tasks. Make sure the
*       digital input task is armed before the analog input task.
*       This will ensure both will start at the same time.
*    5. Read the waveform data continuously until the user hits the
*       stop button or an error occurs.
*    6. Call the Stop function to stop the acquisition.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminals match the Physical Channel
*    I/O controls.
*
*********************************************************************/

#include <string.h>
#include <stdio.h>
#include <NIDAQmx.h>

static TaskHandle  AItaskHandle=0,DItaskHandle=0;


#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

int main(void)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};
	char    trigName[256];

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/

	DAQmxErrChk (DAQmxCreateTask("",&AItaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(AItaskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(AItaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	DAQmxErrChk (GetTerminalNameWithDevPrefix(AItaskHandle,"ai/SampleClock",trigName));
	DAQmxErrChk (DAQmxCreateTask("",&DItaskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(DItaskHandle,"Dev1/port0","",DAQmx_Val_ChanForAllLines));
	DAQmxErrChk (DAQmxCfgSampClkTiming(DItaskHandle,trigName,10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(AItaskHandle,DAQmx_Val_Acquired_Into_Buffer,1000,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(AItaskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(DItaskHandle));
	DAQmxErrChk (DAQmxStartTask(AItaskHandle));

	printf("Acquiring samples continuously. Press Enter to interrupt\n");
	printf("\nRead:\tAI\tDI\tTotal:\tAI\tDI\n");
	getchar();

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( AItaskHandle ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AItaskHandle);
		DAQmxClearTask(AItaskHandle);
		AItaskHandle = 0;
	}
	if( DItaskHandle ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(DItaskHandle);
		DAQmxClearTask(DItaskHandle);
		DItaskHandle = 0;
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}

static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[])
{
	int32	error=0;
	char	device[256];
	int32	productCategory;
	uInt32	numDevices,i=1;

	DAQmxErrChk (DAQmxGetTaskNumDevices(taskHandle,&numDevices));
	while( i<=numDevices ) {
		DAQmxErrChk (DAQmxGetNthTaskDevice(taskHandle,i++,device,256));
		DAQmxErrChk (DAQmxGetDevProductCategory(device,&productCategory));
		if( productCategory!=DAQmx_Val_CSeriesModule && productCategory!=DAQmx_Val_SCXIModule ) {
			*triggerName++ = '/';
			strcat(strcat(strcpy(triggerName,device),"/"),terminalName);
			break;
		}
	}

Error:
	return error;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalAI=0,totalDI=0;
	int32       readAI,readDI;
	float64     AIdata[1000];
	uInt32      DIdata[1000];

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(AItaskHandle,1000,10.0,DAQmx_Val_GroupByChannel,AIdata,1000,&readAI,NULL));
	DAQmxErrChk (DAQmxReadDigitalU32(DItaskHandle,1000,10.0,DAQmx_Val_GroupByChannel,DIdata,1000,&readDI,NULL));

	printf("\t%d\t%d\t\t%d\t%d\r",(int)readAI,(int)readDI,(int)(totalAI+=readAI),(int)(totalDI+=readDI));
	fflush(stdout);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		if( AItaskHandle ) {
			DAQmxStopTask(AItaskHandle);
			DAQmxClearTask(AItaskHandle);
			AItaskHandle = 0;
		}
		if( DItaskHandle ) {
			DAQmxStopTask(DItaskHandle);
			DAQmxClearTask(DItaskHandle);
			DItaskHandle = 0;
		}
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		if( DItaskHandle ) {
			DAQmxStopTask(DItaskHandle);
			DAQmxClearTask(DItaskHandle);
			DItaskHandle = 0;
		}
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
