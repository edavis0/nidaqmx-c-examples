/*********************************************************************
*
* ANSI C Example program:
*    ContStrainSampleswCal.c
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
*    1a.Select the Filter Setting to use. Default means that for a
*    given device its default filter setting and cutoff frequency
*    will be used. Enable will explicitly turn on the filter for a
*    given device and Disable will explicitly turn off the filter for
*    a given device.
*    2. Make sure all strain gages are in their relaxed state.
*    3. You may turn on the 'Do Strain Null?' checkbox to
*       automatically null out your gage by performing a hardware
*       nulling operation (if supported by the hardware) followed by
*       a software nulling operation. (NOTE: The software nulling
*       operation will cause a loss in dynamic range while a hardware
*       nulling operation will not cause any loss in the dynamic
*       range).
*    4. You can turn on the 'Do Shunt Cal?' checkbox to perform a
*       shunt calibration (gain adjust calculation) on your gage (the
*       location and value of the shunt resistor are needed). If you
*       are using an SCXI-1520/PXI-4220, you can turn on the 'Measure
*       actual excitation?' button for more accurate results.
*    5. Run the example and do not start straining the gages until
*       data starts being plotted.
*
* Steps:
*    1. Create a task.
*    2. Create a Strain input channel.
*    3. Set timing parameters. Note that sample mode set to
*       Continuous Samples.
*    4. Set filter parameters.
*    5. If nulling is desired, call
*       DAQmxPerformBridgeOffsetNullingCal function to perform both
*       hardware nulling (if supported) and software nulling.
*    6. If shunt calibration is desired (should only be done if you
*       have shunt resistors connected), perform shunt calibration by
*       calling function DAQmxPerformStrainShuntCal (if you are using
*       an SCXI-1520/PXI-4220, you can measure the actual excitation
*       on your bridge for more accurate measurements).
*    7. Call the Start function to start the acquisition.
*    8. Read the data in the EveryNCallback function until the user
*       hits the stop button or an error occurs.
*    9. Call the Clear Task function to clear the Task.
*    10. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control.
*
*********************************************************************/

#include <stdio.h>
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
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIStrainGageChan(taskHandle,"","",-0.001,0.001,DAQmx_Val_Strain,DAQmx_Val_FullBridgeI,DAQmx_Val_Internal,2.50,2.0,0.0,120.0,0.285,0.0,""));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",10.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxSetAILowpassEnable(taskHandle,"",1));
	DAQmxErrChk (DAQmxSetAILowpassCutoffFreq(taskHandle,"",10));

	DAQmxErrChk (DAQmxPerformBridgeOffsetNullingCal(taskHandle,""));
	DAQmxErrChk (DAQmxPerformStrainShuntCal(taskHandle,"",100000,DAQmx_Val_R4,0));

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
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,-1,10.0,DAQmx_Val_GroupByScanNumber,data,1000,&read,NULL));
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
