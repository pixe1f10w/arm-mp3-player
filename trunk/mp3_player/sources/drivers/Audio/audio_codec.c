/**
* @file
*/
#include <string.h>
#include "driverlib/rom.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "drivers/tlv320aic23b.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "driverlib/systick.h"
#include "driverlib/i2s.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "utils/ustdlib.h"
#include "drivers/sound.h"
#include "file_management.h"
#include "audio_codec.h"
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


unsigned char VolumeValue;
//!the number of DMA transfer <= 1024
//! ulMaxBufferSize = max{(the number of transfer) * (ulBytesPerSample), AUDIO_BUFFER_SIZE>>1};
unsigned long   ulMaxBufferSize;
//
volatile unsigned long ulFlags;
/**
* Buffer management and flags.
*/
#define AUDIO_BUFFER_SIZE       4096
static unsigned char pucBuffer[AUDIO_BUFFER_SIZE];
//static unsigned short usNumBytes;
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
        if(pvBuffer == pucBuffer)
        {
            //
            // Flag if the first half is free.
            //
            ulFlags |= BUFFER_BOTTOM_EMPTY;

        }
        else
        {
            //
            // Flag if the second half is free.
            //
            ulFlags |= BUFFER_TOP_EMPTY;

        }
        //Wakeup play task
        xQueueSendFromISR( xFlagsQueue,NULL,&xTaskWoken);
        //
        // Update the byte count.
        //
    }
}
void initBuffer(unsigned long   ulBufferSize)
{
//! Initialize Buffer
    ulMaxBufferSize = ulBufferSize;
    //usNumBytes = 0;
    ulFlags=BUFFER_BOTTOM_EMPTY|BUFFER_TOP_EMPTY;
}
/**
*
*/
//static unsigned char tmp[AUDIO_BUFFER_SIZE>>1];
unsigned short WriteWAVToBuffer(FIL* pSongFileObject,unsigned char *pData, 
                                unsigned short  nBytes, unsigned char usForcePlay, unsigned char Convert8bit)
{
    unsigned short rt=0,usCount;

    //
    // Must disable I2S interrupts during this time to prevent state
    // problems.
    //
    
    IntDisable(INT_I2S0);
    //vPortEnterCritical( );  
    if (ulFlags & BUFFER_BOTTOM_EMPTY) 
    {
      if(f_read(pSongFileObject,pucBuffer,ulMaxBufferSize, &usCount) != FR_OK)
      {
        rt = 1;
      }else
      {
        if(Convert8bit)
          Convert8Bit(pucBuffer, usCount);
        // Start the playback for a new buffer.
        SoundBufferPlay(&pucBuffer, ulMaxBufferSize, BufferCallback);
        //
        // Bottom half of the buffer is now not empty.
        //
        ulFlags &= ~BUFFER_BOTTOM_EMPTY;
        if(ulMaxBufferSize > usCount)
          rt=1;
      }
      
    }
    if (ulFlags & BUFFER_TOP_EMPTY)
    {
      if(f_read(pSongFileObject,&pucBuffer[AUDIO_BUFFER_SIZE>>1],ulMaxBufferSize, &usCount) != FR_OK)
      {
        rt = 1;
      }else
      {
        if(Convert8bit)
          Convert8Bit(&pucBuffer[AUDIO_BUFFER_SIZE>>1], usCount);
              // Start the playback for a new buffer.
        SoundBufferPlay(&pucBuffer[AUDIO_BUFFER_SIZE>>1], ulMaxBufferSize, BufferCallback);
              //
              // Top half of the buffer is now not empty.
              //
        ulFlags &= ~BUFFER_TOP_EMPTY;
        if(ulMaxBufferSize > usCount)
          rt=1;
      }
    }
    //
    // Must disable I2S interrupts during this time to prevent state
    // problems.
    //
    IntEnable(INT_I2S0);
    //vPortExitCritical( );
    return rt;
}
/**
*
*/
//static unsigned char tmp[AUDIO_BUFFER_SIZE>>1];
static unsigned short index =0;
static unsigned char *pBuff=pucBuffer;
unsigned short WriteMP3ToBuffer(short int *pData, 
                                unsigned short  nsCount)
{
    unsigned short rt=0,i;

    //
    // Must disable I2S interrupts during this time to prevent state
    // problems.
    //
    
   
    //vPortEnterCritical( );
    if(pBuff)
    {
      for(i=0;i<nsCount;i++)
      {
        pBuff[index+2*i]=(unsigned char)(pData[i] & 0x00ff);
        pBuff[index+2*i+1]=(unsigned char)((pData[i]>>8) & 0x00ff);
      }
      index +=nsCount*2;
    
      if(index >= ulMaxBufferSize)
      {
        index=0;
        IntDisable(INT_I2S0);
        if(pBuff==pucBuffer)
        {
          // Start the playback for a new buffer.
          SoundBufferPlay(&pucBuffer, ulMaxBufferSize, BufferCallback);
          //
          // Bottom half of the buffer is now not empty.
          //
          ulFlags &= ~BUFFER_BOTTOM_EMPTY;
          pBuff=&pucBuffer[AUDIO_BUFFER_SIZE>>1];
          
        }else
        {
          // Start the playback for a new buffer.
          SoundBufferPlay(&pucBuffer[AUDIO_BUFFER_SIZE>>1], ulMaxBufferSize, BufferCallback);
          //
          // Bottom half of the buffer is now not empty.
          //
          ulFlags &= ~BUFFER_TOP_EMPTY;
          pBuff=pucBuffer;
        }
        IntEnable(INT_I2S0);
      }
    }
    //
    // Must disable I2S interrupts during this time to prevent state
    // problems.
    //
    
    if(ulFlags ==0)
     WaitBufferSignal(10);
    //vPortExitCritical( );
    return rt;
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
    IntPrioritySet(INT_I2S0,5<<5);
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
    ulFlags = 0;
    initBuffer(AUDIO_BUFFER_SIZE>>1);
    //! Create Queue
    if(xQueueCreate( (signed portCHAR *)cFlagsQueueBuffer,FLAGS_BUFFER_SIZE,
                  FLAGS_QUEUE_LENGTH, FLAGS_QUEUE_ITEM_SIZE, &xFlagsQueue ) != pdPASS)
	{//FAIL;
        return 1;
    }
    return 0;
}
/**
*
*/
void AudioCodecIntHandler(void)
{
  xTaskWoken =pdFALSE;
  SoundIntHandler();
  /* If the peripheral handler task has a priority higher than the interrupted
    task request a switch to the handler task. */
  taskYIELD_FROM_ISR( xTaskWoken );
}
void setupAudioCodecForSong(SongInfoHeader *pSongInfoHeader){
  unsigned long ulBytesPerSample;
  //
  // Reset the byte count.
  //
  //g_ulBytesPlayed = 0;
  //
  // Calculate the Maximum buffer size based on format.  There can only be
  // 1024 samples per ping pong buffer due to uDMA.
  //
  ulBytesPerSample = (pSongInfoHeader->usBitsPerSample *
                        pSongInfoHeader->usNumChannels) >> 3;

  if(((AUDIO_BUFFER_SIZE >> 1) / ulBytesPerSample) > 1024)
  {
      //
      // The maximum number of DMA transfers was more than 1024 so limit
      // it to 1024 transfers.
      //
      ulMaxBufferSize = 1024 * ulBytesPerSample;
  }
  else
  {
      //
      // The maximum number of DMA transfers was not more than 1024.
      //
      ulMaxBufferSize = AUDIO_BUFFER_SIZE >> 1;
  }
  //
  // Adjust the average bit rate for 8 bit mono files.
  //
  if((pSongInfoHeader->usNumChannels == 1) && (pSongInfoHeader->usBitsPerSample == 8))
  {
      pSongInfoHeader->ulAvgByteRate <<=1;
  }
  //
  // Set the format of the playback in the sound driver.
  //
  SoundSetFormat(pSongInfoHeader->ulSampleRate, pSongInfoHeader->usBitsPerSample,
                   pSongInfoHeader->usNumChannels);
}
void VolumeUp(void)
{
  VolumeValue +=2;
  if(VolumeValue>100)
    VolumeValue=100;
  TLV320AIC23BHeadPhoneVolumeSet(VolumeValue);
}
void VolumeDown(void)
{
  VolumeValue -=2;
  if(VolumeValue >100)
    VolumeValue=0;
  TLV320AIC23BHeadPhoneVolumeSet(VolumeValue);
}