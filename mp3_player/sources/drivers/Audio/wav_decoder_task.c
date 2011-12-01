#include "SafeRTOS/SafeRTOS_API.h"
#include "priorities.h"
#include "file_management.h"
#include "audio_codec.h"
#include "sound_player_task.h"
#include "wav_decoder_task.h"
static unsigned long WAVDecoderTaskStack[256];
static xTaskHandle pvCreatedTask ;

static unsigned long Flags = MP3_STOP;
///
FRESULT
OpenWavFile(FIL *psFileObject, char *pcFileName, SongInfoHeader *pSongInfoHeader);
//
void StartWAVPlay(void)
{
  //vTaskSuspendScheduler();
    Flags= MP3_START;
    // xTaskResumeScheduler();
    xTaskResume(pvCreatedTask);
   
}
void PauseWAVPlay(void)
{
  //vTaskSuspendScheduler();
    Flags = MP3_PAUSE;
    //xTaskResumeScheduler();
}
void ContinueWAVPlay(void)
{
  //vTaskSuspendScheduler();
    Flags = MP3_CONTINUE;
     //xTaskResumeScheduler();
    xTaskResume(pvCreatedTask);
   
}
void StopWAVPlay(void)
{
  if(Flags == MP3_PAUSE)
  {
     xTaskResume(pvCreatedTask);
  }
  vTaskSuspendScheduler();
  Flags = MP3_STOP;
  xTaskResumeScheduler();
}
unsigned long GetStatusWAVPlay(void)
{
  return Flags;
}
static unsigned char tmp[64];
static void
WAVDecoderTask(void *pvParameters)
{
    unsigned short usCount;
    unsigned char Convert8Bit;
    FRESULT Result;
    Flags = MP3_STOP;
    while(1)
    {
        if(Flags == MP3_STOP)
        {
            f_close(&sSongFileObject);
            //Wait signal;
            xTaskSuspend(NULL);
        }
        else if(Flags == MP3_START)
        {
            f_close(&sSongFileObject);
            Result = OpenWavFile(&sSongFileObject,sFilePath,&sSongInfoHeader);
            if(Result != FR_OK)
            {
                Flags = MP3_STOP;
            }
            if(sSongInfoHeader.usBitsPerSample == 8)
              Convert8Bit=1;
            else
              Convert8Bit=0;
            setupAudioCodecForSong(&sSongInfoHeader);
            initBuffer(ulMaxBufferSize);//
            Flags = MP3_CONTINUE;
        }
        else if(Flags == MP3_CONTINUE)
        {
            if(WriteWAVToBuffer(&sSongFileObject,tmp, usCount, 0, Convert8Bit))
            {
              vTaskSuspendScheduler();
               Flags = MP3_STOP;
               xTaskResumeScheduler();
            }
            if(Flags != MP3_STOP)
              WaitBufferSignal(10);
        }else{
            //Wait signal
            xTaskSuspend(NULL);
        }
    }
}
/**
* Intialize, create all components for Sound Player Task
*/
char initWAVDecoderTask(void){
  //
  // Create the task.
  //
  if(xTaskCreate(WAVDecoderTask, (signed portCHAR *)"wav",
                   (signed portCHAR *)WAVDecoderTaskStack,
                   sizeof(WAVDecoderTaskStack), NULL, PRIORITY_SOUND_PLAYER_TASK+1,
                   &pvCreatedTask) != pdPASS)
  {
    return(1);//Fail
  }
  
  return 0;//Success
}
//******************************************************************************
//
// Basic wav file RIFF header information used to open and read a wav file.
//
//******************************************************************************
#define RIFF_CHUNK_ID_RIFF      0x46464952
#define RIFF_CHUNK_ID_FMT       0x20746d66
#define RIFF_CHUNK_ID_DATA      0x61746164

#define RIFF_TAG_WAVE           0x45564157

