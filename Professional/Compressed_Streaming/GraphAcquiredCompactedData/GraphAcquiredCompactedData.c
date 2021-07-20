/*********************************************************************
*
* ANSI C Example program:
*    GraphAcquiredCompactedData.c
*
* Example Category:
*    Professional
*
* Description:
*    This example reads from binary files created by other DAQmx
*    Compressed Streaming examples. It first reads the header from
*    the file and uses that information to format and scale the data.
*
* Instructions for Running:
*    1. Select your Data File.
*
* Steps:
*    1. Parse header information in file and check validity.
*    2. Check if header is valid and if configuration is supported.
*    3. Calculate values used to decompress the data.
*    4. Read data blocks from file and convert to samples.
*    5. Scale decompressed samples.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    No I/O connections are needed.
*
* Limitations:
*    The example reads u8 data from disk. If the data was written in
*    a different format, the example will not decompress the data
*    correctly and will return invalid samples.
*
*    The example does not handle tasks with channels with different
*    raw or compressed sample formats. The example will detect this
*    case and return an error.
*
*    The example does not handle files containing uncrompressed data
*    logged from a task with channels on multiple DSA devices. The
*    example will not detect this case and the scaled data returned
*    will not be correct.
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
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

typedef enum {
	BigEndian,
	LittleEndian
} ByteOrder;

typedef struct _ScaleCoeff {
	double	scalingCoeff;
	struct _ScaleCoeff	*next;
} ScaleCoeff;

typedef struct _ChannelInfo {
	char		name[100];
	uInt32		rawSampleResolution;
	uInt32		rawSampleSizeInBits;
	uInt32		rawSampleJustification;
	bool32		signedNumber;
	uInt32		compressionType;
	uInt32		compressedSampleSizeInBits;
	ByteOrder	compressionByteOrder;
	ScaleCoeff	*polynomialScalingCoeffChain;
	struct _ChannelInfo	*next;
} ChannelInfo;

typedef struct {
	char		version[100];
	uInt32		headerSize;
	char		taskName[500];
	uInt32		numberOfChannels;
	uInt32		readBlockSize;
	uInt32		readBlockSizeInBytes;
	ChannelInfo	*channelInfoChain;
} DataFileInfo;

static int ReadScaleAndPlotDataFileData(const char filePath[]);
static DataFileInfo *ParseDataFileHeader(const char filePath[]);
static uInt8 *ReadDataFileData(DataFileInfo *info, const char filePath[], uInt32 *numBytes);
static int PlotScaledData(float64 *data[100], uInt32 numChannels, uInt32 numSamples);
static int DecodeDataWithoutPacking(DataFileInfo *info, uInt8 *rawData, uInt32 numBytes, float64 *data[100]);
static int DecodeDataWithPacking(DataFileInfo *info, uInt8 *rawData, uInt32 numBytes, float64 *data[100]);
static void FreeDataFileInfoContent(DataFileInfo *info);
static long FindOutFileSize(const char filePath[]);

int main(void)
{
	if( !ReadScaleAndPlotDataFileData("c:\\stream.cfg") )
		puts("Error: There was a problem reading from the file or the file format is invalid.");
	puts("\nEnd of program, press Enter key to quit");
	getchar();
	return 0;
}

static int ReadScaleAndPlotDataFileData(const char filePath[])
{
	DataFileInfo    *info;
	uInt8           *rawData=NULL;
	uInt32          numBytes,numSamples=0,size,iChan=0;
	float64         *data[100],**dataPtr=&data[0];

	memset(data,0,sizeof(data));
	puts(filePath);
	if( (info=ParseDataFileHeader(filePath)) && (rawData=ReadDataFileData(info,filePath,&numBytes)) ) {
		size = numBytes/info->numberOfChannels;
		if( info->channelInfoChain->compressionType==DAQmx_Val_LossyLSBRemoval )
			size = (uInt32)((double)size*8.0/info->channelInfoChain->compressedSampleSizeInBits) + 20;
		for(;iChan<info->numberOfChannels;++iChan)
			if( (data[iChan]=(float64*)malloc(sizeof(float64)*size))==NULL )
				goto Error;
		printf("%d channel(s)\n",(int)info->numberOfChannels);
		if (info->channelInfoChain->compressionType==DAQmx_Val_None ||
				(info->channelInfoChain->compressionByteOrder==LittleEndian && info->channelInfoChain->compressedSampleSizeInBits%8==0)) 
		{
			numSamples = DecodeDataWithoutPacking(info,rawData,numBytes,data);
		}
		else
		{
			numSamples = DecodeDataWithPacking(info,rawData,numBytes,data);
		}
		PlotScaledData(data,info->numberOfChannels,numSamples);
	}

Error:
	if( rawData )
		free(rawData);
	while( *dataPtr )
		free(*dataPtr++);
	if( info )
		FreeDataFileInfoContent(info);
	return numSamples>0;
}

static DataFileInfo *ParseDataFileHeader(const char filePath[])
{
	FILE    *f;
	uInt32  numTasks,i=0,channelNum;
	char    justificationBuff[100],signedBuff[100],compBuff[100],byteOrderBuff[100],coeffBuff[1000],*coeffStr;
	static DataFileInfo info;
	ChannelInfo         *chan,**chanInfoChainPtr=&info.channelInfoChain;
	ScaleCoeff          *coeff,**coeffChainPtr;

	info.channelInfoChain = NULL;
	if( (f=fopen(filePath,"r"))==NULL
	 || fscanf(f,"[DAQCompressedBinaryFile]\n")<0
	 || fscanf(f,"Version=%s\n",info.version)<0
	 || fscanf(f,"HeaderSize=%ld\n",&info.headerSize)<0
	 || fscanf(f,"NumberOfTasks=%ld\n",&numTasks)<0
	 || fscanf(f,"[Task0]\n")<0
	 || fscanf(f,"Name=%s\n",info.taskName)<0
	 || fscanf(f,"NumberOfChannels=%ld\n",&info.numberOfChannels)<0
	 || fscanf(f,"ReadBlockSize=%ld\n",&info.readBlockSize)<0
	 || fscanf(f,"ReadBlockSizeInBytes=%ld\n",&info.readBlockSizeInBytes)<0 )
		return NULL;
	if( strcmp(info.version,"1.0.0") || info.numberOfChannels<1 )
		return NULL;

	for(;i<info.numberOfChannels;++i) {
		if( (chan=(ChannelInfo*)malloc(sizeof(ChannelInfo)))==NULL )
			return NULL;
		chan->next = NULL;
		chan->polynomialScalingCoeffChain = NULL;
		*chanInfoChainPtr = chan;
		chanInfoChainPtr = &chan->next;
		if( fscanf(f,"[Task0Channel%ld]\n",&channelNum)<0
		 || fscanf(f,"Name=%s\n",chan->name)<0
		 || fscanf(f,"RawSampleResolution=%ld\n",&chan->rawSampleResolution)<0
		 || fscanf(f,"RawSampleSizeInBits=%ld\n",&chan->rawSampleSizeInBits)<0
		 || fscanf(f,"RawSampleJustification=%s\n",justificationBuff)<0
		 || fscanf(f,"SignedNumber=%s\n",signedBuff)<0
		 || fscanf(f,"CompressionType=%s\n",compBuff)<0
		 || fscanf(f,"CompressedSampleSizeInBits=%ld\n",&chan->compressedSampleSizeInBits)<0
		 || fscanf(f,"CompressionByteOrder=%s\n",byteOrderBuff)<0
		 || fscanf(f,"PolynomialScalingCoeffs=%s\n",coeffBuff)<0 ) {
			FreeDataFileInfoContent(&info);
			return NULL;
		}

		if( channelNum!=i || *coeffBuff=='\0' || *coeffBuff==';' ) {
			FreeDataFileInfoContent(&info);
			return NULL;
		}
		chan->rawSampleJustification = strcmp(justificationBuff,"Left")?DAQmx_Val_RightJustified:DAQmx_Val_LeftJustified;
		chan->signedNumber = strcmp(signedBuff,"TRUE")==0;
		chan->compressionType = strcmp(compBuff,"LosslessPacking")?(strcmp(compBuff,"LossyLSBRemoval")?DAQmx_Val_None:DAQmx_Val_LossyLSBRemoval):DAQmx_Val_LosslessPacking;
		chan->compressionByteOrder = strcmp(byteOrderBuff,"LittleEndian")?BigEndian:LittleEndian;

		coeffChainPtr = &chan->polynomialScalingCoeffChain;
		coeffStr = strtok(coeffBuff,";");
		while( coeffStr ) {
			coeff = (ScaleCoeff*)malloc(sizeof(ScaleCoeff));
			coeff->next = NULL;
			*coeffChainPtr = coeff;
			coeffChainPtr = &coeff->next;
			sscanf(coeffStr,"%le",&coeff->scalingCoeff);
			coeffStr = strtok(NULL,";");
		}
	}

	if( fscanf(f,"[BinaryData]\n")<0
	 || fscanf(f,"Begin=Here\n")<0 ) {
		FreeDataFileInfoContent(&info);
		return NULL;
	}

	fclose(f);
	return &info;
}

static uInt8 *ReadDataFileData(DataFileInfo *info, const char filePath[], uInt32 *numBytes)
{
	FILE	*f;
	long	size=FindOutFileSize(filePath),read;
	uInt8	*data=NULL;

	// open binary file, skip headerSize bytes, start crackin'
	if( (f=fopen(filePath,"rb"))==NULL )
		return NULL;
	if( fseek(f,info->headerSize,SEEK_SET) )
		goto Error;

	if( (data=(uInt8*)malloc(size-=info->headerSize))==NULL )
		goto Error;
	if( (read=fread(data,1,size,f))<size ) {
		free(data);
		data = NULL;
		goto Error;
	}
	if( numBytes )
		*numBytes = size;

Error:
	if( f )
		fclose(f);
	return data;
}

static int PlotScaledData(float64 *data[100], uInt32 numChannels, uInt32 numSamples)
{
	uInt32	iChan=0,iSamp=0;
	double	total;

	if( numChannels<=0 || numSamples<=0 )
		return 0;
	// Plot your data here
	for(;iChan<numChannels;++iChan,printf("Channel: %d\tNumber of Samples: %ld\t\tAverage: %f\n",(int)iChan,numSamples,total/numSamples))
		for(total=0.0,iSamp=0;iSamp<numSamples;total+=data[iChan][iSamp++]);
	return 1;
}

static int DecodeDataWithoutPacking(DataFileInfo *info, uInt8 *rawData, uInt32 numBytes, float64 *data[100])
{
	uInt32	numSamples=0,iSamp,iChan,iByte;
	long	val;

	// For each block of data
	while( numBytes ) {
		// For each sample
		for(iSamp=0;iSamp<info->readBlockSize;++iSamp) {
			ChannelInfo	*chan=info->channelInfoChain;

			// For each channel
			for(iChan=0;iChan<info->numberOfChannels;++iChan,chan=chan->next) {
				uInt32			size,bytes,signmask,extendmask;
				ScaleCoeff		*coeff;
				double			polynom=1.0;

				if( chan==NULL )
					goto Error;

				size = chan->rawSampleSizeInBits;
				bytes = size / 8;
				signmask = 1 << (size-1);
				extendmask = 0xFFFFFFFF - (1 << (32-size)) + 1;

				// Read the bytes
				val = 0;

				if( chan->compressionByteOrder==LittleEndian )
					for(iByte=0;iByte<bytes&&numBytes;++iByte,--numBytes)
						val += *rawData++ << iByte * 8;
				else
					for(iByte=0;iByte<bytes&&numBytes;++iByte,--numBytes)
						val = (val<<8) + *rawData++;

				// Extend sign if needed
				if( chan->signedNumber && size<32 && val&signmask )
					val |= extendmask;

				// Scale
				for(data[iChan][numSamples]=0.0,coeff=chan->polynomialScalingCoeffChain;coeff;coeff=coeff->next,polynom*=val)
					data[iChan][numSamples] += coeff->scalingCoeff * polynom;
			}
			++numSamples;
		}
	}

Error:
	return numSamples;
}

static int DecodeDataWithPacking(DataFileInfo *info, uInt8 *rawData, uInt32 numBytes, float64 *data[100])
{
	uInt32	numSamples=0,iSamp,iChan;
	long	val;

	// For each block of data
	while( numBytes ) {
		int		bitPos=0;
		uInt32	ch=*rawData++,byteCount=1;

		--numBytes;
		// For each sample
		for(iSamp=0;iSamp<info->readBlockSize;++iSamp) {
			ChannelInfo	*chan=info->channelInfoChain;

			// For each channel
			for(iChan=0;iChan<info->numberOfChannels;++iChan,chan=chan->next) {
				uInt32			size,lsb,realsize,signmask,extendmask,bitCount=0;
				ScaleCoeff		*coeff;
				double			polynom=1.0;

				if( chan==NULL )
					goto Error;

				size = chan->compressedSampleSizeInBits;
				lsb = (chan->rawSampleJustification==DAQmx_Val_LeftJustified?chan->rawSampleSizeInBits:chan->rawSampleResolution) - size;
				realsize = size + lsb;
				signmask = 1 << (realsize-1);
				extendmask = 0xFFFFFFFF - (1 << realsize) + 1;
				val = 0;

				while( bitCount<size ) {
					// Shift left
					val <<= 1;

					// Add in the next bit
					val += (ch >> (8-bitPos-1)) & 1;
					// Get next byte if necessary
					++bitCount;
					++bitPos;
					if( bitPos==8 && byteCount<info->readBlockSizeInBytes ) {
						bitPos = 0;
						if( numBytes==0 )
							goto Error;
						else {
							ch = *rawData++;
							--numBytes;
							++byteCount;
						}
					}
				}

				// Scale back in the lost bits
				val <<= lsb;

				// Extend sign if needed
				if( chan->signedNumber && realsize<32 && val&signmask )
					val |= extendmask;

				// Scale
				for(data[iChan][numSamples]=0.0,coeff=chan->polynomialScalingCoeffChain;coeff;coeff=coeff->next,polynom*=val)
					data[iChan][numSamples] += coeff->scalingCoeff * polynom;
			}
			++numSamples;
		}
	}

Error:
	return numSamples;
}

static void FreeDataFileInfoContent(DataFileInfo *info)
{
	ChannelInfo	*chain=info->channelInfoChain,*next;
	ScaleCoeff	*coeff,*nextCoeff;

	while( chain ) {
		next = chain->next;
		coeff = chain->polynomialScalingCoeffChain;
		while( coeff ) {
			nextCoeff = coeff->next;
			free(coeff);
			coeff = nextCoeff;
		}
		free(chain);
		chain = next;
	}
}

static long FindOutFileSize(const char filePath[])
{
	FILE	*f;
	long	size=0;

	if( (f=fopen(filePath,"rb")) && fseek(f,0,SEEK_END)==0 ) {
		size = ftell(f);
		fclose(f);
	}
	return size;
}
