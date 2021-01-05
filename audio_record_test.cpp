#include<stdio.h>
#include<stdlib.h>
 
 
#if 1
#include <media/AudioRecord.h>
 using namespace android;
//==============================================
//	Audio Record Defination
//==============================================
 
static pthread_t		g_AudioRecordThread;
static pthread_t *	g_AudioRecordThreadPtr = NULL;
 
volatile bool 	g_bQuitAudioRecordThread = false;
volatile int 	g_iInSampleTime = 0;
int 	g_iNotificationPeriodInFrames = 8000/10; 
// g_iNotificationPeriodInFrames should be change when sample rate changes.
 
static void *	AudioRecordThread( void *inArg );
//void StartAudioRecordThread();
//void StopAudioRecordThread();
 
void AudioRecordCallback(int event, void* user, void *info)
{
	if(event == android::AudioRecord::EVENT_NEW_POS)
	{
		g_iInSampleTime += g_iNotificationPeriodInFrames;		
		//if(g_iInSampleTime > g_iNotificationPeriodInFrames*100) 
		//		g_bQuitAudioRecordThread = true;
	}
	else if (event == android::AudioRecord::EVENT_MORE_DATA)
	{		
		android::AudioRecord::Buffer* pBuff = (android::AudioRecord::Buffer*)info;
        	pBuff->size = 0;
	}
	else if (event == android::AudioRecord::EVENT_OVERRUN)
	{
		//LOGE(" EVENT_OVERRUN \n");
		printf("[%d%s]EVENT_OVERRUN \n",__LINE__,__FUNCTION__);
	}
}
 
