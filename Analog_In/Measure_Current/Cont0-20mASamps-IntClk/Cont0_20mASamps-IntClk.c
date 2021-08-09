/*********************************************************************
*
* ANSI C Example program:
*    Cont0_20mASamps-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to continuously measure current
*    using an internal hardware clock for timing.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum current ranges, in Amps.
*    Note: For better accuracy try to match the input ranges to the
*          expected current level of the measured signal.
*    3. Set the rate of the acquisition. Higher values will result in
*       faster updates, approximately corresponding to samples per
*       second. Also, set the number of samples to read at a time.
*       This will correspond to how many samples are shown on the
*       graph at once.
*    4. Enter in the parameters of your current shunt resistor. The
*       shunt resistor location will usually be "External" unless you
*       are using an SCXI current input terminal block or SCC current
*       input module. The shunt resistor value should correspond to
*       the shunt resistor that you are using, and is specified in
*       ohms. If you are using an SCXI current input terminal block
*       or SCC current input module, you must select "Internal" for
*       the shunt resistor location.
*
* Steps:
*    1. Create an analog input current channel.
*    2. Create a new Task and Setup Timing.
*    3. Use the DAQmxReadAnalogF64 to measure multiple samples from
*       multiple channels on the data acquisition card. Set a timeout
*       so an error is returned if the samples are not returned in
*       the specified time limit.
*    4. Call the Clear Task function to clear the Task.
*    5. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the physical
*    channel I/O control. If you are using an external shunt
*    resistor, make sure to hook it up in parallel with the current
*    signal you are trying to measure.
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
const float64 sampleRate = 1000; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to generate for each channel in the task.

// DAQmxCreateAICurrentChan Options
const char *physicalChannel = "Dev1/ai0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const int32 terminalConfig = DAQmx_Val_Cfg_Default; // The input terminal configuration for the channel. Options: DAQmx_Val_Cfg_Default, DAQmx_Val_RSE, DAQmx_Val_NRSE, DAQmx_Val_Diff, DAQmx_Val_PseudoDiff
const float64 minVal = 0.0; // The minimum value, in units, that you expect to measure.
const float64 maxVal = 0.02; // The maximum value, in units, that you expect to measure.
const int32 units = DAQmx_Val_Amps; // The units in which to generate voltage. Options: DAQmx_Val_Amps, DAQmx_Val_FromCustomScale
const int32 shuntResistorLoc = DAQmx_Val_Default; // The location of the shunt resistor. Options: DAQmx_Val_Default, DAQmx_Val_Internal, DAQmx_Val_External
const float64 extShuntResistorVal = 249.0; // The value, in ohms, of an external shunt resistor.

// DAQmxCfgSampClkTiming Options
const char *clockSource = "OnboardClock"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_FiniteSamps; // Specifies whether the task agenerates samples continuously or if it generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxRegisterEveryNSamplesEvent Options
const int32 everyNsamplesEventType = DAQmx_Val_Acquired_Into_Buffer; // The type of event you want to receive. Options: DAQmx_Val_Acquired_Into_Buffer, DAQmx_Val_Transferred_From_Buffer
const uInt32 options = 0; // Use this parameter to set certain options. Pass a value of zero if no options need to be set. Options: 0, DAQmx_Val_SynchronousEventCallbacks

// DAQmxReadAnalogF64 Options
const float64 timeout = 10; // The amount of time, in seconds, to wait for the function to read the sample(s).
const bool32 fillMode = DAQmx_Val_GroupByScanNumber; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int         error=0;
	char        errBuff[2048]={'\0'};
	TaskHandle  taskHandle=0;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAICurrentChan(taskHandle,physicalChannel,"",terminalConfig,minVal,maxVal,units,shuntResistorLoc,extShuntResistorVal,""));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,everyNsamplesEventType,sampsPerChan,options,EveryNCallback,NULL));
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
		printf("DAQmx Error %s",errBuff);
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
			printf("%.3f\n", data[i]);
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
