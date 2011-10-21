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
#include "support_play_file.h"
#include "driverlib/flash.h"
#include "driverlib/udma.h"
#include "driverlib/i2s.h"
#include "drivers/sound.h"

#include "conf.h"
#include "sound_player_task.h"
//*****************************************************************************
//*****************************************************************************

#define SOUND_CTRL_QUEUE_LENGTH 1
#define SOUND_CTRL_QUEUE_ITEM_SIZE 0
#define SOUND_CTRL_BUFFER_SIZE    ( ( SOUND_CTRL_QUEUE_LENGTH * SOUND_CTRL_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
portCHAR cSoundCtrlQueueBuffer[ SOUND_CTRL_BUFFER_SIZE ];
xQueueHandle  xSoundCtrlQueue;
#define PLAY_EVNT_QUEUE_LENGTH 1
#define PLAY_EVNT_QUEUE_ITEM_SIZE 0
#define PLAY_EVNT_BUFFER_SIZE    ( ( PLAY_EVNT_QUEUE_LENGTH * PLAY_EVNT_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
portCHAR cPlayEvntQueueBuffer[ PLAY_EVNT_BUFFER_SIZE ];
xQueueHandle  xPlayEvntQueue;
extern char *Filename;
//******************************************************************************
//
// Storage for the filename listbox widget string table.
//
//******************************************************************************


