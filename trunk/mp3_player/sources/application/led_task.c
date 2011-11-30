/**
*@file led_task.c
*@brief create led task
*
*Blink LED in 1s
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
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "idle_task.h"
#include "led_task.h"
#include "priorities.h"

//*****************************************************************************
//----------DEFINES
//*****************************************************************************

//*****************************************************************************
//----------VARIABLES
//*****************************************************************************
/**
*The stack for the LED toggle task.
*/
static unsigned long g_pulLEDTaskStack[128];
/**
* The amount of time to delay between toggles of the LED.
*/
unsigned long g_ulLEDDelay = 500;
//*****************************************************************************
/**
* @fn static void LEDTask(void *pvParameters)
* @brief  implement LED task
*
*  This task simply toggles the user LED at a 1 Hz rate
* @param  pvParameters
* @retval  None
*/
//*****************************************************************************
static void
LEDTask(void *pvParameters)
{
    portTickType ulLastTime;
    /**
    * Get the current tick count.
    */
    ulLastTime = xTaskGetTickCount();
    /**
    *Loop forever.
    */
    while(1)
    {
        /**
        *Turn on the user LED.
        */
        ROM_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_PIN_1);

        /**
        *Wait for the required amount of time.
        */
        xTaskDelayUntil(&ulLastTime, g_ulLEDDelay);

        /**
        * Turn off the user LED.
        */
        ROM_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1, 0);

        /**
        * Wait for the required amount of time.
        */
        xTaskDelayUntil(&ulLastTime, g_ulLEDDelay);
    }
}
//*****************************************************************************
/**
* @fn unsigned long LEDTaskInit(void)
* @brief  Initializes the LED task.
*
* Initialize hardware for task and create task
* @retval  0 if successful, else error code
*/
//*****************************************************************************
unsigned long
LEDTaskInit(void)
{
    /**
    *Initialize the GPIO used to drive the user LED.
    */
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    //! Create the LED task.
    if(xTaskCreate(LEDTask, (signed portCHAR *)"LED",
                   (signed portCHAR *)g_pulLEDTaskStack,
                   sizeof(g_pulLEDTaskStack), NULL, PRIORITY_LED_TASK,
                   NULL) != pdPASS)
    {
        return(1);
    }
    TaskCreated();
    /*! Success.*/
    return(0);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************