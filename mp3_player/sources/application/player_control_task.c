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
#include <string.h>

#include "driverlib/flash.h"
#include "driverlib/udma.h"
#include "driverlib/i2s.h"
#include "drivers/sound.h"
#include "button_event_task.h"
#include "sound_player_task.h"
#include "conf.h"
#include "file_management.h"
#include "lcd_print.h"
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


/**
* menu
*/
#define MENU            0
#define ITEM            1
unsigned char Root = MENU;
unsigned char ChooseItem = NOWPLAY;
//setting: storage info, key sound, sleep time, usb/sd, default
unsigned char key_sound = 0;
/**time to sleep*/
unsigned char sleep_time = 0;
//browse: play, enter, back

//record: enter, pause, exit

//nowplay
unsigned char clock=0;
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
  //DisplayMenu(0x03);
    //
    // Loop forever.
    //
  while(1){
    //get event from button
    if( xQueueReceive(xPlayerCtrlQueue, (unsigned char*)&usEventCode,100)==pdPASS)
    {
      //Events from USB
      if(usEventCode == STOP_PLAYER)
      {
        PlayState=PAUSE_STATE;
        usStopPlayerCtrl = 1;//*Stop Player Ctrl
        soundCtrl = STOP;//*Stop Sound Player
        giveSoundCtrlEvent((unsigned char*)&soundCtrl,portMAX_DELAY);
      }
      else if (usEventCode == START_PLAYER)
      {
        usStopPlayerCtrl = 0;//*Start Player Ctrl
        Root = MENU;
        ChooseItem = NOWPLAY;
        DisplayMenu(0x03);
      }
      
      //USB not pluged in
      if(usStopPlayerCtrl == 0)
      {
        //Play sound Button when button pressed
        if(usEventCode < 10 && SoundButton)
        {
          soundCtrl = PLAY_BTN_SND;
          giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
        }
        //Events from Sound Player
        if(usEventCode==PLAY_DONE)
        {
          /**Play Mode: Normal*/
          //TODO Play next song
          
          if(PlayMode == 0){//normal
            sFilePath = nextFile();
            if(ucFileIndex!=0)
            {
             soundCtrl = START;
             giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);;
            }
          }else if(PlayMode == 1)//Single
          {
            ;
          }else if(PlayMode ==2)//repeate
          {
            soundCtrl = START;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);;
          }else if(PlayMode==3)//loop
          {
            sFilePath = nextFile();
            soundCtrl = START;
            giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);;
          }
          //
          if(ChooseItem == NOWPLAY)
          {
            NowPlay(STATE_UPDATE);
            NowPlay(SELECTED_UPDATE);
          }
        }
        
        //Events form user buttons
        if(Root == ITEM)
        {
          if(ChooseItem == NOWPLAY){
            if(usEventCode == L_CENTER)
            {
              Root = MENU;//*back ROOT
              DisplayMenu(0x03);
            }else if(usEventCode == S_CENTER)
            {
              soundCtrl=PAUSE_PLAY;
              giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
              NowPlay(STATE_UPDATE);
              
            }else if(usEventCode == S_RIGHT)
            {     
              soundCtrl = VLM_UP;
              giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
              NowPlay(VOLUME_UPDATE);
            }
            else if(usEventCode == S_LEFT)
            {
              soundCtrl = VLM_DOWN;
              giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
              NowPlay(VOLUME_UPDATE);
            }
            else if(usEventCode == S_UP)
            {
              //Play next song
              sFilePath = preFile();
              NowPlay(SELECTED_UPDATE);
              
              soundCtrl = START;
              giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
            }
            else if(usEventCode == S_DOWN)
            {
              //Play previous song
              sFilePath = nextFile();
              NowPlay(SELECTED_UPDATE);
              
              soundCtrl = START;
              giveSoundCtrlEvent((unsigned char*)&soundCtrl,100);
            }
          }
          else if(ChooseItem == BROWSE)
          {
            if(usEventCode == L_CENTER)
            {
                //back ROOT/parent's Dir
              vTaskSuspendScheduler();
              if(BackParentDir())
              {
                Root = MENU;
                DisplayMenu(0x03);
              }
              else
                Browse(BROWSE_UPDATE);
              xTaskResumeScheduler();
            }
            else if(usEventCode == S_CENTER)
            {
               //open dir if it's dir, else file and play;
              vTaskSuspendScheduler();
              ListFiles();
              xTaskResumeScheduler();
              ChooseItem = NOWPLAY;
              NowPlay(0x1f);
            }
            else if(usEventCode == S_UP){
              PreItem();
              Browse(BROWSE_UPDATE);
            }else if(usEventCode == S_DOWN){
              NextItem();
              Browse(BROWSE_UPDATE);
            }else if(usEventCode == L_DOWN || usEventCode == L_UP){
              vTaskSuspendScheduler();
              OpenChooseDir();
              xTaskResumeScheduler();
              Browse(BROWSE_UPDATE);
            }
          }else if(ChooseItem == SETTINGS){
            if(usEventCode == L_CENTER){
                Root = MENU;//*back MENU
                DisplayMenu(0x03);
            }else if(usEventCode == S_CENTER)
            {
              if(Selected == PLAY_MODE)
              {
                PlayMode++;
                if(PlayMode >3)
                  PlayMode =0;
              }else if(Selected == BUTTON_SOUND)
              {
                SoundButton++;
                if(SoundButton>1)
                  SoundButton =0;
              }
              Settings(SETTING_MODE);
            }
            else if(usEventCode == S_UP)
            {
              Selected++;
              if(Selected > 1)
                Selected=0;
              Settings(SETTING_MODE);
            }else if(usEventCode == S_DOWN)
            {
              Selected--;
              if(Selected > 1)
                Selected=1;
              Settings(SETTING_MODE);
            }
          }else if(ChooseItem == INFO){
            if(usEventCode == L_CENTER){
                Root = MENU;//*back MENU
                DisplayMenu(0x03);
            }
          }
        }
        else if(Root == MENU){
          if(usEventCode == S_CENTER){
            Root = ITEM;//enter choose item;
            if(ChooseItem==NOWPLAY)
            {
              NowPlay(0x1f);
            }
            else if(ChooseItem==BROWSE)
            {
              vTaskSuspendScheduler();
              ListDirItems();
              xTaskResumeScheduler();
              Browse(0x3);
            }
            else if(ChooseItem==SETTINGS)
            {
              Settings(0x03);
            }
            else if(ChooseItem==INFO)
            {
              Info(0x03);
            }
          }else if(usEventCode == S_UP){
            ChooseItem--;//previous item
            if(ChooseItem > INFO)
              ChooseItem = INFO;
            DisplayMenu(MENU_SELECT_UPDATE);
          }else if(usEventCode == S_DOWN){
            ChooseItem++;//next item
            if(ChooseItem > INFO)
              ChooseItem = NOWPLAY;
            DisplayMenu(MENU_SELECT_UPDATE);
          } 
        }
      }else{
        //USB mass storage
        USBTransfer();
      }
        
    }
    if(Root == ITEM)
    {
        if(ChooseItem == NOWPLAY){
            //Update time playing
            if(clock)
            {
              NowPlay(TIME_UPDATE);
              clock=0;
            }
        }
     }
  }
}
/**
*
*/
char initPlayerControlTask(void){
  
  if(InitFileManagement())
    return 1;
  initLCD();
  
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