static FIL g_sSongFileObject,g_sButtonFileObject;
// The wav file header information.
static SoundInfoHeader g_sSongHeader;
static SoundInfoHeader g_sButtonHeader;
// The stack for the Play task.
static unsigned long SoundPlayerTaskStack[128];
// The period of the Play task.
unsigned long SoundPlayerDelay = 100;
char FileExt = WAV_FILE;
char PlayEvent,SoundCtrlEvent;
#define STOP_BUTTON_SOUND 0
#define OPEN_BUTTON_SOUND 1
#define PLAY_BUTTON_SOUND 2
#define RETORE_SONG       3
unsigned long PlayButtonFlags = STOP_BUTTON_SOUND;
#define STOP_SONG         0
#define START_SONG        1
#define PLAY_SONG         2
#define PAUSE_SONG        3
unsigned long PlaySongFlags = STOP_SONG;
/**
*
*/
void giveSoundCtlEvent(char a){
  SoundCtrlEvent = a;
  xQueueSend(xSoundCtrlQueue, NULL, 0);
}
void givePlayEventCode(char a){
  PlayEvent = a;
  xQueueSend(xPlayEvntQueue, NULL, 0);
}
/**
*
*/
static void
SoundPlayerTask(void *pvParameters){
  //portTickType ulLastTime;
    //
    // Get the current tick count.
    //
    //ulLastTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
  while(1){
    //get event
    if(xQueueReceive(xSoundCtrlQueue, NULL, 0)==pdPASS){
      if(SoundCtrlEvent == PAUSE_PLAY){
        if(PlaySongFlags == PLAY_SONG)
          PlaySongFlags = PAUSE_SONG;
        else if(PlaySongFlags == PAUSE_SONG)
          PlaySongFlags = PLAY_SONG;
      }else if (SoundCtrlEvent == START){
        PlaySongFlags = START_SONG;
      }else
        PlayButtonFlags = OPEN_BUTTON_SOUND;
    }
    if(PlayButtonFlags == STOP_BUTTON_SOUND){
      if(PlaySongFlags == PLAY_SONG){
        //-Enter critical section
         vTaskSuspendScheduler();
        //update buffer for play;
         if( UpdateBufferForPlay(&g_sSongFileObject, &g_sSongHeader,FileExt)==0){
          //end file
           PlaySongFlags = STOP_SONG;
         }
         //-Exit critical section
          xTaskResumeScheduler();
      }else if(PlaySongFlags == PAUSE_SONG){
        ;
      }else if(PlaySongFlags == START_SONG){
        //close current file
        CloseFile(&g_sSongFileObject);
        //open song
        FileExt = CheckExtension(Filename);
        if(FileExt == WAV_FILE){
          if(OpenWavFile(&g_sSongFileObject,Filename,&g_sSongHeader) == FR_OK ){
            //setting for codec: bits/s, Hz, channels
            SoundSetFormat(g_sSongHeader.ulSampleRate,g_sSongHeader.usBitsPerSample,
                         g_sSongHeader.usNumChannels);
            //set state for PLAY
            g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
            //switch to PLAY the song
            PlaySongFlags = PLAY_SONG;
          }else{
            //announce opening fail
            givePlayEventCode(OPEN_FAIL);
            //switch STOP
            PlaySongFlags = STOP_SONG;
          }
        }else if(FileExt == MP3_FILE){
          if(OpenMp3File(&g_sSongFileObject, Filename,&g_sSongHeader) == FR_OK ){
            //setting for codec: bits/s, Hz, channels
            SoundSetFormat(g_sSongHeader.ulSampleRate,g_sSongHeader.usBitsPerSample,
                         g_sSongHeader.usNumChannels);
            //set state for PLAY
            g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
            //switch to PLAY the song
            PlaySongFlags = PLAY_SONG;
          }else{
            //announce opening fail
            givePlayEventCode(OPEN_FAIL);
            //switch STOP
            PlaySongFlags = STOP_SONG;
          }
        }else{
          //annouce not format correctly
          givePlayEventCode(FORMAT_ERR);
          //switch STOP
          PlaySongFlags = STOP_SONG;
        }
      }
    }else{
      if(PlayButtonFlags == OPEN_BUTTON_SOUND){
        //open button sound
        if(OpenWavFile(&g_sButtonFileObject,"0:/SYS/button.wav",&g_sButtonHeader)==FR_OK){
          
          // Set the format of the playback in the sound driver.
          SoundSetFormat(g_sButtonHeader.ulSampleRate,g_sButtonHeader.usBitsPerSample,
                       g_sButtonHeader.usNumChannels);
          g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY|BUFFER_PLAYING);
          PlayButtonFlags = PLAY_BUTTON_SOUND;
        }else{
          //stop the button sound
          PlayButtonFlags = RETORE_SONG;
        }
      }else if(PlayButtonFlags == PLAY_BUTTON_SOUND){
        if(UpdateBufferForPlay(&g_sButtonFileObject, &g_sButtonHeader,WAV_FILE)==0){
          PlayButtonFlags = RETORE_SONG;
        }
      }else if(PlayButtonFlags == RETORE_SONG){
        if(FileExt == WAV_FILE){

            //restore the format of the playback in the sound driver.
            SoundSetFormat(g_sSongHeader.ulSampleRate,g_sSongHeader.usBitsPerSample,
                         g_sSongHeader.usNumChannels);
        }else if(FileExt == MP3_FILE){
          
            //restore the format of the playback in the sound driver.
            SoundSetFormat(g_sSongHeader.ulSampleRate,g_sSongHeader.usBitsPerSample,
                         g_sSongHeader.usNumChannels);
        }
        
        PlayButtonFlags = STOP_BUTTON_SOUND;
      }
    }

    WaitBufferSignal(SoundPlayerDelay);
    //
    // Wait for the required amount of time.
    //
    //xTaskDelayUntil(&ulLastTime, 10);
  }
}
/**
*
*/
char initSoundPlayerTask(void){
    
  init_play_sound();
  //
  if(xQueueCreate( (signed portCHAR *)cSoundCtrlQueueBuffer,SOUND_CTRL_BUFFER_SIZE,
                  SOUND_CTRL_QUEUE_LENGTH, SOUND_CTRL_QUEUE_ITEM_SIZE, &xSoundCtrlQueue ) != pdPASS){
                    //FAIL;
                    while(1);
                  }
  //
  // Create the Play task.
  //
  if(xTaskCreate(SoundPlayerTask, (signed portCHAR *)"play",
                   (signed portCHAR *)SoundPlayerTaskStack,
                   sizeof(SoundPlayerTaskStack), NULL, PRIORITY_SOUND_PLAYER_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}