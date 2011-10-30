#ifndef _SOUND_PLAYER_TASK_H_
#define _SOUND_PLAYER_TASK_H_

portBASE_TYPE giveSoundCtrlEvent(portTickType d,char *e);
portBASE_TYPE takePlayEventCode(portTickType d,char *e);
char initSoundPlayerTask(void);
#endif //_SOUND_PLAYER_TASK_H_