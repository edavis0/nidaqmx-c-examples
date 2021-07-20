/*********************************************************************
*
* ANSI C Example program:
*    ContAcqVoltage-ToFile-Compacted.c
*
* Example Category:
*    Professional
*
* Description:
*    This example demonstrates how to continuously acquire and save
*    compressed raw data into a binary file. You can use the Graph
*    Acquired Compacted Data example to read the scaled data back
*    from the file.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the rate of the acquisition Also set the Samples to Read
*       control. This will determine how many samples are read each
*       time the while loop iterates. Note: The rate should be at
*       least twice as fast as the maximum frequency component of the
*       signal being acquired.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*    4. Select the raw compression configuration.
*    5. Select the File Properties.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the compression attributes.
*    4. Set the rate for the sample clock. Additionally, define the
*       sample mode to be continuous.
*    5. Create a header and write it to the binary file.
*    6. Call the Start function to start the acquistion.
*    7. Read the raw data and write the data to the file in a loop
*       until the stop button is pressed or an error occurs.
*    8. Close the File.
*    9. Call the Clear Task function to clear the task.
*    10. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. For further connection information, refer
*    to your hardware reference manual.
*
* Limitations:
*    The example writes u8 data to disk as this is what the
*    decompaction examples expects. In addition, u8 writes result in
*    the highest to disk performance.
*
*    The example does not handle tasks containing channels from
*    multiple DSA devices with a compression type of "None". The
*    decompaction examples will not extract the samples from the file
*    correctly.
*
*    For cDAQ, the order of channels in the raw data is likely to not
*    be the same as the order specified in the task. The example does
*    not handle this case and the scaled data returned by the
*    decompaction example will not be correct.
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <NIDAQmx.h>
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static bool32 CreateDataFileHeader(char filePath[], TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsPerChan);
static long FindOutFileSize(char filePath[]);
static int WriteDataToDataFile(uInt16 *data, int32 numBytes);
static void CloseDataFile(void);
static bool32 CreateDataFileTaskEntry(TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsToRead);
static bool32 CreateDataFileChannelEntry(TaskHandle taskHandle, int idx);
static int CalculateReadBlockSize(TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsPerChan);

static FILE	*gFileHandle=NULL;
static char	gHeader[2000];
static uInt32 gReadBlockSize;
static uInt16 *data=NULL;
static uInt32 numChannels;

static char	hiddenChanMsg[]="Hidden channels were detected in the task. However, this example does not handle these channels correctly. "
							"For example, cold-junction compensation channels for thermocouples may be added as hidden channels. "
							"In addition, some hardware has channel order restrictions that may result in hidden channels being added to the task. "
							"You can correct the problem by explicitly creating all channels that are currently added as hidden channels.";
static char resolutionMsg[]="Compressed Sample Size exceeds the Resolution of the channel. Configure the Compressed Sample Size to be "
							"less than or equal to the channel Resolution.";

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
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	DAQmxErrChk (DAQmxSetAIRawDataCompressionType(taskHandle,"",DAQmx_Val_LosslessPacking));
	DAQmxErrChk (DAQmxSetAILossyLSBRemovalCompressedSampSize(taskHandle,"",12));
	DAQmxErrChk (DAQmxGetTaskNumChans(taskHandle,&numChannels));

	if( (data=malloc(1000*numChannels*sizeof(uInt16)))==NULL ) {
		puts("Not enough memory");
		goto Error;
	}

	if( !CreateDataFileHeader("C:\\stream.cfg",taskHandle,numChannels,1000) )
		goto Error;

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,1000,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Streaming samples continuously. Press Enter to interrupt\n");
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
		CloseDataFile();
	}
	if( data )
		free(data);
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	puts("End of program, press Enter key to quit");
	getchar();
	return 0;
}

static bool32 CreateDataFileHeader(char filePath[], TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsPerChan)
{
	uInt32  i=0,size;
	char	*s,ch;

	if( filePath==NULL || *filePath=='\0' || (gFileHandle=fopen(filePath,"w"))==NULL )
		goto Error;
	strcpy(gHeader,"[DAQCompressedBinaryFile]\nVersion=1.0.0\nHeaderSize=0deadBEEF0\nNumberOfTasks=1\n");
	if( !CreateDataFileTaskEntry(taskHandle,numChannels,sampsPerChan) )
		return FALSE;
	while( i<numChannels )
		CreateDataFileChannelEntry(taskHandle,i++);
	strcat(gHeader,"[BinaryData]\nBegin=Here\n");
	fputs(gHeader,gFileHandle);
	fclose(gFileHandle);
	size = FindOutFileSize(filePath);
	if( (gFileHandle=fopen(filePath,"r+"))==NULL )
		goto Error;
	// Write the actual header size
	fread(gHeader,1,2*size,gFileHandle);
	if( (s=strstr(gHeader,"0deadBEEF0")) ) {
		ch = s[10];
		sprintf(s,"%010d",(int)size);
		s[10] = ch;
	}
	fseek(gFileHandle,0,SEEK_SET);
	fwrite(gHeader,1,strlen(gHeader),gFileHandle);
	fclose(gFileHandle);

	if( (gFileHandle=fopen(filePath,"ab"))==NULL )
		goto Error;
	return TRUE;

Error:
	puts("Error: There was a problem writing to the file.");
	return FALSE;
}


static long FindOutFileSize(char filePath[])
{
	FILE	*f;
	long	size=0;

	if( (f=fopen(filePath,"rb")) && fseek(f,0,SEEK_END)==0 ) {
		size = ftell(f);
		fclose(f);
	}
	return size;
}

static int WriteDataToDataFile(uInt16 *data, int32 numBytes)
{
	if( gFileHandle )
		fwrite(data,sizeof(uInt8),numBytes,gFileHandle);
	return 0;
}

static void CloseDataFile(void)
{
	if( gFileHandle )
		fclose(gFileHandle);
}

static bool32 CreateDataFileTaskEntry(TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsToRead)
{
	char    taskName[1000];
	bool32  success;

	strcat(gHeader+strlen(gHeader),"[Task0]\n");
	DAQmxGetTaskName(taskHandle,taskName,1000);
	sprintf(gHeader+strlen(gHeader),"Name=%s\n",taskName);
	sprintf(gHeader+strlen(gHeader),"NumberOfChannels=%d\n",(int)numChannels);
	sprintf(gHeader+strlen(gHeader),"ReadBlockSize=%d\n",(int)sampsToRead);
	success = CalculateReadBlockSize(taskHandle,numChannels,sampsToRead);
	sprintf(gHeader+strlen(gHeader),"ReadBlockSizeInBytes=%d\n",(int)gReadBlockSize);
	return success;
}

static bool32 CreateDataFileChannelEntry(TaskHandle taskHandle, int idx)
{
	char    channelName[1000];
	float64 f64,resolution,coeffs[1000];
	uInt32  u32,rawSampSize;
	int32   i32,numCoeffs;

	DAQmxGetNthTaskChannel(taskHandle,idx+1,channelName,1000);
	sprintf(gHeader+strlen(gHeader),"[Task0Channel%d]\nName=%s\n",idx,channelName);
	DAQmxGetAIResolution(taskHandle,channelName,&resolution);
	sprintf(gHeader+strlen(gHeader),"RawSampleResolution=%u\n",(unsigned)resolution);
	DAQmxGetAIRawSampSize(taskHandle,channelName,&rawSampSize);
	sprintf(gHeader+strlen(gHeader),"RawSampleSizeInBits=%u\n",(unsigned)rawSampSize);
	DAQmxGetAIRawSampJustification(taskHandle,channelName,&i32);
	sprintf(gHeader+strlen(gHeader),"RawSampleJustification=%s\n",i32==DAQmx_Val_LeftJustified?"Left":"Right");
	DAQmxGetAIMin(taskHandle,channelName,&f64);
	sprintf(gHeader+strlen(gHeader),"SignedNumber=%s\n",f64<0?"TRUE":"FALSE");
	DAQmxGetAIRawDataCompressionType(taskHandle,channelName,&i32);
	switch( i32 ) {
		case DAQmx_Val_LosslessPacking:
			strcat(gHeader,"CompressionType=LosslessPacking\n");
			DAQmxGetAIResolution(taskHandle,channelName,&f64);
			u32 = (uInt32)f64;
			break;
		case DAQmx_Val_LossyLSBRemoval:
			strcat(gHeader,"CompressionType=LossyLSBRemoval\n");
			DAQmxGetAILossyLSBRemovalCompressedSampSize(taskHandle,channelName,&u32);
			break;
		default:
			strcat(gHeader,"CompressionType=None\n");
			u32 = rawSampSize;
			break;
	}
	sprintf(gHeader+strlen(gHeader),"CompressedSampleSizeInBits=%u\n",(unsigned)u32);
	sprintf(gHeader+strlen(gHeader),"CompressionByteOrder=%s\n",rawSampSize==resolution||i32==DAQmx_Val_None?"LittleEndian":"BigEndian");

	numCoeffs = DAQmxGetAIDevScalingCoeff(taskHandle,channelName,NULL,0);
	if( numCoeffs>1000 )
		numCoeffs = 1000;
	DAQmxGetAIDevScalingCoeff(taskHandle,channelName,coeffs,numCoeffs);
	strcat(gHeader,"PolynomialScalingCoeffs=");
	for(i32=0;i32<numCoeffs;++i32)
		sprintf(gHeader+strlen(gHeader),"%0.15lE;",coeffs[i32]);
	strcat(gHeader,"\n");

	return FALSE;
}

static int CalculateReadBlockSize(TaskHandle taskHandle, uInt32 numChannels, uInt32 sampsPerChan)
{
	uInt32  rawDataWidth,rawSampSize=1,val;
	int32   compType;
	float64 resolution;

	if( DAQmxGetReadRawDataWidth(taskHandle,&rawDataWidth)==DAQmxErrorCompressedSampSizeExceedsResolution
	 || DAQmxGetAIRawSampSize(taskHandle,"",&rawSampSize)==DAQmxErrorCompressedSampSizeExceedsResolution
	 || DAQmxGetAIRawDataCompressionType(taskHandle,"",&compType)==DAQmxErrorCompressedSampSizeExceedsResolution ) {
		printf("Error: %s\n",resolutionMsg);
		return 0;
	}
	switch( compType ) {
		case DAQmx_Val_LosslessPacking:
			DAQmxGetAIResolution(taskHandle,"",&resolution);
			val = (uInt32)resolution;
			break;
		case DAQmx_Val_LossyLSBRemoval:
			DAQmxGetAILossyLSBRemovalCompressedSampSize(taskHandle,"",&val);
			break;
		default:
			val = rawSampSize;
			break;
	}
	rawDataWidth *= 8;	// Multiply by number of bits
	gReadBlockSize = (uInt32)ceil(val*rawDataWidth*sampsPerChan/rawSampSize/8);
	// Detect hidden channels
	if( rawDataWidth%rawSampSize && floor(rawDataWidth/rawSampSize)!=numChannels ) {
		printf("Error: %s\n",hiddenChanMsg);
		return 0;
	}
	return 1;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalRead=0;
	int32       read=0;

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadRaw(taskHandle,1000,10.0,data,1000*numChannels*sizeof(uInt16),&read,NULL,NULL));
	if( read>0 ) {
		WriteDataToDataFile(data,gReadBlockSize);
		printf("Acquired %d samples. Total %d\r",(int)read,(int)(totalRead+=read));
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
