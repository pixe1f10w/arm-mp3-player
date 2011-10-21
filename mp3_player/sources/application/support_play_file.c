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
#include "support_play_file.h"

#define INITIAL_VOLUME_PERCENT 60
//******************************************************************************
//
// The DMA control structure table.
//
//******************************************************************************
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
#define FLAGS_QUEUE_LENGTH 1
#define FLAGS_QUEUE_ITEM_SIZE 0
#define FLAGS_BUFFER_SIZE    ( ( FLAGS_QUEUE_LENGTH * FLAGS_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
portCHAR cFlagsQueueBuffer[ FLAGS_BUFFER_SIZE ];
xQueueHandle  xFlagsQueue;

extern unsigned char volume ;
//******************************************************************************
//
// State information for keep track of time.
//
//******************************************************************************
unsigned long g_ulBytesPlayed;

//******************************************************************************
//
// Buffer management and flags.
//
//******************************************************************************
unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
unsigned long g_ulMaxBufferSize;

//******************************************************************************
//
// Storage for the names of the files in the current directory.  Filenames
// are stored in format "filename.ext".
//
//******************************************************************************
char  maxItemCount;
char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
DIR g_sDirObject;
FILINFO g_sFileInfo;
//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004
volatile unsigned long g_ulFlags;

//
// Globals used to track playback position.
//
unsigned long g_ulBytesRemaining;
unsigned short g_usMinutes;
unsigned short g_usSeconds;

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
char WaitBufferSignal(portTickType timeout){
  if(xQueueReceive(xFlagsQueue, NULL, timeout ) == pdPASS)
    return 0;
  else
    return 1;
}

void CloseFile(FIL *psFileObject){
  f_close(psFileObject);
}
char CheckExtension(char *fileName){
  return WAV_FILE;
}
  


unsigned long
UpdateBufferForPlay(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader,char format)
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
    return(1);
}
int
init_play_sound(void)
{
  //Setup hardware

  //
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
                  FLAGS_QUEUE_LENGTH, FLAGS_QUEUE_ITEM_SIZE, &xFlagsQueue ) != pdPASS){
                    //FAIL;
                    while(1);
                  }
    return 0;
}
//******************************************************************************
//
// This function is called to read the contents of the current directory on
// the SD card and fill the listbox containing the names of all files.
//
//******************************************************************************
int
PopulateFileListBox(void)
{
    unsigned long ulItemCount;
    FRESULT fresult;

    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&g_sDirObject, "0:/");

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        //
        // Ensure that the error is reported.
        //
        return(fresult);
    }

    ulItemCount = 0;

    //
    // Enter loop to enumerate through all directory entries.
    //
    while(1)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        //
        // Add the information as a line in the listbox widget.
        //
        if(ulItemCount < NUM_LIST_STRINGS)
        {
            //
            // Ignore directories.
            //
            if((g_sFileInfo.fattrib & AM_DIR) == 0)
            {
                strncpy(g_pcFilenames[ulItemCount], g_sFileInfo.fname,
                         MAX_FILENAME_STRING_LEN);
            }
        }

        //
        // Ignore directories.
        //
        if((g_sFileInfo.fattrib & AM_DIR) == 0)
        {
            //
            // Move to the next entry in the item array we use to populate the
            // list box.
            //
            ulItemCount++;
        }
    }
    maxItemCount = ulItemCount;
    //
    // Made it to here, return with no errors.
    //
    return(0);
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