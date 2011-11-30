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
#include <string.h>
#include "driverlib/flash.h"
#include "driverlib/udma.h"
#include "driverlib/i2s.h"
#include "drivers/sound.h"

#include "conf.h"
#include "audio_codec.h"
#include "file_management.h"
#include "player_control_task.h"
#include "lcd_print.h"
#include "wav_decoder_task.h"

#include "sound_player_task.h"
#include "mp3_decoder_task.h"
#include "midmad.h"
/**
* Define Queue receiving control event from external
*/
#define SOUND_CTRL_Q_LENGTH       2 //1 Play button, 1 data control
#define SOUND_CTRL_Q_ITEM_SIZE    1
#define SOUND_CTRL_BUFFER_SIZE    ( ( SOUND_CTRL_Q_LENGTH *SOUND_CTRL_Q_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
static portCHAR cSoundCtrlQueueBuffer[ SOUND_CTRL_BUFFER_SIZE ];
static xQueueHandle  xSoundCtrlQueue;

/**File name for input*/
extern char *sFilePath;

/** The stack for the Play task.*/
static unsigned long SoundPlayerTaskStack[128];
/**The period of the Play task.*/
unsigned long SoundPlayerDelay = 100;
/**Extension of file currently*/
static char FileExt = WAV_FILE;
/**Save Events for processing*/
static unsigned char SoundCtrlEvent;
/**Output*/
static unsigned char ucOutputEvent;
/**States switching between playing song and sound button*/
#define PLAY_SONG_STATE   1
#define PLAY_BUTTON_STATE 2
static unsigned long PlayButtonState = PLAY_SONG_STATE;
/**States of Sound Player*/
#define STOP_SONG         0
#define START_SONG        1
#define PLAY_SONG         2
#define PAUSE_SONG        3
static unsigned long PlaySongState = STOP_SONG;

unsigned long (* GetStatusPlay)(void);
void (*StartPlay)(void);
void (*PausePlay)(void);
void (*ContinuePlay)(void);
void (*StopPlay)(void);
/**
* For external call that want to control Sound Play
*/
portBASE_TYPE giveSoundCtrlEvent(unsigned char *data, portTickType timeout){
  return xQueueSend(xSoundCtrlQueue, data, timeout);
}

