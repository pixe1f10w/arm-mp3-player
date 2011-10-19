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
#define ROOT            0
#define	NOWPLAY		1 //start item
#define BROWSE		2
#define SETTING		3
#define RECORD		4 //end item
unsigned char Menu = NOWPLAY;
unsigned char ChooseItem = NOWPLAY;
//setting: storage info, key sound, sleep time, usb/sd, default
unsigned char key_sound = 0;
/**time to sleep*/
unsigned char sleep_time = 0;
//browse: play, enter, back

//record: enter, pause, exit

//nowplay
unsigned char path[32] = "/";
unsigned char volume = 20;
unsigned char play_mode =REPEAT_ALL;

static FIL g_sButFileObject;
// The wav file header information.
static tWaveHeader g_sButWaveHeader;
/**
*
*/
static void
ProcessEventTask(void *pvParameters){
  portTickType ulLastTime;
  char event_code;
  unsigned long tmp;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
  while(1){
    //get event from button
    event_code = get_event_code(portMAX_DELAY);
    //play button sound
    //-Enter critical section
        vTaskSuspendScheduler();
       tmp = get_byes_remain();
    if(WaveOpen(&g_sButFileObject, "SYS/button.wav",
                    &g_sButWaveHeader) == FR_OK){
          set_play_flags(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
          while(UpdateBufferForPlay(&g_sButFileObject, &g_sButWaveHeader)!=0);
    }
    restore_format();
    set_play_flags(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
    set_byes_remain(tmp);
      //-Exit critical section
  xTaskResumeScheduler();
    //process event
    if(Menu == NOWPLAY){
      if(event_code == LONG_M){
          //back ROOT
          Menu = ROOT;
      }else if(event_code == SHORT_M){
          playCtlEvent(PAUSE_PLAY);
      }else if(event_code == SHORT_V_PLUS){  
        //-Enter critical section
        vTaskSuspendScheduler();
        //-Volume up
        SoundVolumeUp(2);
        //-Exit critical section
        xTaskResumeScheduler();
      }else if(event_code == SHORT_V_MINUS){
        //-Enter critical section
        vTaskSuspendScheduler();
        //volume down
        SoundVolumeDown(2);
        //-Exit critical section
        xTaskResumeScheduler();
      }else if(event_code == SHORT_P_PLUS){
          //next song
        playCtlEvent(NEXT_SONG);
      }else if(event_code == SHORT_P_MINUS){
          //previous song
        playCtlEvent(PRE_SONG);
      }
    }else if(Menu == BROWSE){
      if(event_code == LONG_M){
          //back ROOT
          Menu = ROOT;
      }else if(event_code == SHORT_M){
         //open dir if it's dir, else file and play;
      }
    }else if(Menu == SETTING){
      if(event_code == LONG_M){
          //back ROOT
          Menu = ROOT;
      };
    }else if(Menu == RECORD){
      if(event_code == LONG_M){
          //back ROOT
          Menu = ROOT;
      };
    }else if(Menu == ROOT){
      if(event_code == SHORT_M){
        //enter choose item;
        Menu = ChooseItem;
      }else if(event_code == SHORT_P_PLUS){
          //next item
        ChooseItem++;
        if(ChooseItem > RECORD)
          ChooseItem = NOWPLAY;
      }else if(event_code == SHORT_P_MINUS){
          //previous item
        ChooseItem--;
        if(ChooseItem < NOWPLAY)
          ChooseItem = RECORD;
      } 
    }
    // Wait for the required amount of time.
    //
    //xTaskDelayUntil(&ulLastTime, 0);
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