/**
* @file	process_event_task.c
* @brief
*
* Get events from user via buttons.
* Process states (LCD off/on, sleep, run)
* User interface (LCD display menu, items)
* Control play_task
*/
#include "detect_event_task.h"
#include "process_event_task.h"
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

/**
*
*/
char process_event_task_init(void){
  return 0;
}