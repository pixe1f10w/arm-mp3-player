//*****************************************************************************
//
// usbotg_task.c - Task ...
//
//
//*****************************************************************************
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_usb.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/device/usbdevice.h"
#include "usblib/host/usbhost.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "usb_dev_msc.h"
#include "usb_host_msc.h"
#include "idle_task.h"
#include "priorities.h"
#include "player_control_task.h"
#include "usbotg_task.h"

//*****************************************************************************
//
// The item size, queue size, and memory size for the display message queue.
//
//*****************************************************************************
#define DISPLAY_ITEM_SIZE       1
#define DISPLAY_QUEUE_SIZE      1
#define DISPLAY_MEM_SIZE        ((DISPLAY_ITEM_SIZE * DISPLAY_QUEUE_SIZE) +   \
                                 portQUEUE_OVERHEAD_BYTES)
//*****************************************************************************
//
// A buffer to contain the contents of the display message queue.
//
//*****************************************************************************
static unsigned long g_pulDisplayQueueMem[(DISPLAY_MEM_SIZE + 3) / 4];
//*****************************************************************************
//
// The queue that holds messages sent to the display task.
//
//*****************************************************************************
static xQueueHandle g_pDisplayQueue;

//*****************************************************************************
//
// The stack for the display task.
//
//*****************************************************************************
static unsigned long ulUSBOTGTaskStack[128];
//*****************************************************************************
//
// The amount of time to delay of USB OTG Task.
//
//*****************************************************************************
unsigned long ulUSBOTGDelay = 500;
//*****************************************************************************
//
// The current state of the USB in the system based on the detected mode.
//
//*****************************************************************************
volatile tUSBMode g_eCurrentUSBMode = USB_MODE_NONE;
//*****************************************************************************
//
// The size of the host controller's memory pool in bytes.
//
//*****************************************************************************
#define HCD_MEMORY_SIZE         128

//*****************************************************************************
//
// The memory pool to provide to the Host controller driver.
//
//*****************************************************************************
unsigned char g_pHCDPool[HCD_MEMORY_SIZE];

//*****************************************************************************
//
// This global is used to indicate to the main loop that a mode change has
// occured.
//
//*****************************************************************************
unsigned long g_ulNewState;
//*****************************************************************************
//
// Callback function for mode changes.
//
//*****************************************************************************
void
ModeCallback(unsigned long ulIndex, tUSBMode eMode)
{
    //
    // Save the new mode.
    //

    g_eCurrentUSBMode = eMode;

    switch(eMode)
    {
        case USB_MODE_HOST:
        {
            break;
        }
        case USB_MODE_DEVICE:
        {
            break;
        }
        case USB_MODE_NONE:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    g_ulNewState = 1;
}
//*****************************************************************************
//
// This task receives messages from the other tasks and updates the display as
// directed.
//
//*****************************************************************************
static void
USBOTGTask(void *pvParameters)
{
    portTickType ulLastTime;
    unsigned char usCtrlPlayer;
    //
    // Get the current tick count.
    //
    ulLastTime = xTaskGetTickCount();
    //
    // Initialize
    //
    
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Tell the OTG library code how much time has passed in milliseconds
        // since the last call.
        //
        USBOTGMain(xTaskGetTickCount());
        //
        // Handle deferred state change.
        //
        if(g_ulNewState)
        {
            g_ulNewState =0;
            if(g_eCurrentUSBMode == USB_MODE_DEVICE)
            {
              usCtrlPlayer = STOP_PLAYER;
              givePlayerCtrlEvent(&usCtrlPlayer,500);
            }else
            {
              usCtrlPlayer = START_PLAYER;
              givePlayerCtrlEvent(&usCtrlPlayer,500);
            }
        }
        if(g_eCurrentUSBMode == USB_MODE_DEVICE)
        {
            //DeviceMSCMain();
        }
        else if(g_eCurrentUSBMode == USB_MODE_HOST)
        {
            HostMSCMain();
        }
        //
        // Wait for the required amount of time.
        //
        xTaskDelayUntil(&ulLastTime, ulUSBOTGDelay);
    }
}


//*****************************************************************************
//
// Initializes the USB OTG task.
//
//*****************************************************************************
unsigned long
USBOTGTaskInit(void)
{
    //
    // Configure the required pins for USB operation.
    //
    ROM_GPIOPinTypeUSBDigital(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    ROM_GPIOPinTypeUSBDigital(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the USB stack mode and pass in a mode callback.
    //
    USBStackModeSet(0, USB_MODE_OTG, ModeCallback);

    //
    // Initialize the host stack.
    //
    HostMSCInit();

    //
    // Initialize the device stack.
    //
    DeviceMSCInit();

    //
    // Initialize the USB controller for dual mode operation with a 2ms polling
    // rate.
    //
    USBOTGModeInit(0, 2000, g_pHCDPool, HCD_MEMORY_SIZE);
    //
    // Set the new state so that the screen updates on the first
    // pass.
    //
    g_ulNewState = 1;
    //
    // Create a queue for sending messages to the display task.
    //
    if(xQueueCreate((signed portCHAR *)g_pulDisplayQueueMem, DISPLAY_MEM_SIZE,
                    DISPLAY_QUEUE_SIZE, DISPLAY_ITEM_SIZE,
                    &g_pDisplayQueue) != pdPASS)
    {
        return(1);
    }

    //
    // Create the display task.
    //
    if(xTaskCreate(USBOTGTask, (signed portCHAR *)"USB-OTG",
                   (signed portCHAR *)ulUSBOTGTaskStack,
                   sizeof(ulUSBOTGTaskStack), NULL, PRIORITY_USB_CTRL_TASK,
                   NULL) != pdPASS)
    {
        return(1);
    }
    TaskCreated();

    //
    // Success.
    //
    return(0);
}
