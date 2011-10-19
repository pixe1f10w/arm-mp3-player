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
#include "wav_file.h"

#define FLAGS_QUEUE_LENGTH 1
#define FLAGS_QUEUE_ITEM_SIZE 0
#define FLAGS_BUFFER_SIZE    ( ( FLAGS_QUEUE_LENGTH * FLAGS_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
//*****************************************************************************
//----------VARIABLES
//*****************************************************************************
portCHAR cFlagsQueueBuffer[ FLAGS_BUFFER_SIZE ];
xQueueHandle  xFlagsQueue;
//******************************************************************************
//
// The following are data structures used by FatFs.
//
//******************************************************************************

//******************************************************************************
//
// The number of SysTick ticks per second.
//
//******************************************************************************
#define TICKS_PER_SECOND 100



//******************************************************************************
//
// Forward declarations for functions called by the widgets used in the user
// interface.
//
//******************************************************************************

void WaveStop(void);




//******************************************************************************
//
// State information for keep track of time.
//
//******************************************************************************
static unsigned long g_ulBytesPlayed;

//******************************************************************************
//
// Buffer management and flags.
//
//******************************************************************************
#define AUDIO_BUFFER_SIZE       4096
static unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
unsigned long g_ulMaxBufferSize;

//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004
static volatile unsigned long g_ulFlags;

//
// Globals used to track playback position.
//
static unsigned long g_ulBytesRemaining;
static unsigned short g_usMinutes;
static unsigned short g_usSeconds;

