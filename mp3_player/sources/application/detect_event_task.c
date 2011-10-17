/**
* @file detect_event_task.c
* @brief
*
* Wait a signal from interrupt
* Process buttons to give event codes. Then, inform to
* process_event_task
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
#include "detect_event_task.h"
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

#define DETECT_EVENT_QUEUE_LENGTH 1
#define DETECT_EVENT_QUEUE_ITEM_SIZE 0
#define DETECT_EVENT_BUFFER_SIZE    ( ( DETECT_EVENT_QUEUE_LENGTH * DETECT_EVENT_QUEUE_ITEM_SIZE ) + portQUEUE_OVERHEAD_BYTES )

//*****************************************************************************
//----------VARIABLES
//*****************************************************************************
portCHAR cDetectEventQueueBuffer[ DETECT_EVENT_BUFFER_SIZE ];
xQueueHandle  xDetectEventQueue;
// The stack for the Detect Event task.
static unsigned long DetectEventTaskStack[128];
// The period of the Detect Event task.
unsigned long DetectEventDelay = 100;
long key =0x0ff;
long old_key =0x0ff;
char event = 0xff;
char delay =0;
/**
*
*/
char get_event_code(portTickType timeout){
  if(xQueueReceive(xDetectEventQueue, NULL, timeout) == pdPASS){
    return event;
  }
  return 0xff;
}
/**
*
*/
void ButtonsIntHandler(void){
  portBASE_TYPE xTaskWoken = pdFALSE;
  GPIOPinIntClear(BUTTON_PORT, UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
  if(delay > 1){
    key = GPIOPinRead(BUTTON_PORT, UP_BUTTON|DOWN_BUTTON|RIGHT_BUTTON|LEFT_BUTTON|CENTER_BUTTON);
    if((key& UP_BUTTON)&&((old_key & UP_BUTTON)==0)){
      if(delay > 5)
        event = LONG_P_PLUS;
      else
        event = SHORT_P_PLUS;
    }
    if((key& DOWN_BUTTON)&&((old_key & DOWN_BUTTON)==0)){
      if(delay > 5)
        event = LONG_P_MINUS;
      else
        event = SHORT_P_MINUS;
    }
    if((key& RIGHT_BUTTON)&&((old_key & RIGHT_BUTTON)==0)){
      if(delay > 5)
        event = LONG_V_PLUS;
      else
        event = SHORT_V_PLUS;
    }
    if((key& LEFT_BUTTON)&&((old_key & LEFT_BUTTON)==0)){
      if(delay > 5)
        event = LONG_V_MINUS;
      else
        event = SHORT_V_MINUS;
    }
    if((key& CENTER_BUTTON)&&((old_key & CENTER_BUTTON)==0)){
      if(delay > 5)
        event = LONG_M;
      else
        event = SHORT_M;
    }
    old_key = key;
    if(((key & UP_BUTTON)==0)||((key & DOWN_BUTTON)==0)||((key & RIGHT_BUTTON)==0)||
        ((key & LEFT_BUTTON)==0)||((key & CENTER_BUTTON)==0))
      delay =0;
    else //Wakeup in_gate task
      xQueueSendFromISR( xDetectEventQueue,NULL,&xTaskWoken);
  }
  /* If the peripheral handler task has a priority higher than the interrupted
  task request a switch to the handler task. */
  taskYIELD_FROM_ISR( xTaskWoken );
}
/**
*
*/
static void
DetectEventTask(void *pvParameters)
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
    
    if(delay > 5)
      delay = 10;
    else
      delay++;
    //
    // Wait for the required amount of time.
    //
    xTaskDelayUntil(&ulLastTime, 100);
  }
}
/**
*
*/
char detect_event_task_init(void){
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
  if(xQueueCreate( (signed portCHAR *)cDetectEventQueueBuffer,DETECT_EVENT_BUFFER_SIZE,
                  DETECT_EVENT_QUEUE_LENGTH, DETECT_EVENT_QUEUE_ITEM_SIZE, &xDetectEventQueue ) != pdPASS){
                    //FAIL;
                    return(2);
                  }
  //
  // Create the InGate task.
  //
  if(xTaskCreate(DetectEventTask, (signed portCHAR *)"button",
                   (signed portCHAR *)DetectEventTaskStack,
                   sizeof(DetectEventTaskStack), NULL, PRIORITY_DETECT_EVENT_TASK,
                   NULL) != pdPASS)
  {
    return(1);
  }
  return 0;
}