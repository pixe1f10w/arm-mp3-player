#ifndef _PLAY_TASK_H_
#define _PLAY_TASK_H_
//Control play
#define PLAY    0x0000001
void toggle_play_ctl_flags(unsigned long mask);
void signal_for_play(void);
char play_task_init(void);
#endif //_PLAY_TASK_H_