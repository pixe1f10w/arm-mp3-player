#ifndef _SOUND_PLAYER_TASK_H_
#define _SOUND_PLAYER_TASK_H_

#define     MP3_STOP        0x0000
#define     MP3_START       0x0001
#define     MP3_PAUSE       0x0002
#define     MP3_CONTINUE    0x0003


/**
* Define Controls for Sound Player
*/
#define START           11
#define	PLAY    	12
#define	PAUSE           13
#define PAUSE_PLAY      14
#define	STOP            15
#define	VLM_UP          16
#define	VLM_DOWN        17
#define	PLAY_BTN_SND    18
/*
*
*/
portBASE_TYPE giveSoundCtrlEvent(unsigned char *data, portTickType timeout);

char initSoundPlayerTask(void);
#endif //_SOUND_PLAYER_TASK_H_