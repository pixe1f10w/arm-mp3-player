/**
* @file
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
#include "mp3_file.h"
#include "audio_play.h"
//
// The DMA control structure table.
//
#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif
//
//Using Semaphore by Queue
//
#define FLAGS_QUEUE_LENGTH 1
#define FLAGS_QUEUE_ITEM_SIZE 0
#define FLAGS_BUFFER_SIZE    ( ( FLAGS_QUEUE_LENGTH * FLAGS_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
static portCHAR cFlagsQueueBuffer[ FLAGS_BUFFER_SIZE ];
static xQueueHandle  xFlagsQueue;
static portBASE_TYPE xTaskWoken = pdFALSE;
//
// State information for keep track of time.
//
static unsigned long g_ulBytesPlayed;

/**
* Buffer management and flags.
*/
#define AUDIO_BUFFER_SIZE       4096
static unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
unsigned long g_ulMaxBufferSize;

volatile unsigned long g_ulFlags;

//
// Globals used to track playback position.
//
unsigned long g_ulBytesRemaining;
unsigned short g_usMinutes;
unsigned short g_usSeconds;

/**
*
* Handler for bufffers being released. Call from ISR
*
*/
void
BufferCallback(void *pvBuffer, unsigned long ulEvent)
{
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
}
/**
*
*/
char WaitBufferSignal(portTickType timeout){
  if(xQueueReceive(xFlagsQueue, NULL, timeout ) == pdPASS)
    return 0;
  else
    return 1;
}
char StartBufferSignal(portTickType timeout){
  if(xQueueSend(xFlagsQueue, NULL, timeout ) == pdPASS)
    return 0;
  else
    return 1;
}
/**
*
*/
char CheckExtension(char *fileName){
  char i=0;
  while(fileName[i]!='.'&&i<20)
    i++;
  if(i==20)
    return NOT_SUPPORT_FORMAT;
  if(fileName[i+1]=='w'&&fileName[i+2]=='a'&&fileName[i+3]=='v')
    return WAV_FILE;
  else if(fileName[i+1]=='m'&&fileName[i+2]=='p'&&fileName[i+3]=='3')
    return MP3_FILE;
  else
    return NOT_SUPPORT_FORMAT;
    
}
/**
*
*/
unsigned long
UpdateBufferForPlay(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader,char format)
{
static unsigned short usCount;
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
          if(format == WAV_FILE)
            usCount = WaveRead(psFileObject, pSoundInfoHeader, g_pucBuffer);
          else{
            //
            // No longer playing audio.
            //
            g_ulFlags &= ~BUFFER_PLAYING;
            return 0;
          }
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
          if(format == WAV_FILE)
            usCount = WaveRead(psFileObject, pSoundInfoHeader,
                               &g_pucBuffer[AUDIO_BUFFER_SIZE >> 1]);
          else{
            //
            // No longer playing audio.
            //
            g_ulFlags &= ~BUFFER_PLAYING;
            return 0;
          }
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
    //
        // Must disable I2S interrupts during this time to prevent state
        // problems.
        //
        IntEnable(INT_I2S0);
    return(1);
}
int
initAudioCodec(unsigned long volume)
{
    // GPIO Port B pins
    //
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinConfigure(GPIO_PB6_I2S0TXSCK);
    GPIOPinConfigure(GPIO_PB7_NMI);
    //
    // GPIO Port D pins.
    //
    GPIOPinConfigure(GPIO_PD0_I2S0RXSCK);
    GPIOPinConfigure(GPIO_PD1_I2S0RXWS);
    GPIOPinConfigure(GPIO_PD4_I2S0RXSD);
    GPIOPinConfigure(GPIO_PD5_I2S0RXMCLK);
    //
    // GPIO Port E pins
    //
    GPIOPinConfigure(GPIO_PE4_I2S0TXWS);
    GPIOPinConfigure(GPIO_PE5_I2S0TXSD);
    //
    // GPIO Port F pins
    //
    GPIOPinConfigure(GPIO_PF1_I2S0TXMCLK);
    GPIOPinConfigure(GPIO_PF2_LED1);
    GPIOPinConfigure(GPIO_PF3_LED0);
    //
    // Configure and enable uDMA
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    ROM_uDMAControlBaseSet(&sDMAControlTable[0]);
    ROM_uDMAEnable();
    //
    // Enable Interrupts
    //
    ROM_IntMasterEnable();
    //
    // Configure the I2S peripheral.
    //
    SoundInit(0);
    //
    // Set the initial volume to something sensible.  Beware - if you make the
    // mistake of using 24 ohm headphones and setting the volume to 100% you
    // may find it is rather too loud!
    //
    SoundVolumeSet(volume);
    //
    // Not playing anything right now.
    //
    g_ulFlags = 0;
    if(xQueueCreate( (signed portCHAR *)cFlagsQueueBuffer,FLAGS_BUFFER_SIZE,
                  FLAGS_QUEUE_LENGTH, FLAGS_QUEUE_ITEM_SIZE, &xFlagsQueue ) != pdPASS)
	{//FAIL;
        return 1;
    }
    return 0;
}

