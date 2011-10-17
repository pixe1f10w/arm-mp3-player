/**
* @file	process_event_task.c
* @brief
*
* Get events from user via buttons.
* Process states (LCD off/on, sleep, run)
* User interface (LCD display menu, items)
* Control play_task
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
#include "detect_event_task.h"
#include "process_event_task.h"

xQueueHandle  xProcessEventQueue;
// The stack for the Process Event task.
static unsigned long ProcessEventTaskStack[128];
// The period of the Process Event task.
unsigned long ProcessEventDelay = 100;
/**
* menu
*/
#define	NOWPLAY		0
#define BROWSE		1
#define RECORD		2
#define SETTING		3
unsigned char menu = NOWPLAY;

//setting: storage info, key sound, sleep time, usb/sd, default
unsigned char key_sound = 0;
/**time to sleep*/
unsigned char sleep_time = 0;
//browse: play, enter, back

//record: enter, pause, exit

//nowplay
unsigned char path[32] = "/";
unsigned char volume = 20;
unsigned char play_event = NOTHING;
unsigned char play_mode =REPEAT_ALL;

static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;
// The wav file header information.
static tWaveHeader g_sWaveHeader;
/**
*
*/
static void
ProcessEventTask(void *pvParameters){
  portTickType ulLastTime;
  unsigned long PlayFlags =0;
  char event_code;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
  while(1){
        //
    event_code = get_event_code(portMAX_DELAY);
    /*if(WaveOpen(&g_sFileObject, "m.wav",
                &g_sWaveHeader) == FR_OK){
      //
      // Try to play Wav file.
      //
      WavePlay(&g_sFileObject, &g_sWaveHeader);
    }
    */
     if(WaveOpen(&g_sFileObject, "SYS/button.wav",
                    &g_sWaveHeader) == FR_OK){
          PlayFlags = BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING;
          set_play_flags(PlayFlags);
          while(UpdateBufferForPlay(&g_sFileObject, &g_sWaveHeader)!=0);
    }
    
    
    if(event_code == SHORT_V_PLUS)
      SoundVolumeUp(2);
    else if(event_code == SHORT_V_MINUS)
      SoundVolumeDown(2);
    else if(event_code == SHORT_M){
      toggle_play_ctl_flags(PLAY);
      //signal_for_play();
    }
    // Wait for the required amount of time.
    //
    //xTaskDelayUntil(&ulLastTime, 100);
  }
}
/**
*
*/
char process_event_task_init(void){
  //
  // Create the InGate task.
  //
  if(xTaskCreate(ProcessEventTask, (signed portCHAR *)"pro",
                   (signed portCHAR *)ProcessEventTaskStack,
                   sizeof(ProcessEventTaskStack), NULL, PRIORITY_PROCESS_EVENT_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}