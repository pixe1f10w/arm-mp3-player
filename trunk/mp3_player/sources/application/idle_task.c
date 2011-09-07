/**
*@file idle_task.c
*@brief create idle task
*
*When another task don't run, idle task will run
*@author Nguyen Van Hai
*@date 23/08/2011
*@version 0.0
*@pre init
*@bug None
*@warning None
*@copyright GNU
*/
//*****************************************************************************
//
//! \addtogroup tasks_api
//! @{
//
//*****************************************************************************
#include "SafeRTOS/SafeRTOS_API.h"
#include "idle_task.h"

/**
* The stack for the idle task.
*/
unsigned long g_pulIdleTaskStack[128];
/**
*The number of tasks that are running.
*/
static unsigned long g_ulTasks;
//*****************************************************************************
/**
* @fn void TaskCreated(void)
* @brief  Increase The number of the created task
*
*This function is called by the application whenever it creates a task.
* @retval  None
*/
//*****************************************************************************
void
TaskCreated(void)
{
    /**
    * Increment the count of running tasks.
    */
    g_ulTasks++;
}
//*****************************************************************************
/**
* @fn void SafeRTOSTaskDeleteHook(xTaskHandle xTaskToDelete)
* @brief  Decrease The number of the created task
*
* This hook is called by SafeRTOS when a task is deleted.
* @param  xTaskToDelete
* @retval  None
*/
//*****************************************************************************
void
SafeRTOSTaskDeleteHook(xTaskHandle xTaskToDelete)
{
    /**
    * Decrement the count of running tasks.
    */
    g_ulTasks--;
}
//*****************************************************************************
/**
* @fn void SafeRTOSIdleHook(void)
* @brief  Idle Hook
*
* This hook is called by the SafeRTOS idle task when no other tasks are
* runnable. Here nothing.
* @retval  None
*/
//*****************************************************************************
void
SafeRTOSIdleHook(void)
{
    ;
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