/**
* Convert an 8 bit unsigned buffer to 8 bit signed buffer in place so that it
* can be passed into the i2s playback.
*/
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
void setupAudioCodecForSong(SoundInfoHeader pSoundInfoHeader){
  unsigned long ulBytesPerSample;
  //
  // Reset the byte count.
  //
  //g_ulBytesPlayed = 0;
  //
  // Calculate the Maximum buffer size based on format.  There can only be
  // 1024 samples per ping pong buffer due to uDMA.
  //
  ulBytesPerSample = (pSoundInfoHeader.usBitsPerSample *
                        pSoundInfoHeader.usNumChannels) >> 3;

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

  g_usSeconds = pSoundInfoHeader.ulDataSize/pSoundInfoHeader.ulAvgByteRate;
  g_usMinutes = g_usSeconds/60;
  g_usSeconds -= g_usMinutes*60;
  //
  // Set the number of data bytes in the file.
  //
  g_ulBytesRemaining = pSoundInfoHeader.ulDataSize;
  //
  // Adjust the average bit rate for 8 bit mono files.
  //
  if((pSoundInfoHeader.usNumChannels == 1) && (pSoundInfoHeader.usBitsPerSample == 8))
  {
      pSoundInfoHeader.ulAvgByteRate <<=1;
  }
  //
  // Set the format of the playback in the sound driver.
  //
  SoundSetFormat(pSoundInfoHeader.ulSampleRate, pSoundInfoHeader.usBitsPerSample,
                   pSoundInfoHeader.usNumChannels);
}


//
static SoundInfoHeader g_sButtonHeader;
static FIL g_sButtonFileObject;
unsigned long s_ulMaxBufferSize;
unsigned long s_ulBytesRemaining;
unsigned long s_ulBytesPlayed;
unsigned long s_ulFlags;

void playButtonSound(SoundInfoHeader SongInfoHeader){
  //Store pre song
  s_ulMaxBufferSize= g_ulMaxBufferSize;
  s_ulBytesRemaining= g_ulBytesRemaining;
  s_ulBytesPlayed= g_ulBytesPlayed;
  // No longer playing audio.
  g_ulFlags &= ~BUFFER_PLAYING;
  //Open the button song
  if(OpenWavFile(&g_sButtonFileObject,"0:/SYS/button.wav",&g_sButtonHeader) == FR_OK ){
    //Setting for codec: bits/s, Hz, channels
    setupAudioCodecForSong(g_sButtonHeader);
    //set state for PLAY
    g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
    //Play the button song
    while(UpdateBufferForPlay(&g_sButtonFileObject,&g_sButtonHeader,WAV_FILE)!=0);
  }
  g_ulFlags &= ~BUFFER_PLAYING;
  //Wait for DMA TX is done
  while(uDMAChannelModeGet(UDMA_CHANNEL_I2S0TX | UDMA_PRI_SELECT) !=
           UDMA_MODE_STOP);
  while(uDMAChannelModeGet(UDMA_CHANNEL_I2S0TX | UDMA_ALT_SELECT) !=
           UDMA_MODE_STOP);
  //Retore pre song
  g_ulMaxBufferSize= s_ulMaxBufferSize;
  g_ulBytesRemaining= s_ulBytesRemaining;
  g_ulBytesPlayed= s_ulBytesPlayed;
  //
  // Set the format of the playback in the sound driver.
  //
  SoundSetFormat(SongInfoHeader.ulSampleRate, SongInfoHeader.usBitsPerSample,
                   SongInfoHeader.usNumChannels);
  g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
}
void AudioCodecIntHandler(void)
{
  xTaskWoken =pdFALSE;
  SoundIntHandler();
  /* If the peripheral handler task has a priority higher than the interrupted
    task request a switch to the handler task. */
  taskYIELD_FROM_ISR( xTaskWoken );
}