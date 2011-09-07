#ifndef _PROCESS_EVENT_TASK_H_
#define _PROCESS_EVENT_TASK_H_
/**
* Define events
*/
//for play_task: play_event
#define	STOP		0
#define START		1
#define	VOLUME		2
#define	SEEK		3
#define NEXT_SONG	4
#define	PRE_SONG	5
#define NOTHING		6
//play mode
#define REPEAT_ALL	1
#define REPEAT_ONE	2
#define RANDOM		3
#define NORMAL		4

char get_play_event(void);
char get_volume(void);
char * get_path(void);
char get_mode(void);

char process_event_task_init(void);
#endif //_PROCESS_EVENT_TASK_H_