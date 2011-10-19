#ifndef _PLAY_TASK_H_
#define _PLAY_TASK_H_
//Control play
#define PAUSE_PLAY          0
#define NEXT_SONG           1
#define PRE_SONG            3

void playCtlEvent(char a);
void restore_format(void);
char play_task_init(void);
#endif //_PLAY_TASK_H_