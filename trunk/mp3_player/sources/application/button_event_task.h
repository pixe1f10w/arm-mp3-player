#ifndef _BUTTON_EVENT_TASK_H_
#define _BUTTON_EVENT_TASK_H_

/**
*
*/
portBASE_TYPE  takeButtonEvntCode(portTickType Timeout,unsigned char *x);
char giveButtonEvntCode(unsigned char e);
char initButtonEventTask(void);
#endif //_BUTTON_EVENT_TASK_H_