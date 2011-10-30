/**
*
*/


#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "driverlib/systick.h"
#include "driverlib/i2s.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "utils/ustdlib.h"
#include "drivers/sound.h"
#include "audio_play.h"
#include "wav_file.h"


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
OpenWavFile(FIL *psFileObject, char *pcFileName, SoundInfoHeader *pSoundInfoHeader)
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

    pSoundInfoHeader->usFormat = pusBuffer[0];
    pSoundInfoHeader->usNumChannels =  pusBuffer[1];
    pSoundInfoHeader->ulSampleRate = pulBuffer[1];
    pSoundInfoHeader->ulAvgByteRate = pulBuffer[2];
    pSoundInfoHeader->usBitsPerSample = pusBuffer[7];

    //
    // Only mono and stereo supported.
    //
    if(pSoundInfoHeader->usNumChannels > 2)
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
    pSoundInfoHeader->ulDataSize = pulBuffer[1];
 
    //
    // Adjust the average bit rate for 8 bit mono files.
    //
    if((pSoundInfoHeader->usNumChannels == 1) && (pSoundInfoHeader->usBitsPerSample == 8))
    {
        pSoundInfoHeader->ulAvgByteRate <<=1;
    }
    return(FR_OK);
}



//******************************************************************************
//
// This function will handle reading the correct amount from the wav file and
// will also handle converting 8 bit unsigned to 8 bit signed if necessary.
//
//******************************************************************************
unsigned short
WaveRead(FIL *psFileObject, SoundInfoHeader *pSoundInfoHeader, unsigned char *pucBuffer)
{
    unsigned long ulBytesToRead;
    unsigned short usCount;

    //
    // Either read a half buffer or just the bytes remaining if we are at the
    // end of the file.
    //
    if(g_ulBytesRemaining < g_ulMaxBufferSize)
    {
        ulBytesToRead = g_ulBytesRemaining;
    }
    else
    {
        ulBytesToRead = g_ulMaxBufferSize;
    }

    //
    // Read in another buffer from the sd card.
    //
    if(f_read(psFileObject, pucBuffer, ulBytesToRead, &usCount) != FR_OK)
    {
        return(0);
    }

    //
    // Decrement the number of data bytes remaining to be read.
    //
    g_ulBytesRemaining -= usCount;

    //
    // Need to convert the audio from unsigned to signed if 8 bit
    // audio is used.
    //
    if(pSoundInfoHeader->usBitsPerSample == 8)
    {
        Convert8Bit(pucBuffer, usCount);
    }

    return(usCount);
}
