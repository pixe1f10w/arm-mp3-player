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
#include "player_control_task.h"
#include "button_event_task.h"
#include "lcd_print.h"
#include "file_management.h"
//*****************************************************************************
//----------DEFINES
//*****************************************************************************
#define BUTTON_PORT         GPIO_PORTJ_BASE
#define SYSCTL_BUTTON_PORT  SYSCTL_PERIPH_GPIOJ
#define UP_BUTTON           GPIO_PIN_1 //PG
#define DOWN_BUTTON         GPIO_PIN_0 //PA
#define RIGHT_BUTTON        GPIO_PIN_5 //PB
#define LEFT_BUTTON         GPIO_PIN_0 //PG
#define CENTER_BUTTON       GPIO_PIN_1 //PA

// The stack for the Button Event task.
static unsigned long ButtonEventTaskStack[128];
// The period of the Button Event task.
static unsigned long ButtonEventDelay = 100;
static unsigned long ButtonA;
static unsigned long OldButtonA;
static unsigned long ButtonB;
static unsigned long OldButtonB;
static unsigned long ButtonG;
static unsigned long OldButtonG;
static unsigned char ButtonEvent;
static unsigned char ucFlag;
static unsigned char Delay =0,i;

/**
*
*/
void GPIOAIntHandler(void){

  GPIOPinIntClear(GPIO_PORTA_BASE, DOWN_BUTTON|CENTER_BUTTON);
  if(Delay > 1){
    ButtonA = GPIOPinRead(GPIO_PORTA_BASE, DOWN_BUTTON|CENTER_BUTTON);
    
    if((ButtonA& DOWN_BUTTON)&&((OldButtonA & DOWN_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_DOWN;
      else
        ButtonEvent = S_DOWN;
    }
    
    if((ButtonA& CENTER_BUTTON)&&((OldButtonA & CENTER_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_CENTER;
      else
        ButtonEvent = S_CENTER;
    }
    OldButtonA = ButtonA;
    if(((ButtonA & DOWN_BUTTON)==0)||((ButtonA & CENTER_BUTTON)==0))
      Delay =0;
    else//Have a button pressed
      ucFlag=1;
  }
}
void GPIOBIntHandler(void){

  GPIOPinIntClear(GPIO_PORTB_BASE, RIGHT_BUTTON);
  if(Delay > 1){
    ButtonB = GPIOPinRead(GPIO_PORTB_BASE, RIGHT_BUTTON);
   
    if((ButtonB& RIGHT_BUTTON)&&((OldButtonB & RIGHT_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_RIGHT;
      else
        ButtonEvent = S_RIGHT;
    }
    
    OldButtonB = ButtonB;
    if((ButtonB & RIGHT_BUTTON)==0)
      Delay =0;
    else//Have a button pressed
      ucFlag=1;
  }
}
void GPIOGIntHandler(void){

  GPIOPinIntClear(GPIO_PORTG_BASE, UP_BUTTON|LEFT_BUTTON);
  if(Delay > 1){
    ButtonG = GPIOPinRead(GPIO_PORTG_BASE, UP_BUTTON|LEFT_BUTTON);
    if((ButtonG& UP_BUTTON)&&((OldButtonG & UP_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_UP;
      else
        ButtonEvent = S_UP;
    }
    if((ButtonG& LEFT_BUTTON)&&((OldButtonG & LEFT_BUTTON)==0)){
      if(Delay > 5)
        ButtonEvent = L_LEFT;
      else
        ButtonEvent = S_LEFT;
    }
    OldButtonG = ButtonG;
    if(((ButtonG & UP_BUTTON)==0)|| ((ButtonG & LEFT_BUTTON)==0))
      Delay =0;
    else//Have a button pressed
      ucFlag=1;
  }
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
    ucFlag =0;
    i=0;
    //
    // Loop forever.
    //
  while(1){
    //
    // Call the FatFs tick timer.
    //
    disk_timerproc();
    
    //
    if(Delay > 5)
      Delay = 10;
    else
      Delay++;
    //Check a button pressed
    if(ucFlag)
    {
      //Send Event to Player Control Task
      givePlayerCtrlEvent((unsigned char *)&ButtonEvent,0);
      ucFlag = 0;
    }
    if(PlayState==PLAY_STATE)
      i++;
    if(i==12)
    {
      SecondCurrent++;
      if(SecondCurrent==60)
      {
        MinsCurrent++;
        SecondCurrent=0;
      }
      i=0;
      clock=1;
    }
    //
    // Wait for the required amount of time.
    //
    xTaskDelayUntil(&ulLastTime, ButtonEventDelay);
  }
}
/**
*
*/
char initButtonEventTask(void){
  //setup interrupt
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  //Setup for external interrupts
  GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, DOWN_BUTTON|CENTER_BUTTON);
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, RIGHT_BUTTON);
  GPIOPinTypeGPIOInput(GPIO_PORTG_BASE, UP_BUTTON|LEFT_BUTTON);
  
  GPIOIntTypeSet(GPIO_PORTA_BASE,DOWN_BUTTON|CENTER_BUTTON,GPIO_BOTH_EDGES);
  GPIOIntTypeSet(GPIO_PORTB_BASE,RIGHT_BUTTON,GPIO_BOTH_EDGES);
  GPIOIntTypeSet(GPIO_PORTG_BASE,UP_BUTTON|LEFT_BUTTON,GPIO_BOTH_EDGES);
  //
  // Enable the interrupts.
  //
  GPIOPinIntEnable(GPIO_PORTA_BASE,DOWN_BUTTON|CENTER_BUTTON);
  GPIOPinIntEnable(GPIO_PORTB_BASE,RIGHT_BUTTON);
  GPIOPinIntEnable(GPIO_PORTG_BASE,UP_BUTTON|LEFT_BUTTON);
  
  IntEnable(INT_GPIOA);
  IntEnable(INT_GPIOB);
  IntEnable(INT_GPIOG);
  //Init all Buttons not pressed
  ButtonA = DOWN_BUTTON|CENTER_BUTTON;
  OldButtonA = ButtonA;
  ButtonB = RIGHT_BUTTON;
  OldButtonB = ButtonB;
  ButtonG = UP_BUTTON|LEFT_BUTTON;
  OldButtonG = ButtonG;
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