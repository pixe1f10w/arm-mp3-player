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
#include "support_play_file.h"
#include "button_event_task.h"
#include "sound_player_task.h"
#include "conf.h"
#include "player_control_task.h"

xQueueHandle  xProcessEventQueue;
// The stack for the Process Event task.
static unsigned long ProcessEventTaskStack[128];
// The period of the Process Event task.
unsigned long ProcessEventDelay = 100;
char *Filename;
FATFS g_sFatFs;
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
unsigned char volume = 20;
unsigned char play_mode =REPEAT_ALL;

/**
*
*/
static void
ProcessEventTask(void *pvParameters){
  //portTickType ulLastTime;
  char eventCode;
  char itemCount = 0;
    //
    // Get the current tick count.
    //
    //ulLastTime = xTaskGetTickCount();
  PopulateFileListBox();
    //
    // Loop forever.
    //
  while(1){
    //get event from button
    eventCode = takeButtonEvntCode(1000);
    //if(eventCode == 0xff)
    //  eventCode = NOTHING;
    //play button sound
    giveSoundCtlEvent(PLAY_BTN_SND);
    //process event
    if(Menu == NOWPLAY){
      if(eventCode == L_CENTER){
          //back ROOT
          Menu = ROOT;
      }else if(eventCode == S_CENTER){
          giveSoundCtlEvent(PAUSE_PLAY);
      }else if(eventCode == S_RIGHT){  
        //-Enter critical section
        vTaskSuspendScheduler();
        //-Volume up
        volume+=2;
        SoundVolumeUp(2);
        //-Exit critical section
        xTaskResumeScheduler();
      }else if(eventCode == S_LEFT){
        //-Enter critical section
        vTaskSuspendScheduler();
        //volume down
        volume -=2;
        SoundVolumeDown(2);
        //-Exit critical section
        xTaskResumeScheduler();
      }else if(eventCode == S_UP){
        itemCount++;//next song
        if(itemCount >= maxItemCount)
          itemCount =0;
        Filename = g_pcFilenames[itemCount];
        giveSoundCtlEvent(START);
      }else if(eventCode == S_DOWN){
        //previous song
        if(itemCount <= 0)
          itemCount = maxItemCount;
        else
          itemCount--;
        Filename = g_pcFilenames[itemCount];
        giveSoundCtlEvent(START);
      }
    }else if(Menu == BROWSE){
      if(eventCode == L_CENTER){
          //back ROOT
          Menu = ROOT;
      }else if(eventCode == S_CENTER){
         //open dir if it's dir, else file and play;
      }
    }else if(Menu == SETTING){
      if(eventCode == L_CENTER){
          //back ROOT
          Menu = ROOT;
      };
    }else if(Menu == RECORD){
      if(eventCode == L_CENTER){
          //back ROOT
          Menu = ROOT;
      };
    }else if(Menu == ROOT){
      if(eventCode == S_CENTER){
        //enter choose item;
        Menu = ChooseItem;
      }else if(eventCode == S_UP){
          //next item
        ChooseItem++;
        if(ChooseItem > RECORD)
          ChooseItem = NOWPLAY;
      }else if(eventCode == S_DOWN){
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
char initPlayerControlTask(void){
  FRESULT fresult;
  //
    // Mount the file system, using logical disk 0.
    //
    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
    {
        return(1);
    }
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