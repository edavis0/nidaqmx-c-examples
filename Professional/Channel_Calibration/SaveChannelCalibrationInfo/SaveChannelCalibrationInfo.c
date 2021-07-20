/*********************************************************************
*
* ANSI C Example program:
*    SaveChannelCalibrationInfo.c
*
* Example Category:
*    Professional
*
* Description:
*    This example demonstrates how to access channel calibration data
*    of the specified task and save the information to disk.
*
* Instructions for Running:
*    1. Enter the name of an existing task.
*    2. Specify a file name where calibration information will be
*       saved.
*
* Steps:
*    1. Load the specified task.
*    2. Enable the channel calibration option,
*       ApplyCalibrationIfExpired. This prevents errors if the
*       calibration is expired. This setting is not saved and will
*       not affect the channel calibration.
*    3. Get the names of all the channels in the task.
*    4. Get calibration information for each channel.
*    5. Format calibration information into a report.
*    6. Save report to disk.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    No I/O connections are needed.
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NIDAQmx.h>

int32 GetChanCalInfoSize (TaskHandle taskHandle, const char *chanName, uInt32 *chanCalInfoSize);
int32 GetChanCalInfo (TaskHandle taskHandle, const char *chanName, char *calInfoBuf);
int32 GetChannelNames (TaskHandle task, uInt32 numChannels, char ***channelNames);
void FreeChannelNames (char **channelNames, uInt32 numChannels);

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
#define DAQmxNullChk(functionCall) {if ((functionCall) == 0) {error = DAQmxErrorPALMemoryFull;goto Error;}}

#define CHANNEL_NAME_FORMAT_STRING				"Channel Name: %s\n"
#define CALIBRATION_ENABLED_FORMAT_STRING		"Calibration Enabled: %s\n"
#define CALIBRATION_DATE_FORMAT_STRING			"Calibration Date: %d/%d/%d\n"
#define EXPIRATION_DATE_FORMAT_STRING			"Expiration Date: %d/%d/%d\n"
#define EMPTY_LINE								"\n"
#define SCALING_DATA_TYPE_FORMAT_STRING			"Scaling data: %s\n"
#define TABLE_SCALING_DATA_COLUMN_LABELS		"Reference Values\tUncalibrated Values\n"
#define POLYNOMIAL_SCALING_DATA_COLUMN_LABELS	"Forward Coefficient Values\tReverse Coefficient Values\n"
#define SCALING_DATA_FORMAT_STRING				"%.6E\t%.6E\n"
#define VERIFICATION_DATA_LABEL					"Verification data\n"
#define VERIFICATION_DATA_COLUMN_LABELS			"Reference Values\tAcquired Values\n"
#define VERIFICATION_DATA_FORMAT_STRING			"%.6E\t%.6E\n"
#define TABLE_SCALE								"Table scale"
#define POLYNOMIAL_SCALE						"Polynomial scale"
#define YES										"Yes"
#define NO										"No"


int main(void)
{
	int32       error=0;
	TaskHandle	taskHandle=0;
	char		*taskName="<Specify task name here>",*filePath="<Specify file path here.txt>";
	char		**channelNames = NULL,*calInfoBuf=NULL;
	uInt32 		numChannels,i,calInfoSize=0,chanCalInfoSize;
	int 		oneOrMoveChansWithValidCalInfo=0;
	FILE 		*file=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// Load the task
	/*********************************************/
	DAQmxErrChk (DAQmxLoadTask(taskName,&taskHandle));
	// If the calibration is expired DAQmx will return errors when we get the values of
	// calibration attributes.  We enable the Apply Calibration If Expired attribute
	// in order to avoid those errors.  This setting is not saved and will not affect the
	// channel calibration.
	DAQmxErrChk (DAQmxSetAIChanCalApplyCalIfExp(taskHandle,"",1));

	/*********************************************/
	// Get the names of all channels in the task
	/*********************************************/
	DAQmxErrChk (DAQmxGetTaskNumChans(taskHandle,&numChannels));
	DAQmxErrChk (GetChannelNames(taskHandle,numChannels,&channelNames));

	/*********************************************/
	// Read and format the calibration info
	/*********************************************/
	for (i = 0; i < numChannels; i++) {
		DAQmxErrChk (GetChanCalInfoSize(taskHandle,channelNames[i],&chanCalInfoSize));
		if( chanCalInfoSize > 0 )
			oneOrMoveChansWithValidCalInfo = TRUE;
		calInfoSize += chanCalInfoSize;
	}

	if(!oneOrMoveChansWithValidCalInfo) {
		printf("The channels in this task do not contain valid channel calibration information.\n");
		goto Error;
	}

	DAQmxNullChk (calInfoBuf = malloc (calInfoSize));
	strcpy(calInfoBuf,"");
	
	for (i = 0; i < numChannels; i++) {
		DAQmxErrChk (GetChanCalInfo(taskHandle,channelNames[i],calInfoBuf));
	}
	
	/*********************************************/
	// Write the calibration info to a file
	/*********************************************/
	file = fopen (filePath, "w");
	if(!file) {
	    printf("Could not open file: %s.\n",filePath);
	    goto Error;
	}
	
	if(!fwrite (calInfoBuf, sizeof(char), strlen(calInfoBuf)+1, file)) {
	    printf("Could not write to file: %s.\n",filePath);
	    goto Error;
	}
	
	printf("Successfully saved calibration info.\n");
	DAQmxErrChk (DAQmxSetAIChanCalApplyCalIfExp(taskHandle,"",0));

