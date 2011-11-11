#ifndef _SOUND_PLAYER_TASK_H_
#define _SOUND_PLAYER_TASK_H_

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