/*********************************************************************
*
* ANSI C Example program:
*    ContStrainSamples.c
*
* Example Category:
*    AI
*
* Description:
*    This example acquires strain measurements on input channels.
*
* Instructions for Running:
*    1. Enter the list of physical channels, and set the attributes
*       of the strain configuration connected to all the channels.
*       The 'Maximum Value' and 'Minimum Value' inputs specify the
*       range, in strains, that you expect of your measurements.
*    2. Make sure all strain gages are in their relaxed state.
*    3. Run the example and do not start straining the gages until
*       data starts being plotted.
*
* Steps:
*    1. Create a task.
*    2. Create a Strain input channel.
*    3. Set timing parameters. Note that sample mode set to
*       Continuous Samples.
*    4. Set filter parameters.
*    5. Call the Start function to start the acquisition.
*    6. Read the data in the EveryNCallback function until the user
*       hits the stop button or an error occurs.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control.
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
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.

// DAQmxCreateAIStrainGageChan Options
const char *physicalChannel = ""; // The names of the physical channels to use to create virtual channels.
const float64 minVal = -0.001; // The minimum value, in units, that you expect to measure.
const float64 maxVal = 0.001; // The maximum value, in units, that you expect to measure.
const int32 units = DAQmx_Val_Strain; // The units to use to return the measurement. Options: DAQmx_Val_Strain, DAQmx_Val_FromCustomScale
const int32 strainConfig = DAQmx_Val_FullBridgeI; // The strain gage bridge configuration. Options: DAQmx_Val_FullBridgeI, DAQmx_Val_FullBridgeII, DAQmx_Val_FullBridgeIII, DAQmx_Val_HalfBridgeI, DAQmx_Val_HalfBridgeII, DAQmx_Val_QuarterBridgeI, DAQmx_Val_QuarterBridgeII
const int32 voltageExcitSource = DAQmx_Val_Internal; // The source of excitation. Options: DAQmx_Val_Internal, DAQmx_Val_External, DAQmx_Val_None
const float64 voltageExcitVal = 2.50; // Specifies in volts the amount of excitation supplied to the sensor.
const float64 gageFactor = 2.0; // The sensitivity of the strain gages and relates the change in electrical resistance to the change in strain.
const float64 initialBridgeVoltage = 0.0; // The bridge output voltage in the unloaded condition.
const float64 nominalGageResistance = 120.0; // The resistance, in ohms, of the gages in an unstrained position.
const float64 poissonRatio = 0.285; // The ratio of lateral strain to axial strain in the material in which you measure strain.
const float64 leadWireResistance = 0.0; // The amount, in ohms, of resistance in the lead wires.

// DAQmxCfgSampClkTiming Options
const char *clockSource = ""; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxRegisterEveryNSamplesEvent Options
const int32 everyNsamplesEventType = DAQmx_Val_Acquired_Into_Buffer; // The type of event you want to receive. Options: DAQmx_Val_Acquired_Into_Buffer, DAQmx_Val_Transferred_From_Buffer
const uInt32 options = 0; // Use this parameter to set certain options. Pass a value of zero if no options need to be set. Options: 0, DAQmx_Val_SynchronousEventCallbacks

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIStrainGageChan(taskHandle,physicalChannel,"",minVal,maxVal,units,strainConfig,voltageExcitSource,voltageExcitVal,gageFactor,initialBridgeVoltage,nominalGageResistance,poissonRatio,leadWireResistance,""));
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
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,-1,10.0,DAQmx_Val_GroupByScanNumber,data,1000,&read,NULL));
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