Error:
	if( calInfoBuf )
		free(calInfoBuf);
	if( file)
		fclose(file);
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

int32 GetChanCalInfoSize (TaskHandle taskHandle, const char *chanName, uInt32 *chanCalInfoSize)
{
	int32       error=0;
	int32		scaleType,numScalingVals,numCoeffVals,numVerifVals;
	uInt32		size=0;
	bool32      hasValidCalInfo;

	*chanCalInfoSize = 0;
	
	DAQmxErrChk (DAQmxGetAIChanCalHasValidCalInfo(taskHandle,chanName,&hasValidCalInfo));
	
	if(hasValidCalInfo) {
        DAQmxErrChk (DAQmxGetAIChanCalScaleType(taskHandle,chanName,&scaleType));
	    DAQmxErrChk (numScalingVals=DAQmxGetAIChanCalTableScaledVals(taskHandle,chanName,NULL,0));
        DAQmxErrChk (numCoeffVals=DAQmxGetAIChanCalPolyForwardCoeff(taskHandle,chanName,NULL,0));
		DAQmxErrChk (numVerifVals=DAQmxGetAIChanCalVerifRefVals(taskHandle,chanName,NULL,0));

		size += strlen (CHANNEL_NAME_FORMAT_STRING);
		size += strlen (chanName);
		size += strlen (CALIBRATION_ENABLED_FORMAT_STRING);
		size += strlen (YES);  // YES is longest string of YES and NO
		size += strlen (CALIBRATION_DATE_FORMAT_STRING);
		size += strlen ("xx/xx/xxxx");  // "xx/xx/xxxx" is longest date value
		size += strlen (EXPIRATION_DATE_FORMAT_STRING);
		size += strlen ("xx/xx/xxxx");  // "xx/xx/xxxx" is longest date value
		size += strlen (EMPTY_LINE);
		size += strlen (SCALING_DATA_TYPE_FORMAT_STRING);
		size += strlen (POLYNOMIAL_SCALE);  // POLYNOMIAL_SCALE is longest string of POLYNOMIAL_SCALE and TABLE_SCALE
		size += strlen (POLYNOMIAL_SCALING_DATA_COLUMN_LABELS); // POLYNOMIAL_SCALING_DATA_COLUMN_LABELS is the longest string of POLYNOMIAL_SCALING_DATA_COLUMN_LABELS and TABLE_SCALING_DATA_COLUMN_LABELS
		size += strlen (SCALING_DATA_FORMAT_STRING);
		if( scaleType == DAQmx_Val_Table )
			size += numScalingVals * (strlen ("-x.xxxxxxE+x") * 2);
		else
			size += numCoeffVals * (strlen ("-x.xxxxxxE+x") * 2);
		size += strlen (EMPTY_LINE);
		size += strlen (VERIFICATION_DATA_LABEL);
		size += strlen (VERIFICATION_DATA_COLUMN_LABELS);
		size += strlen (VERIFICATION_DATA_FORMAT_STRING);
		size += numVerifVals * (strlen ("-x.xxxxxxE+x") * 2);
		size += strlen (EMPTY_LINE);
		size += 64; // Safety cushion
	}
	
	*chanCalInfoSize = size;
	
Error:
	return error;
}
				
