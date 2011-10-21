#ifndef _PLAYER_CONTROL_TASK_H_
#define _PLAYER_CONTROL_TASK_H_
/**
* Define events
*/
//for play_task: play_event

//play mode
#define REPEAT_ALL	1
#define REPEAT_ONE	2
#define RANDOM		3
#define NORMAL		4

char get_play_event(void);
char get_volume(void);
char * get_path(void);
char get_mode(void);

char initPlayerControlTask(void);
#endif //_PLAYER_CONTROL_TASK_H_