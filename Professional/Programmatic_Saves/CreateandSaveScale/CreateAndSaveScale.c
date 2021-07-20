/*********************************************************************
*
* ANSI C Example program:
*    CreateAndSaveScale.c
*
* Example Category:
*    Professional
*
* Description:
*    This example demonstrates how to save a NI-DAQmx linear scale
*    programmatically. The saved scale shows up in MAX and can be
*    used like just like the NI-DAQmx scales that are created in MAX.
*
* Instructions for Running:
*    1. Set the slope for the linear scale.
*    2. Set the y intercept for the linear scale.
*    3. Select the pre-scaled units for the scale.
*    4. Type in the scaled units for the scale.
*    5. Assign a name to the scale you are creating.
*    6. Set the Author tag for the scale.
*    7. Set the appropriate attributes for the scale. More
*       information on these attributes can be found inthe function
*       help for DAQmxSaveScale.
*
* Steps:
*    1. Create a linear scale.
*    2. Call the DAQmxSaveScale function. This saves the scale which
*       can be viewed in Measurement and Automation Explorer.
*    3. Display an error if any.
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
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Create Scale Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateLinScale("TempScaleName",5.0,3.0,DAQmx_Val_Volts,"RPMs"));

	/*********************************************/
	// DAQmx Save Code
	/*********************************************/
	DAQmxErrChk (DAQmxSaveScale("TempScaleName","NI-DAQmx Example Linear Scale","National Instruments",
		DAQmx_Val_Save_Overwrite | DAQmx_Val_Save_AllowInteractiveEditing | DAQmx_Val_Save_AllowInteractiveDeletion));
	printf("Successfully created and saved scale.\n");

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