int32 GetChanCalInfo (TaskHandle taskHandle, const char *chanName, char *calInfoBuf)
{
	int32       error=0;
	float64		*refVals=0,*uncalibratedVals=0,*verifRefVals=0,*verifAcqVals=0;
	float64		*forwardCoeffVals=0,*reverseCoeffVals=0;
	uInt32      expMonth,expDay,expYear,calMonth,calDay,calYear;
	bool32      hasValidCalInfo,calEnabled;
	int32		numScaledVals,numVerifVals,numCoeffVals,scaleType;
	int			i;
	char		*bufPtr;
	
    DAQmxErrChk (DAQmxGetAIChanCalHasValidCalInfo(taskHandle,chanName,&hasValidCalInfo));
    
    if(hasValidCalInfo) {
        DAQmxErrChk (DAQmxGetAIChanCalEnableCal(taskHandle,chanName,&calEnabled));
        DAQmxErrChk (DAQmxGetAIChanCalCalDate(taskHandle,chanName,&calYear,&calMonth,&calDay,NULL,NULL));
        DAQmxErrChk (DAQmxGetAIChanCalExpDate(taskHandle,chanName,&expYear,&expMonth,&expDay,NULL,NULL));
        DAQmxErrChk (DAQmxGetAIChanCalScaleType(taskHandle,chanName,&scaleType));
        
        DAQmxErrChk (numScaledVals=DAQmxGetAIChanCalTableScaledVals(taskHandle,chanName,NULL,0));
        if( numScaledVals > 0 ) {
	        DAQmxNullChk (refVals=malloc(numScaledVals*sizeof(float64)));
	        DAQmxNullChk (uncalibratedVals=malloc(numScaledVals*sizeof(float64)));
	        DAQmxErrChk (DAQmxGetAIChanCalTableScaledVals(taskHandle,chanName,refVals,numScaledVals));
			DAQmxErrChk (DAQmxGetAIChanCalTablePreScaledVals(taskHandle,chanName,uncalibratedVals,numScaledVals));
        }
        
        DAQmxErrChk (numCoeffVals=DAQmxGetAIChanCalPolyForwardCoeff(taskHandle,chanName,NULL,0));
        if( numCoeffVals ) {
	        DAQmxNullChk (forwardCoeffVals=malloc(numCoeffVals*sizeof(float64)));
	        DAQmxNullChk (reverseCoeffVals=malloc(numCoeffVals*sizeof(float64)));
	        DAQmxErrChk (DAQmxGetAIChanCalPolyForwardCoeff(taskHandle,chanName,forwardCoeffVals,numCoeffVals));
			DAQmxErrChk (DAQmxGetAIChanCalPolyReverseCoeff(taskHandle,chanName,reverseCoeffVals,numCoeffVals));
        }
        
		DAQmxErrChk (numVerifVals=DAQmxGetAIChanCalVerifRefVals(taskHandle,chanName,NULL,0));
		if( numVerifVals ) {
	        DAQmxNullChk (verifRefVals=malloc(numVerifVals*sizeof(float64)));
	        DAQmxNullChk (verifAcqVals=malloc(numVerifVals*sizeof(float64)));
			DAQmxErrChk (DAQmxGetAIChanCalVerifRefVals(taskHandle,chanName,verifRefVals,numVerifVals));
			DAQmxErrChk (DAQmxGetAIChanCalVerifAcqVals(taskHandle,chanName,verifAcqVals,numVerifVals));
		}
		
		bufPtr = calInfoBuf+strlen(calInfoBuf);
		bufPtr += sprintf(bufPtr,CHANNEL_NAME_FORMAT_STRING,chanName);
		bufPtr += sprintf(bufPtr,CALIBRATION_ENABLED_FORMAT_STRING,calEnabled ? YES : NO);
		bufPtr += sprintf(bufPtr,CALIBRATION_DATE_FORMAT_STRING,(int)calMonth,(int)calDay,(int)calYear);
		bufPtr += sprintf(bufPtr,EXPIRATION_DATE_FORMAT_STRING,(int)expMonth,(int)expDay,(int)expYear);
		bufPtr += sprintf(bufPtr,EMPTY_LINE);
		bufPtr += sprintf(bufPtr,SCALING_DATA_TYPE_FORMAT_STRING,(scaleType==DAQmx_Val_Table) ? TABLE_SCALE : POLYNOMIAL_SCALE);
		
		if(scaleType==DAQmx_Val_Table)
			bufPtr += sprintf(bufPtr,TABLE_SCALING_DATA_COLUMN_LABELS);
		else
			bufPtr += sprintf(bufPtr,POLYNOMIAL_SCALING_DATA_COLUMN_LABELS);		
			
		if(scaleType==DAQmx_Val_Table)
			for (i = 0; i < numScaledVals; i++)
				bufPtr += sprintf(bufPtr,SCALING_DATA_FORMAT_STRING,refVals[i],uncalibratedVals[i]);
		else
			for (i = 0; i < numCoeffVals; i++)
				bufPtr += sprintf(bufPtr,SCALING_DATA_FORMAT_STRING,forwardCoeffVals[i],reverseCoeffVals[i]);
				
		bufPtr += sprintf(bufPtr,EMPTY_LINE);
		bufPtr += sprintf(bufPtr,VERIFICATION_DATA_LABEL);
		bufPtr += sprintf(bufPtr,VERIFICATION_DATA_COLUMN_LABELS);
		
		for (i = 0; i < numVerifVals; i++)
			bufPtr += sprintf(bufPtr,VERIFICATION_DATA_FORMAT_STRING,verifRefVals[i],verifAcqVals[i]);
			
		bufPtr += sprintf(bufPtr,EMPTY_LINE);
	}
	
Error:
	if( refVals )
		free(refVals);
	if( uncalibratedVals )
		free(uncalibratedVals);
	if( verifRefVals )
		free(verifRefVals);
	if( verifAcqVals )
		free(verifAcqVals);
	if( forwardCoeffVals )
		free(forwardCoeffVals);
	if( reverseCoeffVals)
		free(reverseCoeffVals);
	return error;
}

int32 GetChannelNames (TaskHandle task, uInt32 numChannels, char ***channelNames)
{
	int32 error = DAQmxSuccess;
	uInt32 i;

	*channelNames = (char **)calloc (numChannels, sizeof (char *));
	if (!(*channelNames))
		goto Error;
	for (i = 0; i < numChannels; i++)
	{
		DAQmxNullChk((*channelNames)[i] = (char *)malloc (255 + 1));  /// Assume channel names are no longer than 255 characters.
		DAQmxErrChk(DAQmxGetNthTaskChannel(task, i+1, (*channelNames)[i], 256));			
	}

Error:
	return error;
}

void FreeChannelNames (char **channelNames, uInt32 numChannels)
{
	uInt32 i;

	if (channelNames) {
		for (i = 0; i < numChannels; i++)
			if (channelNames[i])
				free (channelNames[i]);
		free (channelNames);
	}
}

