/*********************************************************************
*
* ANSI C Example program:
*    CreateAndSaveGlobalChannel.c
*
* Example Category:
*    Professional
*
* Description:
*    This example demonstrates how to save a NI-DAQmx channel
*    programmatically. The saved channel shows up in MAX and can be
*    used like just like the NI-DAQmx Global Channels that are
*    created in MAX.
*
* Instructions for Running:
*    1. Select the Physical Channel corresponding to where your
*       signal is input on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select the Input Terminal Configuration for the channel you
*       are creating.
*    4. Assign a name to the Global Virtual Channel you are creating.
*    5. Set the Author tag for the Global Virtual Channel.
*    6. Set the appropriate attributes for the global virtual
*       channel. More information on these attributes can be found in
*       the function help for DAQmxSaveGlobalChan.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel. Make sure to assign a
*       name to the channel you are creating. This example creates a
*       DAQmx Task which contains a local virtual channel called
*       "NI-DAQmx AI Voltage Channel".
*    3. Call the DAQmxSaveGlobalChan function. This saves your local
*       virtual channel called "NI-DAQmx AI Voltage Channel" as a
*       Global Channel called "NI-DAQmx Example AI Channel". This new
*       global channel can be viewed in Measurement and Automation
*       Explorer.
*    4. Call the Clear Task function to clear the task.
*    5. Display an error if any.
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
	// DAQmx Create Channel Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","NI-DAQmx Example AI Channel",DAQmx_Val_RSE,
		-10.0,10.0,DAQmx_Val_Volts,NULL));

	/*********************************************/
	// DAQmx Save Code
	/*********************************************/
	DAQmxErrChk (DAQmxSaveGlobalChan(taskHandle,"NI-DAQmx Example AI Channel","NI-DAQmx Example AI Channel","National Instruments",
		DAQmx_Val_Save_Overwrite | DAQmx_Val_Save_AllowInteractiveEditing | DAQmx_Val_Save_AllowInteractiveDeletion));
	printf("Successfully created and saved global channel.\n");

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
