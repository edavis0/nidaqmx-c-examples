/*********************************************************************
*
* ANSI C Example program:
*    ContGen-ExtClk-DigStart.c
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to continuously output a waveform
*    using an external sample clock and a digital start trigger.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is output on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage range.
*    3. Select the Sample Clock source.
*    4. Select the Digital Trigger source.
*    5. Specify the desired Trigger Edge.
*    6. Select the desired waveform type.
*    7. The rest of the parameters in the Waveform Information
*       section will affect the way the waveform is created, before
*       it's sent to the analog output of the board. Select the
*       number of samples per cycle and the total number of cycles to
*       be used as waveform data.
*
* Steps:
*    1. Create a task.
*    2. Create an Analog Output Voltage channel.
*    3. Define the Sample Clock source. Additionally, define the
*       sample mode to be continuous.
*    4. Define the Triggering parameters: Source and Edge.
*    5. Write the waveform to the output buffer.
*    6. Call the Start function.
*    7. Wait until the user presses the Stop button.
*    8. Call the Clear Task function to clear the Task.
*    9. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminal matches the Physical
*    Channel I/O Control. Also, make sure your digital trigger
*    terminal matches the Trigger Source Control. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <NIDAQmx.h>
#include <stdio.h>
#include <math.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

#define PI	3.1415926535

int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 1000; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.

// DAQmxCreateAOVoltageChan Options
const char *physicalChannel = "Dev1/ao0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const float64 minVal = -10.0; // The minimum value, in units, that you expect to generate.
const float64 maxVal = 10.0; // The maximum value, in units, that you expect to generate.
const int32 units = DAQmx_Val_Volts; // The units in which to generate voltage. Options: DAQmx_Val_Volts, DAQmx_Val_FromCustomScale

// DAQmxCfgSampClkTiming Options
const char *clockSource = "/Dev1/PFI0"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task agenerates samples continuously or if it generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxCfgDigEdgeStartTrig Options
const char *startTriggerSource = "/Dev1/PFI0"; // The name of a terminal where there is a digital signal to use as the source of the trigger.
const int32 startTriggerEdge = DAQmx_Val_Rising; // Specifies on which edge of a digital signal to start acquiring or generating samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling

// DAQmxWriteAnalogF64 Options
const bool32 autoStart = 0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const float32 timeout = 10; // The amount of time, in seconds, to wait for the function to read the sample(s).
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	float64     data[sampsPerChan];
	char        errBuff[2048]={'\0'};
	int         i=0;

	for(;i<1000;i++)
		data[i] = 9.95*sin((double)i*2.0*PI/(double)sampsPerChan);

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandle,physicalChannel,"",minVal,maxVal,units,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandle,startTriggerSource,startTriggerEdge));

	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle,sampsPerChan,autoStart,timeout,dataLayout,data,NULL,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Generating voltage continuously. Press Enter to interrupt\n");
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