/**
* Main Task of Sound Player
*/
static void
SoundPlayerTask(void *pvParameters){
  portTickType ulLastTime;

    //
    // Loop forever.
    //
  ulLastTime=xTaskGetTickCount( );
  while(1){
    if(xQueueReceive(xSoundCtrlQueue,&SoundCtrlEvent, 0)==pdPASS)
    {
      //Play sound Button
      if(SoundCtrlEvent == PLAY_BTN_SND)
        PlayButtonState = PLAY_BUTTON_STATE;
      //Volume up/Volume down
      else if(SoundCtrlEvent == VLM_UP)
      {
        VolumeUp();
      }
      else if(SoundCtrlEvent == VLM_DOWN)
      {
        VolumeDown();
      }
      //Pause or Play
      else if(SoundCtrlEvent == PAUSE_PLAY)
      {
        if(PlaySongState == PAUSE_SONG || PlaySongState == STOP_SONG)
          SoundCtrlEvent = PLAY;
        else if(PlaySongState ==PLAY_SONG)
          SoundCtrlEvent = PAUSE;
      }
      ////////////////////////////////////
      //START playing the song from any state
      if(SoundCtrlEvent == START)
      {
        
        PlaySongState = START_SONG;
      }
      //===PLAYING SONG state==============
      else if(PlaySongState == PLAY_SONG)
      {
        //STOP playing the song
        if(SoundCtrlEvent == STOP)
        {
          StopPlay();
          PlaySongState = STOP_SONG;
          PlayState=PAUSE_STATE;
        }
        //PAUSE playing the song
        else if(SoundCtrlEvent == PAUSE)
        {
          PlaySongState = PAUSE_SONG;
          PlayState=PAUSE_STATE;
          PausePlay();
        }
      }
      //STOP state
      else if(PlaySongState == STOP_SONG)
      {
        
        if(SoundCtrlEvent == PLAY)
          //Switch to START_SONG
          PlaySongState = START_SONG;
      }
      //PAUSE state
      else if(PlaySongState == PAUSE_SONG){
        if(SoundCtrlEvent == PLAY)
        {
          ContinuePlay();
          PlaySongState = PLAY_SONG;
          PlayState=PLAY_STATE;
        }
        else if(SoundCtrlEvent == STOP)
        {
          //Switch to STOP_SONG
          StopPlay();
          PlaySongState = STOP_SONG;
          PlayState=PAUSE_STATE;
        }
      }
    }
    if(PlayButtonState == PLAY_SONG_STATE){
      //===PLAY_SONG==============
      if(PlaySongState == PLAY_SONG){
        //-Enter critical section
         //vTaskSuspendScheduler();
        //update buffer for play;
         if(GetStatusPlay()==MP3_STOP)
         {
           //PLAY_DONE file
           PlaySongState = STOP_SONG;
           PlayState=PAUSE_STATE;
           //close current file
           f_close(&sSongFileObject);
           //PlayEvent
           ucOutputEvent = PLAY_DONE;
           givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,portMAX_DELAY);
           
         }
         //-Exit critical section
         //xTaskResumeScheduler();
      }//==========START_SONG===============
      else if(PlaySongState == START_SONG){
        if(StopPlay)
          StopPlay();
        if(GetStatusPlay)
          while(GetStatusPlay()!=MP3_STOP);
         xTaskDelay(10);
         
        FileExt=CheckExtension(sFilePath);
        if(FileExt == WAV_FILE)
        {
          GetStatusPlay=GetStatusWAVPlay;
          StartPlay=StartWAVPlay;
          PausePlay=PauseWAVPlay;
          ContinuePlay=ContinueWAVPlay;
          StopPlay=StopWAVPlay;
          //
          StartPlay();
          PlaySongState = PLAY_SONG;
          PlayState=PLAY_STATE;
        }
        else if(FileExt == MP3_FILE)
        {
          GetStatusPlay=GetStatusMP3Play;
          StartPlay=StartMP3Play;
          PausePlay=PauseMP3Play;
          ContinuePlay=ContinueMP3Play;
          StopPlay=StopMP3Play;
          //
          StartPlay();
          PlaySongState = PLAY_SONG;
          PlayState=PLAY_STATE;
        }else{
          PlaySongState = STOP_SONG;
           PlayState=PAUSE_STATE;
           //PlayEvent
           ucOutputEvent = FORMAT_ERR;
           givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,portMAX_DELAY);
        }
      }
      else if(PlaySongState == STOP_SONG)
      {
        ;
      }
    }else if(PlayButtonState == PLAY_BUTTON_STATE){
      PlayButtonState = PLAY_SONG_STATE;
    }
    xTaskDelayUntil(&ulLastTime,SoundPlayerDelay);
  }
}
/**
* Intialize, create all components for Sound Player Task
*/
char initSoundPlayerTask(void){
  //Intialize Hardware controled  
  initAudioCodec(VolumeValue);
  
  //Create Queue
  if(xQueueCreate( (signed portCHAR *)cSoundCtrlQueueBuffer,SOUND_CTRL_BUFFER_SIZE,
                  SOUND_CTRL_Q_LENGTH, SOUND_CTRL_Q_ITEM_SIZE, &xSoundCtrlQueue ) != pdPASS)
  {
     return(1);//Fail
  }
  //
  // Create the Play task.
  //
  if(xTaskCreate(SoundPlayerTask, (signed portCHAR *)"play",
                   (signed portCHAR *)SoundPlayerTaskStack,
                   sizeof(SoundPlayerTaskStack), NULL, PRIORITY_SOUND_PLAYER_TASK,
                   NULL) != pdPASS)
  {
    return(1);//Fail
  }
  //
  if(initWAVDecoderTask())
    return 1;
  if(initMP3DecoderTask())
    return 1;
  return 0;//Success
}