//******************************************************************************
//
// Handler for bufffers being released. Call from ISR
//
//******************************************************************************
void
BufferCallback(void *pvBuffer, unsigned long ulEvent)
{
  portBASE_TYPE xTaskWoken = pdFALSE;
    if(ulEvent & BUFFER_EVENT_FREE)
    {
        if(pvBuffer == g_pucBuffer)
        {
            //
            // Flag if the first half is free.
            //
            g_ulFlags |= BUFFER_BOTTOM_EMPTY;
        }
        else
        {
            //
            // Flag if the second half is free.
            //
            g_ulFlags |= BUFFER_TOP_EMPTY;
        }
        //Wakeup play task
        xQueueSendFromISR( xFlagsQueue,NULL,&xTaskWoken);
        //
        // Update the byte count.
        //
        g_ulBytesPlayed += AUDIO_BUFFER_SIZE >> 1;
    }
    /* If the peripheral handler task has a priority higher than the interrupted
    task request a switch to the handler task. */
    //taskYIELD_FROM_ISR( xTaskWoken );
}
char wait_buffer_signal(portTickType timeout){
  if(xQueueReceive(xFlagsQueue, NULL, timeout ) == pdPASS)
    return 0;
  else
    return 1;
}
void set_play_flags(unsigned long a){
  g_ulFlags = a;
}
unsigned long get_play_flags(void){
  return g_ulFlags;
}
unsigned long get_byes_remain(void){
  return g_ulBytesRemaining;
}
void set_byes_remain( unsigned long a){
  g_ulBytesRemaining = a;
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
FRESULT
WaveOpen(FIL *psFileObject, const char *pcFileName, tWaveHeader *pWaveHeader)
{
    unsigned long *pulBuffer;
    unsigned short *pusBuffer;
    unsigned long ulChunkSize;
    unsigned short usCount;
    unsigned long ulBytesPerSample;
    FRESULT Result;

    pulBuffer = (unsigned long *)g_pucBuffer;
    pusBuffer = (unsigned short *)g_pucBuffer;

    Result = f_open(psFileObject, pcFileName, FA_READ);
    if(Result != FR_OK)
    {
        return(Result);
    }

    //
    // Read the first 12 bytes.
    //
    Result = f_read(psFileObject, g_pucBuffer, 12, &usCount);
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
    Result = f_read(psFileObject, g_pucBuffer, 8, &usCount);
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
    Result = f_read(psFileObject, g_pucBuffer, ulChunkSize, &usCount);
    if(Result != FR_OK)
    {
        f_close(psFileObject);
        return(Result);
    }

    pWaveHeader->usFormat = pusBuffer[0];
    pWaveHeader->usNumChannels =  pusBuffer[1];
    pWaveHeader->ulSampleRate = pulBuffer[1];
    pWaveHeader->ulAvgByteRate = pulBuffer[2];
    pWaveHeader->usBitsPerSample = pusBuffer[7];

    //
    // Reset the byte count.
    //
    g_ulBytesPlayed = 0;

    //
    // Calculate the Maximum buffer size based on format.  There can only be
    // 1024 samples per ping pong buffer due to uDMA.
    //
    ulBytesPerSample = (pWaveHeader->usBitsPerSample *
                        pWaveHeader->usNumChannels) >> 3;

    if(((AUDIO_BUFFER_SIZE >> 1) / ulBytesPerSample) > 1024)
    {
        //
        // The maximum number of DMA transfers was more than 1024 so limit
        // it to 1024 transfers.
        //
        g_ulMaxBufferSize = 1024 * ulBytesPerSample;
    }
    else
    {
        //
        // The maximum number of DMA transfers was not more than 1024.
        //
        g_ulMaxBufferSize = AUDIO_BUFFER_SIZE >> 1;
    }

    //
    // Only mono and stereo supported.
    //
    if(pWaveHeader->usNumChannels > 2)
    {
        f_close(psFileObject);
        return(FR_INVALID_NAME);
    }

    //
    // Read the next chunk header.
    //
    Result = f_read(psFileObject, g_pucBuffer, 8, &usCount);
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
    pWaveHeader->ulDataSize = pulBuffer[1];

    g_usSeconds = pWaveHeader->ulDataSize/pWaveHeader->ulAvgByteRate;
    g_usMinutes = g_usSeconds/60;
    g_usSeconds -= g_usMinutes*60;

    //
    // Set the number of data bytes in the file.
    //
    g_ulBytesRemaining = pWaveHeader->ulDataSize;

    //
    // Adjust the average bit rate for 8 bit mono files.
    //
    if((pWaveHeader->usNumChannels == 1) && (pWaveHeader->usBitsPerSample == 8))
    {
        pWaveHeader->ulAvgByteRate <<=1;
    }

    //
    // Set the format of the playback in the sound driver.
    //
    SoundSetFormat(pWaveHeader->ulSampleRate, pWaveHeader->usBitsPerSample,
                   pWaveHeader->usNumChannels);
    return(FR_OK);
}

//******************************************************************************
//
// This closes out the wav file.
//
//******************************************************************************
void
WaveClose(FIL *psFileObject)
{
    //
    // Close out the file.
    //
    f_close(psFileObject);
}

//******************************************************************************
//
// Convert an 8 bit unsigned buffer to 8 bit signed buffer in place so that it
// can be passed into the i2s playback.
//
//******************************************************************************
void
Convert8Bit(unsigned char *pucBuffer, unsigned long ulSize)
{
    unsigned long ulIdx;

    for(ulIdx = 0; ulIdx < ulSize; ulIdx++)
    {
        //
        // In place conversion of 8 bit unsigned to 8 bit signed.
        //
        *pucBuffer = ((short)(*pucBuffer)) - 128;
        pucBuffer++;
    }
}

//******************************************************************************
//
// This function will handle stopping the playback of audio.  It will not do
// this immediately but will defer stopping audio at a later time.  This allows
// this function to be called from an interrupt handler.
//
//******************************************************************************
void
WaveStop(void)
{
    //
    // Stop playing audio.
    //
    g_ulFlags &= ~BUFFER_PLAYING;
}

//******************************************************************************
//
// This function will handle reading the correct amount from the wav file and
// will also handle converting 8 bit unsigned to 8 bit signed if necessary.
//
//******************************************************************************
unsigned short
WaveRead(FIL *psFileObject, tWaveHeader *pWaveHeader, unsigned char *pucBuffer)
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
    if(pWaveHeader->usBitsPerSample == 8)
    {
        Convert8Bit(pucBuffer, usCount);
    }

    return(usCount);
}

