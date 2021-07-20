/*********************************************************************
*
* ANSI C Example program:
*    ContAcqSndPressSamps-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to create a sound pressure task
*    for acquiring data from a microphone. The code scales the
*    microphone voltage to proper engineering units and provides IEPE
*    current excitation to the microphone, if necessary.

*
*    NOTE: This code is intended to run with Dynamic Signal
*    Acquisition (DSA) devices. It will not work "as-is" with
*    multifunction (MIO) DAQ hardware.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DSA device.
*    2. Enter the maximum expected sound pressure level in dB. DAQmx
*       will set the gain on your DSA device to provide the best
*       possible dynamic range for a sound pressure that does not
*       exceed the level you enter.
*    3. Set the rate of the acquisition. Also set the Samples to Read
*       control. This will determine how many samples are read at a
*       time. This also determines how many points are plotted on the
*       graph each time.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input sound pressure channel. This step
*       specifies the expected sound pressure level range, the
*       microphone sensitivity, and the IEPE excitation settings.
*    3. Set the sample rate and define a continuous acquisition.
*    4. Call the Start function to start the acquisition.
*    5. Read the waveform data in the EveryNCallback function until
*       the user hits the stop button or an error occurs.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your microphone input terminal matches the Physical
*    Channel I/O control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	// Note:  DSA devices now support including channels from multiple 
	// devices in a single task. DAQmx automatically synchronizes the 
	// devices in such a task.  See the DAQmx Help >> Device Considerations >>
	// Multi Device Tasks section for further details.
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIMicrophoneChan(taskHandle,"PXI1Slot2/ai0","",DAQmx_Val_PseudoDiff,DAQmx_Val_Pascals,50,120.0,DAQmx_Val_Internal,0.004,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",25600.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1024));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,1000,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Press Enter to interrupt\n");
	getchar();

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalRead=0;
	int32       read=0;
	float64     data[1000];

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,1000,10.0,DAQmx_Val_GroupByScanNumber,data,1000,&read,NULL));
	if( read>0 ) {
		printf("Acquired %d samples. Total %d\r",(int)read,(int)(totalRead+=read));
		fflush(stdout);
	}

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
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
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
