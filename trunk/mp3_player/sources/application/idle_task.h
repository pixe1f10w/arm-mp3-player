//*****************************************************************************
/**
*@file idle_task.c
*@brief prototype for idle task
*
*@author Nguyen Van Hai
*@date 23/08/2011
*@version 0.0
*@pre init
*@bug None
*@warning None
*@copyright GNU
*/
//*****************************************************************************

#ifndef __IDLE_TASK_H__
#define __IDLE_TASK_H__

//*****************************************************************************
//
// Prototypes for the idle task.
//
//*****************************************************************************
extern unsigned long g_pulIdleTaskStack[128];
extern void TaskCreated(void);
extern void SafeRTOSTaskDeleteHook(xTaskHandle xTaskToDelete);
extern void SafeRTOSIdleHook(void);

#endif // __IDLE_TASK_H__