//******************************************************************************
//
// This will play the file passed in via the psFileObject parameter based on
// the format passed in the pWaveHeader structure.  The WaveOpen() function
// can be used to properly fill the pWaveHeader and psFileObject structures.
//
//******************************************************************************
unsigned long
WavePlay(FIL *psFileObject, tWaveHeader *pWaveHeader)
{
    static unsigned short usCount;

    //
    // Mark both buffers as empty.
    //
    g_ulFlags = BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY;

    //
    // Indicate that the application is about to start playing.
    //
    g_ulFlags |= BUFFER_PLAYING;

    while(1)
    {
        //
        // Must disable I2S interrupts during this time to prevent state
        // problems.
        //
        IntDisable(INT_I2S0);
    
        //
        // If the refill flag gets cleared then fill the requested side of the
        // buffer.
        //
        if(g_ulFlags & BUFFER_BOTTOM_EMPTY)
        {
            //
            // Read out the next buffer worth of data.
            //
            usCount = WaveRead(psFileObject, pWaveHeader, g_pucBuffer);

            //
            // Start the playback for a new buffer.
            //
            SoundBufferPlay(g_pucBuffer, usCount, BufferCallback);

            //
            // Bottom half of the buffer is now not empty.
            //
            g_ulFlags &= ~BUFFER_BOTTOM_EMPTY;
        }

        if(g_ulFlags & BUFFER_TOP_EMPTY)
        {
            //
            // Read out the next buffer worth of data.
            //
            usCount = WaveRead(psFileObject, pWaveHeader,
                               &g_pucBuffer[AUDIO_BUFFER_SIZE >> 1]);

            //
            // Start the playback for a new buffer.
            //
            SoundBufferPlay(&g_pucBuffer[AUDIO_BUFFER_SIZE >> 1],
                            usCount, BufferCallback);

            //
            // Top half of the buffer is now not empty.
            //
            g_ulFlags &= ~BUFFER_TOP_EMPTY;
        }

        //
        // If something reset this while playing then stop playing and break
        // out of the loop.
        //
        if((g_ulFlags & BUFFER_PLAYING) == 0)
        {
            break;
        }

        //
        // Audio playback is done once the count is below a full buffer.
        //
        if((usCount < g_ulMaxBufferSize) || (g_ulBytesRemaining == 0))
        {
            //
            // No longer playing audio.
            //
            g_ulFlags &= ~BUFFER_PLAYING;

            //
            // Wait for the buffer to empty.
            //
            while(g_ulFlags != (BUFFER_TOP_EMPTY | BUFFER_BOTTOM_EMPTY))
            {
            }
            break;
        }

        //
        // Must disable I2S interrupts during this time to prevent state
        // problems.
        //
        IntEnable(INT_I2S0);
    }

    //
    // Close out the file.
    //
    WaveClose(psFileObject);

    return(0);
}

unsigned long
UpdateBufferForPlay(FIL *psFileObject, tWaveHeader *pWaveHeader)
{
    static unsigned short usCount;
    //
    // If the refill flag gets cleared then fill the requested side of the
    // buffer.
    //
    if(g_ulFlags & BUFFER_BOTTOM_EMPTY)
        {
            //
            // Read out the next buffer worth of data.
            //
            usCount = WaveRead(psFileObject, pWaveHeader, g_pucBuffer);

            //
            // Start the playback for a new buffer.
            //
            SoundBufferPlay(g_pucBuffer, usCount, BufferCallback);

            //
            // Bottom half of the buffer is now not empty.
            //
            g_ulFlags &= ~BUFFER_BOTTOM_EMPTY;
        }

        if(g_ulFlags & BUFFER_TOP_EMPTY)
        {
            //
            // Read out the next buffer worth of data.
            //
            usCount = WaveRead(psFileObject, pWaveHeader,
                               &g_pucBuffer[AUDIO_BUFFER_SIZE >> 1]);

            //
            // Start the playback for a new buffer.
            //
            SoundBufferPlay(&g_pucBuffer[AUDIO_BUFFER_SIZE >> 1],
                            usCount, BufferCallback);

            //
            // Top half of the buffer is now not empty.
            //
            g_ulFlags &= ~BUFFER_TOP_EMPTY;
        }

        //
        // If something reset this while playing then stop playing and break
        // out of the loop.
        //
        if((g_ulFlags & BUFFER_PLAYING) == 0)
        {
            return 0;
        }

        //
        // Audio playback is done once the count is below a full buffer.
        //
        if((usCount < g_ulMaxBufferSize) || (g_ulBytesRemaining == 0))
        {
            //
            // No longer playing audio.
            //
            g_ulFlags &= ~BUFFER_PLAYING;

            //
            // Wait for the buffer to empty.
            //
            if(g_ulFlags == (BUFFER_TOP_EMPTY | BUFFER_BOTTOM_EMPTY))
              return 0;
        }
    return(1);
}



//******************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//******************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//******************************************************************************
//
// The program main function.  It performs initialization, then handles wav
// file playback.
//
//******************************************************************************
int
init_wav_play(void)
{
    //
    // Not playing anything right now.
    //
    g_ulFlags = 0;
    if(xQueueCreate( (signed portCHAR *)cFlagsQueueBuffer,FLAGS_BUFFER_SIZE,
                  FLAGS_QUEUE_LENGTH, FLAGS_QUEUE_ITEM_SIZE, &xFlagsQueue ) != pdPASS){
                    //FAIL;
                    while(1);
                  }
    return 0;
}
