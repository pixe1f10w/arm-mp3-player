#ifndef _PLAYER_CONTROL_TASK_H_
#define _PLAYER_CONTROL_TASK_H_

/**
* Define Events from Buttons
*/
#define	S_CENTER	0
#define L_CENTER	1
#define	S_UP    	2
#define	L_UP	        3
#define S_DOWN  	4
#define	L_DOWN  	5
#define	S_RIGHT 	6
#define	L_RIGHT 	7
#define S_LEFT  	8
#define	L_LEFT  	9
/**
* Define Controls for Player Control
*/
#define STOP_PLAYER     10
#define START_PLAYER    11
/**
* Define Events from Sound Player
*/
#define PLAY_DONE       12
#define FORMAT_ERR      13
#define OPEN_ERR        14

extern unsigned char clock;
portBASE_TYPE givePlayerCtrlEvent(unsigned char *data, portTickType timeout);
char get_play_event(void);
char get_volume(void);
char * get_path(void);
char get_mode(void);

char initPlayerControlTask(void);
#endif //_PLAYER_CONTROL_TASK_H_