static void *	AudioRecordThread( void *inArg )
{
	uint64_t  						inHostTime 				= 0;
	void *								inBuffer 					= NULL; 
	audio_source_t 				inputSource 			= AUDIO_SOURCE_MIC;
	audio_format_t 				audioFormat 			= AUDIO_FORMAT_PCM_16_BIT;	
	audio_channel_mask_t 	channelConfig 		= AUDIO_CHANNEL_IN_MONO; //AUDIO_CHANNEL_IN_STEREO;	
	int 									bufferSizeInBytes = 1600;
	int 									sampleRateInHz 		= 16000;	
	android::AudioRecord *	pAudioRecord 		= NULL;
	FILE * 									g_pAudioRecordFile 		= NULL;
	//char 										strAudioFile[] 				= "/mnt/sdcard/external_sd/AudioRecordFile.pcm";
	char 										strAudioFile[] 				= "./AudioRecordFile.pcm";
 
	int iNbChannels 		= 2;	// 1 channel for mono, 2 channel for streo
	int iBytesPerSample = 2; 	// 16bits pcm, 2Bytes
	int frameSize 			= 0;	// frameSize = iNbChannels * iBytesPerSample
    //int minFrameCount 	= 0;	// get from AudroRecord object
    size_t minFrameCount 	= 0;	// get from AudroRecord object
	int iWriteDataCount = 0;	// how many data are there write to file
	printf("[%d%s]thread enter ok!\n",__LINE__,__FUNCTION__);
 
	
	String16 pack_name("wang_test");
	
 
	printf("[%d%s]thread enter ok!\n",__LINE__,__FUNCTION__);
	
#if 1
	// log the thread id for debug info
	//LOGD("%s  Thread ID  = %d  \n", __FUNCTION__,  pthread_self());  
	printf("[%d%s]  Thread ID  = %ld  \n", __LINE__,__FUNCTION__,  pthread_self());
	g_iInSampleTime = 0;
	g_pAudioRecordFile = fopen(strAudioFile, "wb+");	
	if(g_pAudioRecordFile == NULL)
	{
		printf("open file erro !\n");
	}
	
	iNbChannels = (channelConfig == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
	frameSize 	= iNbChannels * iBytesPerSample;	
 
	android::status_t 	status = android::AudioRecord::getMinFrameCount(
		&minFrameCount, sampleRateInHz, audioFormat, channelConfig);	
	
	if(status != android::NO_ERROR)
	{
		//LOGE("%s  AudioRecord.getMinFrameCount fail \n", __FUNCTION__);
		printf("[%d%s]AudioRecord.getMinFrameCount fail \n",__LINE__,__FUNCTION__);
		goto exit ;
	}
	
	//LOGE("sampleRateInHz = %d minFrameCount = %d iNbChannels = %d frameSize = %d ", 
	//	sampleRateInHz, minFrameCount, iNbChannels, frameSize);	
	printf("[%d%s]sampleRateInHz = %d minFrameCount = %d iNbChannels = %d frameSize = %d \n", 
		__LINE__,__FUNCTION__,sampleRateInHz, minFrameCount, iNbChannels, frameSize);
	
	bufferSizeInBytes = minFrameCount * frameSize;
	
	inBuffer = malloc(bufferSizeInBytes); 
	if(inBuffer == NULL)
	{		
		//LOGE("%s  alloc mem failed \n", __FUNCTION__);		
		printf("[%d%s]  alloc mem failed \n",__LINE__, __FUNCTION__);
		goto exit ; 
	}
 
	g_iNotificationPeriodInFrames = sampleRateInHz/10;	
	
	
	
	pAudioRecord  = new android::AudioRecord(pack_name);
	if(NULL == pAudioRecord)
	{
		//LOGE(" create native AudioRecord failed! ");
		printf(" [%d%s] create native AudioRecord failed! \n",__LINE__,__FUNCTION__);
		goto exit;
	}
	
	pAudioRecord->set( inputSource,
                                    sampleRateInHz,
                                    audioFormat,
                                    channelConfig,
                                    0,
                                    AudioRecordCallback,
                                    NULL,
                                    0,
                                    true); 
 
	if(pAudioRecord->initCheck() != android::NO_ERROR)  
	{
		//LOGE("AudioTrack initCheck error!");
		printf("[%d%s]AudioTrack initCheck error!\n",__LINE__,__FUNCTION__);
		goto exit;
	}
 
	if(pAudioRecord->setPositionUpdatePeriod(g_iNotificationPeriodInFrames) != android::NO_ERROR)
	{
		//LOGE("AudioTrack setPositionUpdatePeriod error!");
		printf("[%d%s]AudioTrack setPositionUpdatePeriod error!\n",__LINE__,__FUNCTION__);
		goto exit;
	}
	
	if(pAudioRecord->start()!= android::NO_ERROR)
	{
		//LOGE("AudioTrack start error!");
		printf("[%d%s]AudioTrack start error!\n",__LINE__,__FUNCTION__);
		goto exit;
	}	
	
	while (!g_bQuitAudioRecordThread)
	{
		//inHostTime = UpTicks();
		int readLen = pAudioRecord->read(inBuffer, bufferSizeInBytes);		
		int writeResult = -1;
		
		if(readLen > 0) 
		{
			iWriteDataCount += readLen;
			if(NULL != g_pAudioRecordFile)
			{
				writeResult = fwrite(inBuffer, 1, readLen, g_pAudioRecordFile);				
				if(writeResult < readLen)
				{
					//LOGE("Write Audio Record Stream error");
					printf("[%d%s]Write Audio Record Stream error\n",__LINE__,__FUNCTION__);
				}
			}			
 
			// write PCM data to file or other stream,implement it yourself
			//writeResult = WriteAudioData(					
			//		g_iInSampleTime, 
			//		inHostTime, 
			//		inBuffer, 
			//		readLen);
			//LOGD("readLen = %d  writeResult = %d  iWriteDataCount = %d", readLen, writeResult, iWriteDataCount);			
		}
		else 
		{
			//LOGE("pAudioRecord->read  readLen = 0");
			printf("[%d%s]pAudioRecord->read  readLen = 0\n",__LINE__,__FUNCTION__);
		}
	}
 
		
	exit:
 
	if(NULL != g_pAudioRecordFile)
	{
		fflush(g_pAudioRecordFile);
		fclose(g_pAudioRecordFile);
		g_pAudioRecordFile = NULL;
	}
 
	if(pAudioRecord)
	{
		pAudioRecord->stop();
		//delete pAudioRecord;
		pAudioRecord == NULL;
	}
 
	if(inBuffer)
	{
		free(inBuffer);
		inBuffer = NULL;
	}
#endif
	//LOGD("%s  Thread ID  = %d  quit\n", __FUNCTION__,  pthread_self());
	printf("[%d%s] Thread ID  = %d  quit\n", __LINE__,__FUNCTION__,  pthread_self());
	return NULL;
	
}
 
int main()
{
	printf("hello world! \n");	
	pthread_t record_pid ;
	if(pthread_create(&record_pid,NULL,AudioRecordThread,NULL)<0)
	{
		printf("%d%s pthread create erro !\n",__LINE__,__FUNCTION__);
	}
	
	while(1)
	{
		
	}
	return 0;
}
#else
#include<stdio.h>
#include<stdlib.h>
int main()
{
	printf("hello world! \n");
	return 0;
}
#endif
