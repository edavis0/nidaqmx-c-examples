/*********************************************************************
*
* ANSI C Example program:
*    Acq-IntClk-DigStartRef.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a finite amount of data
*    using an internal clock and a digital start and reference
*    trigger.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select the number of Samples per Channel to acquire.
*    4. Set the Rate of the acquisition.
*    Note: The Rate should be AT LEAST twice as fast as the maximum
*          frequency component of the signal being acquired.
*    5. Set the Source of the Start and Reference trigger as well as
*       the polarity of the Start and Reference Edges. Additionally,
*       set the number of Pre-Trigger Samples to be acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Define the parameters for an Internal Clock Source.
*       Additionally, define the sample mode to be Finite.
*    4. Define the parameters for a Digital Edge Start Trigger and a
*       Digital Edge Reference Trigger.
*    5. Call the Start function to start the acquisition.
*    6. Use the Read function to retrieve the waveform. Set a timeout
*       so an error is returned if the samples are not returned in
*       the specified time limit.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O Control. Also, make sure your digital trigger
*    terminals match the Trigger Source Controls. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

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
const int32 sampleMode = DAQmx_Val_FiniteSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxCfgDigEdgeStartTrig Options
const char *startTriggerSource = "/Dev1/PFI0"; // The name of a terminal where there is a digital signal to use as the source of the trigger.
const int32 startTriggerEdge = DAQmx_Val_Rising; // Specifies on which edge of a digital signal to start acquiring or generating samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling

// DAQmxCfgDigEdgeRefTrig Options
const char *refTriggerSource = "/Dev1/PFI1"; // Specifies the name of a terminal where there is a digital signal to use as the source of the trigger. 
const int32 refTriggerEdge = DAQmx_Val_Rising; // Specifies on which edge of the digital signal the Reference Trigger occurs. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const uInt32 pretriggerSamples = 5; // The minimum number of samples per channel to acquire before recognizing the Reference Trigger.

// DAQmxReadAnalogF64 Options
const float64 timeout = 10; // The amount of time, in seconds, to wait for the function to read the sample(s).
const bool32 fillMode = DAQmx_Val_GroupByScanNumber; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	int32       read;
	float64     data[sampsPerChan];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,physicalChannel,"",terminalConfig,minVal,maxVal,units,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandle,startTriggerSource,startTriggerEdge));
	DAQmxErrChk (DAQmxCfgDigEdgeRefTrig(taskHandle,refTriggerSource,refTriggerEdge,pretriggerSamples));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,sampsPerChan,timeout,fillMode,data,sampsPerChan,&read,NULL));

	printf("Acquired %d points\n",(int)read);

	// Display acquisition results
	for (int i = 0; i < sampsPerChan; i++)
	{
		printf("%.2f\n", data[i]);
	}
	printf("Press Enter key to end program.\n");

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
