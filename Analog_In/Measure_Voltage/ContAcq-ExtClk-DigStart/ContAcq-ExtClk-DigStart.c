/*********************************************************************
*
* ANSI C Example program:
*    ContAcq-ExtClk-DigStart.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a continuous amount of
*    data using an external sample clock, started by a digital edge.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select a source for the sample clock.
*    4. Set the approximate Rate of the external clock. This allows
*       the internal characteristics of the acquisition to be as
*       efficient as possible. Also set the Samples to Read control.
*       This will determine how many samples are read at a time. This
*       also determines how many points are plotted on the graph each
*       time.
*    5. Select a source for the digital edge start trigger.
*    6. Select the edge, rising or falling, on which to trigger.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Define the parameters for an External Clock Source.
*       Additionally, define the sample mode to be continuous. The
*       external clock rate is given to allow the internal
*       characteristics of the acquisition to be as efficient as
*       possible.
*    4. Set the parameters for a digital edge start trigger.
*    5. Call the Start function to start the acquisition.
*    6. Read the waveform data in the EveryNCallback function until
*       the user hits the stop button or an error occurs.
*    Note: This example reads data from one or more channels and
*          returns an array of data. Use the Index Array function to
*          access an individual channel of data.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. Also, make sure that your digital trigger
*    signal is connected to the terminal specified in Trigger Source.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 1000.0; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.

// DAQmxCreateAIVoltageChan Options
const char *physicalChannel = "Dev1/ai0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const int32 terminalConfig = DAQmx_Val_Cfg_Default; // The input terminal configuration for the channel. Options: DAQmx_Val_Cfg_Default, DAQmx_Val_RSE, DAQmx_Val_NRSE, DAQmx_Val_Diff, DAQmx_Val_PseudoDiff
const float64 minVal = -10.0; // The minimum value, in units, that you expect to measure.
const float64 maxVal = 10.0; // The maximum value, in units, that you expect to measure.
const int32 units = DAQmx_Val_Volts; // The units to use to return the voltage measurements. Options: DAQmx_Val_Volts, DAQmx_Val_FromCustomScale

// DAQmxCfgSampClkTiming Options
const char *clockSource = "OnboardClock"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxCfgDigEdgeStartTrig Options
const char *startTriggerSource = "/Dev1/PFI0"; // The name of a terminal where there is a digital signal to use as the source of the trigger.
const int32 startTriggerEdge = DAQmx_Val_Rising; // Specifies on which edge of a digital signal to start acquiring or generating samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling

// DAQmxRegisterEveryNSamplesEvent Options
const int32 everyNsamplesEventType = DAQmx_Val_Acquired_Into_Buffer; // The type of event you want to receive. Options: DAQmx_Val_Acquired_Into_Buffer,  DAQmx_Val_Transferred_From_Buffer
const uInt32 nSamples = 10; // The number of samples after which each event should occur.
const uInt32 options = 0; // Use this parameter to set certain options. Pass a value of zero if no options need to be set. Options: 0,  DAQmx_Val_SynchronousEventCallbacks

// DAQmxReadAnalogF64 Options
const float64 timeout = 10; // The amount of time, in seconds, to wait for the function to read the sample(s).
const bool32 fillMode = DAQmx_Val_GroupByScanNumber; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,physicalChannel,"",terminalConfig,minVal,maxVal,units,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandle,startTriggerSource,startTriggerEdge));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,everyNsamplesEventType,nSamples,options,EveryNCallback,NULL));
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

int32 EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalRead=0;
	int32       read=0;
	float64     data[sampsPerChan];

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,sampsPerChan,timeout,fillMode,data,sampsPerChan,&read,NULL));
	if( read>0 ) {
		printf("Acquired %d samples. Total %d\n",(int)read,(int)(totalRead+=read));
		for (int i = 0; i < sampsPerChan; i++)
		{
			printf("%.2f\n", data[i]);
		}
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

int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
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
