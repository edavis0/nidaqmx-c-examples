/*********************************************************************
*
* ANSI C Example program:
*    ContThrmcplSamps-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to make continuous, hardware-timed
*    temperature measurement using a thermocouple.
*
* Instructions for Running:
*    1. Specify the Physical Channel where you have connected the
*       thermocouple.
*    2. Enter the Minimum and Maximum temperature values in degrees C
*       you expect to measure. A smaller range will allow a more
*       accurate measurement.
*    3. Enter the scan rate at which you want to run the acquisition.
*    4. Specify the type of thermocouple you are using.
*    5. Thermocouple measurements require cold-junction compensation
*       (CJC) to correctly scale them. Specify the source of your
*       cold-junction compensation.
*    6. If your CJC source is "Internal", skip the rest of the steps.
*    7. If your CJC source is "Constant Value", specify the value
*       (usually room temperature) in degrees C.
*    8. If your CJC source is "Channel", specify the CJC Channel
*       name.
*    9. Specify the appropriate Auto Zero Mode. See your device's
*       hardware manual to find out if your device supports this
*       attribute.
*
* Steps:
*    1. Create a task.
*    2. Create a Thermocouple (TC) temperature measurement channel.
*    3. If your device supports Auto Zero Mode, set the AutoZero
*       attribute for all channels in the task.
*    4. Call the Timing function to specify the hardware timing
*       parameters. Use device's internal clock, continuous mode
*       acquisition and the sample rate specified by the user.
*    5. Call the Start function to program and start the acquisition.
*    6. Read N samples and plot it. By default, the Read function
*       reads all available samples, but you can specify how many
*       samples to read at a time and the timeout value. Continue
*       reading data until the stop button is pressed or an error
*       occurs.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Connect your thermocouple to the terminals corresponding to the
*    Physical Channel I/O Control value.
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
const float64 sampleRate = 10.0; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 10; // The number of samples to acquire or generate for each channel in the task.
// DAQmxCreateAIThrmcplChan Options
const char *physicalChannel = "Dev1/ai0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const float64 minVal = 0.0; // The minimum value, in units, that you expect to measure.
const float64 maxVal = 100.0; // The maximum value, in units, that you expect to measure.
const int32 units = DAQmx_Val_DegC; // The units to use to return the measurement. Options: DAQmx_Val_DegC, DAQmx_Val_DegF, DAQmx_Val_Kelvins, DAQmx_Val_DegR
const int32 thermocoupleType = DAQmx_Val_J_Type_TC; // The type of thermocouple connected to the channel. Options: DAQmx_Val_J_Type_TC, DAQmx_Val_K_Type_TC, DAQmx_Val_N_Type_TC, DAQmx_Val_R_Type_TC, DAQmx_Val_S_Type_TC, DAQmx_Val_T_Type_TC, DAQmx_Val_B_Type_TC, DAQmx_Val_E_Type_TC
const int32 cjcSource = DAQmx_Val_BuiltIn; // The source of cold junction compensation. Options: DAQmx_Val_BuiltIn, DAQmx_Val_ConstVal, DAQmx_Val_Chan
const float64 cjcVal = 25.0; // The temperature of the cold junction of the thermocouple if you set cjcSource to DAQmx_Val_ConstVal.
const char *cjcChannel = ""; // The channel that acquires the temperature of the thermocouple cold-junction if you set cjcSource to DAQmx_Val_Chan.
// DAQmxCfgSampClkTiming Options
const char *clockSource = "OnboardClock"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint
// DAQmxRegisterEveryNSamplesEvent Options
const int32 everyNsamplesEventType = DAQmx_Val_Acquired_Into_Buffer; // The type of event you want to receive. Options: DAQmx_Val_Acquired_Into_Buffer, DAQmx_Val_Transferred_From_Buffer
const uInt32 nSamples = 10; // The number of samples after which each event should occur.
const uInt32 options = 0; // Use this parameter to set certain options. Pass a value of zero if no options need to be set. Options: 0, DAQmx_Val_SynchronousEventCallbacks
// DAQmxReadAnalogF64 Options
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for the function to read the sample(s).
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
	DAQmxErrChk (DAQmxCreateAIThrmcplChan(taskHandle,physicalChannel,"",minVal,maxVal,units,thermocoupleType,cjcSource,cjcVal,cjcChannel));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));

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
