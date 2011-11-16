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

#include "conf.h"
#include "audio_play.h"
#include "player_control_task.h"
#include "lcd_print.h"
#include "sound_player_task.h"
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
/**File Object for open/read file*/
static FIL g_sSongFileObject;
/** The wav file header information.*/
static SoundInfoHeader g_sSongHeader;
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
  /** Test Events
    PlayButtonState = PLAY_BUTTON_STATE;
    PlaySongState = START_SONG;
    SoundCtrlEvent = PLAY;
  */
    //
    // Loop forever.
    //
  while(1){
    if(xQueueReceive(xSoundCtrlQueue,&SoundCtrlEvent, 0)==pdPASS)
    {
      //Play sound Button
      if(SoundCtrlEvent == PLAY_BTN_SND)
        PlayButtonState = PLAY_BUTTON_STATE;
      //Volume up
      else if(SoundCtrlEvent == VLM_UP)
        SoundVolumeUp(2);
      //Volume down
      else if(SoundCtrlEvent == VLM_DOWN)
        SoundVolumeDown(2);
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
          PlaySongState = STOP_SONG;
        }
        //PAUSE playing the song
        else if(SoundCtrlEvent == PAUSE)
        {
          PlaySongState = PAUSE_SONG;
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
          PlaySongState = PLAY_SONG;
        else if(SoundCtrlEvent == STOP)
        {
          //close the song
          f_close(&g_sSongFileObject);
          //Switch to STOP_SONG
          PlaySongState = STOP_SONG;
        }
      }
    }
    if(PlayButtonState == PLAY_SONG_STATE){
      //===PLAY_SONG==============
      if(PlaySongState == PLAY_SONG){
        //-Enter critical section
         //vTaskSuspendScheduler();
        //update buffer for play;
         if( UpdateBufferForPlay(&g_sSongFileObject, &g_sSongHeader,FileExt)==0)
         {
           //PLAY_DONE file
           PlaySongState = STOP_SONG;
           //close current file
           f_close(&g_sSongFileObject);
           //PlayEvent
           ucOutputEvent = PLAY_DONE;
           givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,portMAX_DELAY);
           
         }
         //-Exit critical section
         //xTaskResumeScheduler();
      }//==========START_SONG===============
      else if(PlaySongState == START_SONG){
        // No longer playing audio.
        g_ulFlags &= ~BUFFER_PLAYING;
        //close the current song
        f_close(&g_sSongFileObject);
        //===Open song
        FileExt = CheckExtension(sFilePath);
        if(FileExt == WAV_FILE){
          if(OpenWavFile(&g_sSongFileObject,sFilePath,&g_sSongHeader,1) != FR_OK ){
            //announce opening fail
            ucOutputEvent = OPEN_ERR;
            givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,100);
            //switch STOP
            
            PlaySongState = STOP_SONG;
          }
        }else if(FileExt == MP3_FILE){
          if(OpenMp3File(&g_sSongFileObject, sFilePath,&g_sSongHeader) != FR_OK ){
            //announce opening fail
            ucOutputEvent = OPEN_ERR;
            givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,100);
            //switch STOP
            PlaySongState = STOP_SONG;
         }
        }else{
          //annouce not format correctly
          ucOutputEvent = FORMAT_ERR;
          givePlayerCtrlEvent((unsigned char *)&ucOutputEvent,100);
          //switch STOP
          PlaySongState = STOP_SONG;
        }
        
        if(PlaySongState == START_SONG)
        {
          
          //setting for codec: bits/s, Hz, channels
          setupAudioCodecForSong(g_sSongHeader);
          //set state for PLAY
          g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
          //switch to PLAY the song
          PlaySongState = PLAY_SONG;
          StartBufferSignal(0);
        }
      }
    }else if(PlayButtonState == PLAY_BUTTON_STATE){
      //-Enter critical section
      vTaskSuspendScheduler();
      playButtonSound(g_sSongHeader);
      //-Exit critical section
      xTaskResumeScheduler();
      
      //Switch to PLAY_SONG
      PlayButtonState = PLAY_SONG_STATE;
      StartBufferSignal(0);
    }
    WaitBufferSignal(SoundPlayerDelay);
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
  
  return 0;//Success
}