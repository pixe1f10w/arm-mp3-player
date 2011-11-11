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
#include "button_event_task.h"
#include "sound_player_task.h"
#include "conf.h"
#include "file_management.h"
#include "player_control_task.h"
/**
* Queue for control
*/
#define PLAYER_CTRL_Q_LENGTH      2 //1 Play button, 1 data control
#define PLAYER_CTRL_Q_ITEM_SIZE   1
#define PLAYER_CTRL_BUFFER_SIZE   ( ( PLAYER_CTRL_Q_LENGTH *PLAYER_CTRL_Q_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
static portCHAR cPlayerCtrlQueueBuffer[ PLAYER_CTRL_BUFFER_SIZE ];
xQueueHandle  xPlayerCtrlQueue;

// The stack for the Process Event task.
static unsigned long PlayerCtrlTaskStack[128];
// The period of the Process Event task.
unsigned long PlayerCtrlDelay = 100;
//
char *sFilePath;
//Save file system
static FATFS g_sFatFs;
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
unsigned char volume = 60;
unsigned char play_mode =REPEAT_ALL;

//Save Event for processing
unsigned char usEventCode;
unsigned char usStopPlayerCtrl;
/**
*
*/
portBASE_TYPE givePlayerCtrlEvent(unsigned char *data, portTickType timeout){
  return xQueueSend(xPlayerCtrlQueue, (unsigned char*)data, timeout);
}

/**
*
*/
static void
PlayerCtrlTask(void *pvParameters){
  //portTickType ulLastTime;
  
  //char itemCount = 0;
  unsigned char soundCtrl = 0;
  //Start Player Ctrl
  usStopPlayerCtrl = 0;
  
  //List files in root
  ListFiles();
  sFilePath=getCurrentFile();
    //
    // Loop forever.
    //
  while(1){
    //get event from button
    if( xQueueReceive(xPlayerCtrlQueue, (unsigned char*)&usEventCode,100)==pdPASS)
    {
      //Start/stop Player Ctrl
      if(usEventCode == STOP_PLAYER)
      {
        usStopPlayerCtrl = 1;
        //Stop Sound Player
        soundCtrl = STOP;
        giveSoundCtrlEvent((unsigned char*)&soundCtrl,portMAX_DELAY);
      }
      else if (usEventCode == START_PLAYER)
        usStopPlayerCtrl = 0;
      
      if(usStopPlayerCtrl == 0)
      {
        //Play sound Button when button pressed
        if(usEventCode < 10)
        {
          soundCtrl = PLAY_BTN_SND;
          giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
        }
        if(usEventCode==PLAY_DONE)
        {
          /**Play 1 round*/
          //Play next song
          sFilePath = nextFile();
          if(ucFileIndex!=0)
          {
           soundCtrl = START;
           giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);;
          }
        }
        //NOWPLAY
        if(Menu == NOWPLAY){
          if(usEventCode == L_CENTER){
              //back ROOT
              Menu = ROOT;
          }else if(usEventCode == S_CENTER){
            soundCtrl=PAUSE_PLAY;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
          }else if(usEventCode == S_RIGHT){
            soundCtrl = VLM_UP;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
            volume +=2;
          }else if(usEventCode == S_LEFT){
            soundCtrl = VLM_DOWN;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
            volume -=2;
          }else if(usEventCode == S_UP){
            //Play next song
            sFilePath = nextFile();
            soundCtrl = START;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
          }else if(usEventCode == S_DOWN){
            //Play previous song
            sFilePath = preFile();
            soundCtrl = START;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
          }
        }else if(Menu == BROWSE){
          if(usEventCode == L_CENTER){
              //back ROOT/parent's Dir
            if(BackParentDir())
              Menu = ROOT;
          }else if(usEventCode == S_CENTER){
             //open dir if it's dir, else file and play;
            ListFiles();
            Menu = NOWPLAY;
            ChooseItem = NOWPLAY;
          }else if(usEventCode == S_UP){
            PreItem();
          }else if(usEventCode == S_DOWN){
            NextItem();
          }else if(usEventCode == L_DOWN || usEventCode == L_UP){
            OpenChooseDir();
          }
        }else if(Menu == SETTING){
          if(usEventCode == L_CENTER){
              //back ROOT
              Menu = ROOT;
          }
        }else if(Menu == RECORD){
          if(usEventCode == L_CENTER){
              //back ROOT
              Menu = ROOT;
          }
        }else if(Menu == ROOT){
          if(usEventCode == S_CENTER){
            //enter choose item;
            Menu = ChooseItem;
            if(Menu==BROWSE)
              ListDirItems();
          }else if(usEventCode == S_UP){
              //next item
            ChooseItem++;
            if(ChooseItem > RECORD)
              ChooseItem = NOWPLAY;
          }else if(usEventCode == S_DOWN){
              //previous item
            ChooseItem--;
            if(ChooseItem < NOWPLAY)
              ChooseItem = RECORD;
          } 
        }
      }
    }
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
  
  
  if(xQueueCreate( (signed portCHAR *)cPlayerCtrlQueueBuffer,PLAYER_CTRL_BUFFER_SIZE,
                  PLAYER_CTRL_Q_LENGTH, PLAYER_CTRL_Q_ITEM_SIZE, &xPlayerCtrlQueue ) != pdPASS)
  {
                    //FAIL;
                    return(1);
  }
  //
  // Create the InGate task.
  //
  if(xTaskCreate(PlayerCtrlTask, (signed portCHAR *)"pro",
                   (signed portCHAR *)PlayerCtrlTaskStack,
                   sizeof(PlayerCtrlTaskStack), NULL, PRIORITY_PLAYER_CTRL_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}