/**
* @file play_task.c
* @brief
*
* - Take the order from process_event_task
* - Process: - play/stop
*			 - change song
*			 - seek
*			 - volume
* - Read files from SD card/USB
*/

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "priorities.h"
#include "mp3_file.h"
#include "wav_file.h"
#include <string.h>

#include "driverlib/flash.h"
#include "driverlib/udma.h"
#include "driverlib/i2s.h"
#include "drivers/sound.h"
#include "play_task.h"
//*****************************************************************************
//----------DEFINES
//*****************************************************************************
#define INITIAL_VOLUME_PERCENT 60

#define PLAY_QUEUE_LENGTH 1
#define PLAY_QUEUE_ITEM_SIZE 0
#define PLAY_BUFFER_SIZE    ( ( PLAY_QUEUE_LENGTH * PLAY_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )

//*****************************************************************************
//----------VARIABLES
//*****************************************************************************
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
//******************************************************************************
//
// Storage for the filename listbox widget string table.
//
//******************************************************************************
#define NUM_LIST_STRINGS 48
const char *g_ppcDirListStrings[NUM_LIST_STRINGS];

//******************************************************************************
//
// Storage for the names of the files in the current directory.  Filenames
// are stored in format "filename.ext".
//
//******************************************************************************
#define MAX_FILENAME_STRING_LEN (8 + 1 + 3 + 1)
char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];

static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;
// The wav file header information.
static tWaveHeader g_sWaveHeader;
portCHAR cPlayQueueBuffer[ PLAY_BUFFER_SIZE ];
xQueueHandle  xPlayQueue;
// The stack for the Play task.
static unsigned long PlayTaskStack[128];
// The period of the Play task.
unsigned long PlayDelay = 100;

char PlayEvent;
#define START       0
#define PAUSE       1
#define STOP        2
#define PLAY        3
#define NEXT        4
#define PREVIOUS    5
unsigned long PlayCtlFlags = START;
#define CONTINUOUS  0
#define SINGLE      1
unsigned char ListState = CONTINUOUS;
//******************************************************************************
//
// This function is called to read the contents of the current directory on
// the SD card and fill the listbox containing the names of all files.
//
//******************************************************************************
static int
PopulateFileListBox(unsigned long *count)
{
    unsigned long ulItemCount;
    FRESULT fresult;

    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&g_sDirObject, "/");

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
    *count = ulItemCount;
    //
    // Made it to here, return with no errors.
    //
    return(0);
}
void playCtlEvent(char a){
  PlayEvent = a;
  xQueueSend(xPlayQueue, NULL, 0);
}
void restore_format(void){
  SoundSetFormat(g_sWaveHeader.ulSampleRate, g_sWaveHeader.usBitsPerSample,
                   g_sWaveHeader.usNumChannels);
}
/**
*
*/
static void
PlayTask(void *pvParameters){
  portTickType ulLastTime;
  unsigned long max_count,count;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
  while(1){
    //get event
    if(xQueueReceive(xPlayQueue, NULL, 0)==pdPASS){
      if(PlayEvent == PAUSE_PLAY){
        if(PlayCtlFlags == PLAY)
          PlayCtlFlags = PAUSE;
        else if(PlayCtlFlags == PAUSE)
          PlayCtlFlags = PLAY;
      }else if (PlayEvent == NEXT_SONG){
        PlayCtlFlags = NEXT;
      }else if(PlayEvent == PRE_SONG){
        PlayCtlFlags = PREVIOUS;
      }
    }
    if(PlayCtlFlags == PLAY){
      //-Enter critical section
        vTaskSuspendScheduler();
      //update buffer for play;
       if( UpdateBufferForPlay(&g_sFileObject, &g_sWaveHeader)==0){
        //end file
         PlayCtlFlags = STOP;
       }
       //-Exit critical section
        xTaskResumeScheduler();
    }else if(PlayCtlFlags == PAUSE){
      ;
    }else if(PlayCtlFlags == STOP || PlayCtlFlags == NEXT){
      //close current file
      WaveClose(&g_sFileObject);
      //open next song
      count++;
      if(count >= max_count)
        count = 0;
      if(WaveOpen(&g_sFileObject, g_pcFilenames[count],
                     &g_sWaveHeader) == FR_OK ){
        set_play_flags(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
        if(ListState == CONTINUOUS){
          //play the song
          PlayCtlFlags = PLAY;
        }else if(ListState == SINGLE){
          //transfer to PAUSE
          PlayCtlFlags = PAUSE;
        }
                     }else{
        set_play_flags(~BUFFER_PLAYING);
        PlayCtlFlags = PAUSE;
                     }
        
      
    }else if(PlayCtlFlags == PREVIOUS){
      //close current file
      WaveClose(&g_sFileObject);
      //open pre song
      count--;
      if(count > max_count)
        count = max_count -1;
      if(WaveOpen(&g_sFileObject, g_pcFilenames[count],
                  &g_sWaveHeader) == FR_OK){
        set_play_flags(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
        if(ListState == CONTINUOUS){
          //play the song
          PlayCtlFlags = PLAY;
        
        }else if(ListState == SINGLE){
          //transfer to PAUSE
          PlayCtlFlags = PAUSE;
        }
                  }else{
        set_play_flags(~BUFFER_PLAYING);
        PlayCtlFlags = PAUSE;
                     }
      
    }else if(PlayCtlFlags == START){
      //list songs
      PopulateFileListBox(&max_count);
      //open first song
      count =0;
      while(WaveOpen(&g_sFileObject, g_pcFilenames[count],
                    &g_sWaveHeader) != FR_OK && count < max_count)
        count++;
      if(count < max_count){
        set_play_flags(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
        PlayCtlFlags = PAUSE;
      }
      //??????
    }

    wait_buffer_signal(PlayDelay);

    //
    // Wait for the required amount of time.
    //
    //xTaskDelayUntil(&ulLastTime, 10);
  }
}
/**
*
*/
char play_task_init(void){
    FRESULT fresult;
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
    // Mount the file system, using logical disk 0.
    //
    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
    {
        return(1);
    }


    init_wav_play();

    //
    // Configure the I2S peripheral.
    //
    SoundInit(0);

    //
    // Set the initial volume to something sensible.  Beware - if you make the
    // mistake of using 24 ohm headphones and setting the volume to 100% you
    // may find it is rather too loud!
    //
    SoundVolumeSet(INITIAL_VOLUME_PERCENT);
  if(xQueueCreate( (signed portCHAR *)cPlayQueueBuffer,PLAY_BUFFER_SIZE,
                  PLAY_QUEUE_LENGTH, PLAY_QUEUE_ITEM_SIZE, &xPlayQueue ) != pdPASS){
                    //FAIL;
                    while(1);
                  }
  //
  // Create the InGate task.
  //
  if(xTaskCreate(PlayTask, (signed portCHAR *)"play",
                   (signed portCHAR *)PlayTaskStack,
                   sizeof(PlayTaskStack), NULL, PRIORITY_PLAY_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}