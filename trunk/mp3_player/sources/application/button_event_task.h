#ifndef _BUTTON_EVENT_TASK_H_
#define _BUTTON_EVENT_TASK_H_

/**
*
*/
portBASE_TYPE  takeButtonEvntCode(portTickType Timeout,char *x);
char giveButtonEvntCode(char e);
char initButtonEventTask(void);
#endif //_BUTTON_EVENT_TASK_H_