#define RIFF_FORMAT_UNKNOWN     0x0000
#define RIFF_FORMAT_PCM         0x0001
#define RIFF_FORMAT_MSADPCM     0x0002
#define RIFF_FORMAT_IMAADPCM    0x0011
//******************************************************************************
//
// This function can be used to test if a file is a wav file or not and will
// also return the wav file header information in the pWaveHeader structure.
// If the file is a wav file then the psFileObject pointer will contain an
// open file pointer to the wave file ready to be passed into the WavePlay()
// function.
//
//******************************************************************************
static unsigned char buff[16];
FRESULT
OpenWavFile(FIL *psFileObject, char *pcFileName, SongInfoHeader *pSongInfoHeader)
{
  
    unsigned long *pulBuffer=(unsigned long *)buff;
    unsigned short *pusBuffer=(unsigned short *)buff;
    unsigned long ulChunkSize;
    unsigned short usCount;
    FRESULT Result;
    Result = f_open(psFileObject, pcFileName, FA_READ);
    if(Result != FR_OK)
    {
        return(Result);
    }

    //
    // Read the first 12 bytes.
    //
    Result = f_read(psFileObject, buff, 12, &usCount);
    if(Result != FR_OK)
    {
        f_close(psFileObject);
        return(Result);
    }

    //
    // Look for RIFF tag.
    //
    if((pulBuffer[0] != RIFF_CHUNK_ID_RIFF) || (pulBuffer[2] != RIFF_TAG_WAVE))
    {
        f_close(psFileObject);
        return(FR_INVALID_NAME);
    }

    //
    // Read the next chunk header.
    //
    Result = f_read(psFileObject,buff, 8, &usCount);
    if(Result != FR_OK)
    {
        f_close(psFileObject);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_FMT)
    {
        f_close(psFileObject);
        return(FR_INVALID_NAME);
    }
    //
    // Save the size of the data.
    //
    pSongInfoHeader->ulDataSize = pulBuffer[1];
    //
    // Read the format chunk size.
    //
    ulChunkSize = pulBuffer[1];

    if(ulChunkSize > 16)
    {
        f_close(psFileObject);
        return(FR_INVALID_NAME);
    }

    //
    // Read the next chunk header.
    //
    Result = f_read(psFileObject, buff, ulChunkSize, &usCount);
    if(Result != FR_OK)
    {
        f_close(psFileObject);
        return(Result);
    }

    pSongInfoHeader->usFormat = pusBuffer[0];
    pSongInfoHeader->usNumChannels =  pusBuffer[1];
    pSongInfoHeader->ulSampleRate = pulBuffer[1];
    pSongInfoHeader->ulAvgByteRate = pulBuffer[2];
    pSongInfoHeader->usBitsPerSample = pusBuffer[7];
    
    //
    // Only mono and stereo supported.
    //
    if(pSongInfoHeader->usNumChannels > 2)
    {
        f_close(psFileObject);
        return(FR_INVALID_NAME);
    }

    //
    // Read the next chunk header.
    //
    Result = f_read(psFileObject,buff, 8, &usCount);
    if(Result != FR_OK)
    {
        f_close(psFileObject);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_DATA)
    {
        f_close(psFileObject);
        return(Result);
    }

    //
    // Save the size of the data.
    //
    pSongInfoHeader->ulDataSize = pulBuffer[1];
    pSongInfoHeader->ulSecond = pSongInfoHeader->ulDataSize/pSongInfoHeader->ulAvgByteRate;
    pSongInfoHeader->ulMins = pSongInfoHeader->ulSecond/60;
    pSongInfoHeader->ulSecond -= pSongInfoHeader->ulMins*60;
    MinsCurrent=0;
    SecondCurrent=0;
    //
    // Adjust the average bit rate for 8 bit mono files.
    //
    if((pSongInfoHeader->usNumChannels == 1) && (pSongInfoHeader->usBitsPerSample == 8))
    {
        pSongInfoHeader->ulAvgByteRate <<=1;
    }
    return(FR_OK);
}