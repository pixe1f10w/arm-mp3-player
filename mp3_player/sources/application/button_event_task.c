/**
* @file event_task.c
* @brief
*
*
*/
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "third_party/fatfs/src/diskio.h"
#include "priorities.h"
#include "conf.h"
#include "button_event_task.h"
//*****************************************************************************
//----------DEFINES
//*****************************************************************************
#define BUTTON_PORT    GPIO_PORTJ_BASE
#define SYSCTL_BUTTON_PORT  SYSCTL_PERIPH_GPIOJ
#define UP_BUTTON      GPIO_PIN_3
#define DOWN_BUTTON    GPIO_PIN_7
#define RIGHT_BUTTON   GPIO_PIN_6
#define LEFT_BUTTON    GPIO_PIN_4
#define CENTER_BUTTON  GPIO_PIN_5

#define BUTTON_EVENT_QUEUE_LENGTH 1
#define BUTTON_EVENT_QUEUE_ITEM_SIZE 0
#define BUTTON_EVENT_BUFFER_SIZE    ( ( BUTTON_EVENT_QUEUE_LENGTH * BUTTON_EVENT_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )

//*****************************************************************************
//----------VARIABLES
//*****************************************************************************
portCHAR cButtonEventQueueBuffer[ BUTTON_EVENT_BUFFER_SIZE ];
xQueueHandle  xButtonEventQueue;
// The stack for the Button Event task.
static unsigned long ButtonEventTaskStack[128];
// The period of the Button Event task.
unsigned long ButtonEventDelay = 100;
long Button =0x0ff;
long OldButton =0x0ff;
char ButtonEvent = 0xff;
char Delay =0;
/**
*
*/
char takeButtonEvntCode(portTickType Timeout){
  if(xQueueReceive(xButtonEventQueue, NULL, Timeout) == pdPASS){
    return ButtonEvent;
  }
  return 0xff;
}
char giveButtonEvntCode(char e){
  ButtonEvent = e;
  xQueueSend(xButtonEventQueue, NULL, 0);
  return 0;
}
/**
*
*/
void ButtonsIntHandler(void){
  portBASE_TYPE xTaskWoken = pdFALSE;
  GPIOPinIntClear(BUTTON_PORT, UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
  if(Delay > 1){
    Button = GPIOPinRead(BUTTON_PORT, UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
    if((Button& UP_BUTTON)&&((OldButton & UP_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_UP;
      else
        ButtonEvent = S_UP;
    }
    if((Button& DOWN_BUTTON)&&((OldButton & DOWN_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_DOWN;
      else
        ButtonEvent = S_DOWN;
    }
    if((Button& RIGHT_BUTTON)&&((OldButton & RIGHT_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_RIGHT;
      else
        ButtonEvent = S_RIGHT;
    }
    if((Button& LEFT_BUTTON)&&((OldButton & LEFT_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_LEFT;
      else
        ButtonEvent = S_LEFT;
    }
    if((Button& CENTER_BUTTON)&&((OldButton & CENTER_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_CENTER;
      else
        ButtonEvent = S_CENTER;
    }
    OldButton = Button;
    if(((Button & UP_BUTTON)==0)||((Button & DOWN_BUTTON)==0)||((Button & RIGHT_BUTTON)==0)||
        ((Button & LEFT_BUTTON)==0)||((Button & CENTER_BUTTON)==0))
      Delay =0;
    else //Wakeup in_gate task
      xQueueSendFromISR( xButtonEventQueue,NULL,&xTaskWoken);
  }
  /* If the peripheral handler task has a priority higher than the interrupted
  task request a switch to the handler task. */
  taskYIELD_FROM_ISR( xTaskWoken );
}
/**
*
*/
static void
ButtonEventTask(void *pvParameters)
{
  portTickType ulLastTime;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
  while(1){
    //
    // Call the FatFs tick timer.
    //
    disk_timerproc();
    
    if(Delay > 5)
      Delay = 10;
    else
      Delay++;
    //
    // Wait for the required amount of time.
    //
    xTaskDelayUntil(&ulLastTime, 100);
  }
}
/**
*
*/
char initButtonEventTask(void){
  //setup interrupt
  SysCtlPeripheralEnable(SYSCTL_BUTTON_PORT);
  //J7 for external interrupts
  GPIOPinTypeGPIOInput(BUTTON_PORT, UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
  GPIOIntTypeSet(BUTTON_PORT,UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON,GPIO_BOTH_EDGES);
  //
  // Enable the interrupts.
  //
  GPIOPinIntEnable(BUTTON_PORT,UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
  IntEnable(INT_GPIOJ);
  //Create Queues (semaphore)
  if(xQueueCreate( (signed portCHAR *)cButtonEventQueueBuffer,BUTTON_EVENT_BUFFER_SIZE,
                  BUTTON_EVENT_QUEUE_LENGTH, BUTTON_EVENT_QUEUE_ITEM_SIZE, &xButtonEventQueue ) != pdPASS){
                    //FAIL;
                    return(2);
                  }
  //
  // Create the InGate task.
  //
  if(xTaskCreate(ButtonEventTask, (signed portCHAR *)"button",
                   (signed portCHAR *)ButtonEventTaskStack,
                   sizeof(ButtonEventTaskStack), NULL, PRIORITY_BUTTON_EVENT_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}