/*********************************************************************
*
* ANSI C Example program:
*    CreateAndSaveTask.c
*
* Example Category:
*    Professional
*
* Description:
*    This example demonstrates how to save a NI-DAQmx task
*    programmatically. The saved task shows up in MAX and can be used
*    like just like the NI-DAQmx tasks that are created in MAX.
*
* Instructions for Running:
*    1. Select the Physical Channel corresponding to where your
*       signal is input on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select the number of samples to acquire.
*    4. Set the rate of the acquisition.
*    Note: The rate should be AT LEAST twice as fast as the maximum
*          frequency component of the signal being acquired.
*    5. Assign a name to the task you are creating.
*    6. Set the Author tag for the task.
*    7. Set the appropriate attributes for the task. More information
*       on these attributes can be found in the function help for
*       DAQmxSaveTask.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the rate for the sample clock. Additionally, define the
*       sample mode to be finite and set the number of samples to be
*       acquired per channel.
*    4. Call the DAQmxSaveTask function. This saves the task which
*       can be viewed in Measurement and Automation Explorer.
*    5. Call the Clear Task function to clear the task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    No I/O connections are needed.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Create Task Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,1000));

	/*********************************************/
	// DAQmx Save Code
	/*********************************************/
	DAQmxErrChk (DAQmxSaveTask(taskHandle,"NI-DAQmx Example Task","National Instruments",
		DAQmx_Val_Save_Overwrite | DAQmx_Val_Save_AllowInteractiveEditing | DAQmx_Val_Save_AllowInteractiveDeletion));
	printf("Successfully created and saved task.\n");

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 )
		DAQmxClearTask(taskHandle);
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
