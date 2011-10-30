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
//*****************************************************************************
//*****************************************************************************

#define PLAY_CTRL_QUEUE_LENGTH 2 //1 Play button, 1 data control
#define PLAY_CTRL_QUEUE_ITEM_SIZE 1
#define PLAY_CTRL_BUFFER_SIZE    ( ( PLAY_CTRL_QUEUE_LENGTH *PLAY_CTRL_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
static portCHAR cPlayCtrlQueueBuffer[ PLAY_CTRL_BUFFER_SIZE ];
static xQueueHandle  xPlayCtrlQueue;
#define PLAY_EVNT_QUEUE_LENGTH 1
#define PLAY_EVNT_QUEUE_ITEM_SIZE 1
#define PLAY_EVNT_BUFFER_SIZE    ( ( PLAY_EVNT_QUEUE_LENGTH * PLAY_EVNT_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )
static portCHAR cPlayEvntQueueBuffer[ PLAY_EVNT_BUFFER_SIZE ];
static xQueueHandle  xPlayEvntQueue;
extern char *Filename;
//******************************************************************************
//
// Storage for the filename listbox widget string table.
//
//******************************************************************************


static FIL g_sSongFileObject,g_sButtonFileObject;
static FATFS g_sFatFs;
// The wav file header information.
static SoundInfoHeader g_sSongHeader;
static SoundInfoHeader g_sButtonHeader;
// The stack for the Play task.
static unsigned long SoundPlayerTaskStack[128];
// The period of the Play task.
unsigned long SoundPlayerDelay = 100;
static char FileExt = WAV_FILE;
static char PlayEvent,SoundCtrlEvent;
#define PLAY_SONG_STATE   1
#define PLAY_BUTTON_STATE 2
static unsigned long PlayButtonState = PLAY_SONG_STATE;
#define STOP_SONG         0
#define START_SONG        1
#define PLAY_SONG         2
#define PAUSE_SONG        3
static unsigned long PlaySongState = STOP_SONG;
/**
*
*/
portBASE_TYPE giveSoundCtrlEvent(portTickType d,char *a){
  return xQueueSend(xPlayCtrlQueue, (char*)a, d);
}
/**
*
*/
static portBASE_TYPE takeSoundCtrlEvent(portTickType d){
  return xQueueReceive(xPlayCtrlQueue, (char*)&SoundCtrlEvent, d);
  //return SoundCtrlEvent;
}
/**
*
*/
portBASE_TYPE takePlayEventCode(portTickType d, char * e){
  return xQueueReceive(xPlayEvntQueue, (char*)e, d);
}
/**
*
*/
static portBASE_TYPE givePlayEventCode(portTickType d,char *e){
  return xQueueSend(xPlayEvntQueue, (char*)e, d);
}
/**
*
*/
static void
SoundPlayerTask(void *pvParameters){
  portTickType ulLastTime;
  unsigned long i=0;
  FRESULT fresult;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
     //
    // Mount the file system, using logical disk 0.
    //
    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
    {
        return;
    }
    //
    // Loop forever.
    //
  //g_ulFlags=(BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY);
    PlayButtonState = PLAY_BUTTON_STATE;
    PlaySongState = START_SONG;
    SoundCtrlEvent = PLAY;
  while(1){
    if(takeSoundCtrlEvent(0)==pdPASS)
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
        if(PlaySongState == PAUSE_SONG)
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
         vTaskSuspendScheduler();
        //update buffer for play;
         if( UpdateBufferForPlay(&g_sSongFileObject, &g_sSongHeader,FileExt)==0){
           //PLAY_DONE file
           PlaySongState = START_SONG;//STOP_SONG;
           //PlayButtonState = PLAY_BUTTON_STATE;
           //close current file
           f_close(&g_sSongFileObject);
           //PlayEvent
           //PlayEvntCode = PLAY_DONE;
           //givePlayEventCode(100,&PlayEvntCode);
         }
         //-Exit critical section
         xTaskResumeScheduler();
      }//==========START_SONG===============
      else if(PlaySongState == START_SONG){
        // No longer playing audio.
        g_ulFlags &= ~BUFFER_PLAYING;
        // Wait for the buffer to empty.
         //while(g_ulFlags != (BUFFER_TOP_EMPTY | BUFFER_BOTTOM_EMPTY));
        //close the current song
        f_close(&g_sSongFileObject);
        //===Open song
        FileExt = CheckExtension("0:/m.wav");
        if(FileExt == WAV_FILE){
          if(OpenWavFile(&g_sSongFileObject,"0:/m.wav",&g_sSongHeader) != FR_OK ){
            //announce opening fail
            //PlayEvntCode = OPEN_FAIL;
           //givePlayEventCode(100,&PlayEvntCode);
            //switch STOP
            
            PlaySongState = STOP_SONG;
          }
        }else if(FileExt == MP3_FILE){
          if(OpenMp3File(&g_sSongFileObject, Filename,&g_sSongHeader) != FR_OK ){
            //announce opening fail
            //PlayEvntCode = OPEN_FAIL;
           //givePlayEventCode(100,&PlayEvntCode);
            //switch STOP
            PlaySongState = STOP_SONG;
         }
        }else{
          //annouce not format correctly
          //PlayEvntCode = FORMAT_ERR;
          //givePlayEventCode(100,&PlayEvntCode);
          //switch STOP
          PlaySongState = STOP_SONG;
        }
        if(PlaySongState == START_SONG){
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
    //
    // Wait for the required amount of time.
    //
    //xTaskDelayUntil(&ulLastTime, 0);
  }
}
/**
*
*/
char initSoundPlayerTask(void){
    
  initAudioCodec(70);
  //
  if(xQueueCreate( (signed portCHAR *)cPlayCtrlQueueBuffer,PLAY_CTRL_BUFFER_SIZE,
                  PLAY_CTRL_QUEUE_LENGTH, PLAY_CTRL_QUEUE_ITEM_SIZE, &xPlayCtrlQueue ) != pdPASS)
  {
                    //FAIL;
                    return(1);
   }
  //
  if(xQueueCreate( (signed portCHAR *)cPlayEvntQueueBuffer,PLAY_EVNT_BUFFER_SIZE,
                  PLAY_EVNT_QUEUE_LENGTH, PLAY_EVNT_QUEUE_ITEM_SIZE, &xPlayEvntQueue ) != pdPASS)
  {
                    //FAIL;
                    return(1);
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