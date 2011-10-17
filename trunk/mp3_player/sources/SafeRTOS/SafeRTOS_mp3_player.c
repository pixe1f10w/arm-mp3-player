//*****************************************************************************
/**
*@file SafeRTOS_mp3_player.c
*@brief main
*
*Initializ all tasks here.
*@author Nguyen Van Hai
*@date 23/08/2011
*@version 0.0
*@pre init
*@bug None
*@warning None
*@copyright GNU
*/
//*****************************************************************************
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "idle_task.h"
#include "led_task.h"
#include "detect_event_task.h"
#include "process_event_task.h"
#include "play_task.h"
//*****************************************************************************
//
//! The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
//! This hook is called by SafeRTOS when an error is detected.
//
//*****************************************************************************
static void
SafeRTOSErrorHook(xTaskHandle xHandleOfTaskWithError,
                  signed portCHAR *pcNameOfTaskWithError,
                  portBASE_TYPE xErrorCode)
{
    //
    //! A fatal SafeRTOS error was detected, so display an error message.
    //
  
    //
    //! This function can not return, so loop forever.  Interrupts are disabled
    //! on entry to this function, so no processor interrupts will interrupt
    //! this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
//! The parameters used to initialize SafeRTOS.
//
//*****************************************************************************
static xPORT_INIT_PARAMETERS g_sSafeRTOSPortInit =
{
    //
    //! System clock rate.
    //
    80000000,

    //
    //! Scheduler tick rate.
    //
    1000 / portTICK_RATE_MS,

    //
    //! Task delete hook.
    //
    SafeRTOSTaskDeleteHook,

    //
    //! Error hook.
    //
    SafeRTOSErrorHook,

    //
    //! Idle hook.
    //
    SafeRTOSIdleHook,

    //
    //! System stack location.
    //
    0,

    //
    //! System stack size.
    //
    0,

    //
    //! Vector table base.
    //
    0
};

//*****************************************************************************
/**
* @fn int main(void)
* @brief  Initialize SafeRTOS and start the initial set of tasks.
*
* @retval  Never
*/
//*****************************************************************************
int
main(void)
{
    //
    //! Set the clocking to run at 80 MHz from the PLL.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
    //
    //! Enable all GPIO banks.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);

    //
    //! Set the location and size of the system stack.
    //
    g_sSafeRTOSPortInit.pulSystemStackLocation =
        (unsigned portLONG *)(*(unsigned long *)0);
    g_sSafeRTOSPortInit.ulSystemStackSizeBytes = 128 * 4;

    //
    //! Set the location of the vector table.
    //
    g_sSafeRTOSPortInit.pulVectorTableBase =
        (unsigned portLONG *)HWREG(NVIC_VTABLE);

    //
    //! Initialize the SafeRTOS kernel.
    //
    vTaskInitializeScheduler((signed portCHAR *)g_pulIdleTaskStack,
                             sizeof(g_pulIdleTaskStack), 0,
                             &g_sSafeRTOSPortInit);
    
    //
    //! Create the LED task.
    //
    if(LEDTaskInit() != 0)
    {
        while(1)
        {
        }
    }
    
    if(detect_event_task_init() !=0){
      while(1);
    }
    if(process_event_task_init() !=0){
      while(1);
    }
    if(play_task_init() !=0){
      while(1);
    }
    //
    //! Start the scheduler.  This should not return.
    //
    xTaskStartScheduler(pdTRUE);

    //
    //! In case the scheduler returns for some reason, loop
    //! forever.
    //

    while(1)
    {
    }
}
