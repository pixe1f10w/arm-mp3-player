#ifndef _DETECT_EVENT_TASK_H_
#define _DETECT_EVENT_TASK_H_
/**
* Define button event
*/
#define	SHORT_M			0
#define LONG_M			1
#define	SHORT_P_PLUS	2
#define	LONG_P_PLUS		3
#define SHORT_P_MINUS	4
#define	LONG_P_MINUS	5
#define	SHORT_V_PLUS	6
#define	LONG_V_PLUS		7
#define SHORT_V_MINUS	8
#define	LONG_V_MINUS	9
/**
*
*/
char get_event_code(portTickType timeout);
char detect_event_task_init(void);
#endif //_DETECT_EVENT_TASK_H_