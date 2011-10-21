#ifndef _BUTTON_EVENT_TASK_H_
#define _BUTTON_EVENT_TASK_H_

/**
*
*/
char takeButtonEvntCode(portTickType Timeout);
char giveButtonEvntCode(char e);
char initButtonEventTask(void);
#endif //_BUTTON_EVENT_TASK_H_