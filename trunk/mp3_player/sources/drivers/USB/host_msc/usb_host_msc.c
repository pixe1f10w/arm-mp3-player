//*****************************************************************************
//
// usb_host_msc.c - Example program for reading files from a USB flash drive.
//
//
//*****************************************************************************

#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"

#include "utils/ustdlib.h"
#include "usblib/usblib.h"
#include "usblib/usbmsc.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "usb_host_msc.h"
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Mass Storage Class Host Example (usb_host_msc)</h1>
//!
//! This example application demonstrates reading a file system from
//! a USB flash disk.  It makes use of FatFs, a FAT file system driver.  It
//! provides a simple widget-based console on the display and also a UART-based
//! command line for viewing and navigating the file system on the flash disk.
//!
//! For additional details about FatFs, see the following site:
//! http://elm-chan.org/fsw/ff/00index_e.html
//!
//! The application may also be operated via a serial terminal attached to
//! UART0. The RS232 communication parameters should be set to 115,200 bits
//! per second, and 8-n-1 mode.  When the program is started a message will be
//! printed to the terminal.  Type ``help'' for command help.
//
//*****************************************************************************

//*****************************************************************************
//
// Debug-related definitions and declarations.
//
// Debug output is available via UART0 if DEBUG is defined during build.
//
//*****************************************************************************
#ifdef DEBUG
//*****************************************************************************
//
// Map all debug print calls to UARTprintf in debug builds.
//
//*****************************************************************************
#define DEBUG_PRINT UARTprintf

#else

//*****************************************************************************
//
// Compile out all debug print calls in release builds.
//
//*****************************************************************************
#define DEBUG_PRINT while(0) ((int (*)(char *, ...))0)
#endif

//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary
// data from the USB disk.  There are two buffers allocated of this size.
// The buffer size must be large enough to hold the longest expected
// full path name, including the file name, and a trailing null character.
//
//*****************************************************************************
#define PATH_BUF_SIZE   80

//*****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//*****************************************************************************
#define CMD_BUF_SIZE    64

//*****************************************************************************
//
// Defines the number of times to call to check if the attached device is
// ready.
//
//*****************************************************************************
#define USBMSC_DRIVE_RETRY      4




//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;


//*****************************************************************************
//
// A structure that holds a mapping between an FRESULT numerical code,
// and a string representation.  FRESULT codes are returned from the FatFs
// FAT file system driver.
//
//*****************************************************************************
typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

//*****************************************************************************
//
// A macro to make it easy to add result codes to the table.
//
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

//*****************************************************************************
//
// A table that holds a mapping between the numerical FRESULT code and
// it's name as a string.  This is used for looking up error codes for
// printing to the console.
//
//*****************************************************************************
tFresultString g_sFresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_RW_ERROR),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_MKFS_ABORTED)
};

//*****************************************************************************
//
// Error reasons returned by ChangeDirectory().
//
//*****************************************************************************
#define NAME_TOO_LONG_ERROR 1
#define OPENDIR_ERROR       2

//*****************************************************************************
//
// A macro that holds the number of result codes.
//
//*****************************************************************************
#define NUM_FRESULT_CODES (sizeof(g_sFresultStrings) / sizeof(tFresultString))

//*****************************************************************************
//
// The number of SysTick ticks per second.
//
//*****************************************************************************
#define TICKS_PER_SECOND 100
#define MS_PER_SYSTICK (1000 / TICKS_PER_SECOND)

//*****************************************************************************
//
// Our running system tick counter and a global used to determine the time
// elapsed since last call to GetTickms().
//
//*****************************************************************************
unsigned long g_ulSysTickCount;
unsigned long g_ulLastTick;

//*****************************************************************************
//
// Holds global flags for the system.
//
//*****************************************************************************
static unsigned long ulFlags = 0;

//*****************************************************************************
//
// Flag indicating that some USB device is connected.
//
//*****************************************************************************
#define FLAGS_DEVICE_PRESENT    0x00000001

//*****************************************************************************
//
// Hold the current state for the application.
//
//*****************************************************************************
volatile enum
{
    //
    // No device is present.
    //
    STATE_NO_DEVICE,

    //
    // Mass storage device is being enumerated.
    //
    STATE_DEVICE_ENUM,

    //
    // Mass storage device is ready.
    //
    STATE_DEVICE_READY,

    //
    // An unsupported device has been attached.
    //
    STATE_UNKNOWN_DEVICE,

    //
    // A mass storage device was connected but failed to ever report ready.
    //
    STATE_TIMEOUT_DEVICE,

    //
    // A power fault has occurred.
    //
    STATE_POWER_FAULT
}
g_eState;

//*****************************************************************************
//
// The instance data for the MSC driver.
//
//*****************************************************************************
unsigned long g_ulMSCInstance = 0;

//*****************************************************************************
//
// Declare the USB Events driver interface.
//
//*****************************************************************************
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);

//*****************************************************************************
//
// The global that holds all of the host drivers in use in the application.
// In this case, only the MSC class is loaded.
//
//*****************************************************************************
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_USBHostMSCClassDriver,
    &g_sUSBEventDriver
};

//*****************************************************************************
//
// This global holds the number of class drivers in the g_ppHostClassDrivers
// list.
//
//*****************************************************************************
static const unsigned long g_ulNumHostClassDrivers =
    sizeof(g_ppHostClassDrivers) / sizeof(tUSBHostClassDriver *);

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//
//*****************************************************************************
/**#if defined(ewarm)
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[6];
#elif defined(ccs)
#pragma DATA_ALIGN(g_sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[6];
#else
tDMAControlTable sDMAControlTable[6] __attribute__ ((aligned(1024)));
#endif
*/
extern tDMAControlTable sDMAControlTable[];
//****************************************************************************
//
// Initializes the file system module.
//
// \param None.
//
// This function initializes the third party FAT implementation.
//
// \return Returns \e true on success or \e false on failure.
//
//*****************************************************************************
tBoolean
FileInit(void)
{
    //
    // Mount the file system, using logical disk 1.
    //
    if(f_mount(1, &g_sFatFs) != FR_OK)
    {
        return(false);
    }
    return(true);
}
//*****************************************************************************
//
// This is the callback from the MSC driver.
//
// \param ulInstance is the driver instance which is needed when communicating
// with the driver.
// \param ulEvent is one of the events defined by the driver.
// \param pvData is a pointer to data passed into the initial call to register
// the callback.
//
// This function handles callback events from the MSC driver.  The only events
// currently handled are the MSC_EVENT_OPEN and MSC_EVENT_CLOSE.  This allows
// the main routine to know when an MSC device has been detected and
// enumerated and when an MSC device has been removed from the system.
//
// \return None
//
//*****************************************************************************
void
MSCCallback(unsigned long ulInstance, unsigned long ulEvent, void *pvData)
{
    //
    // Determine the event.
    //
    switch(ulEvent)
    {
        //
        // Called when the device driver has successfully enumerated an MSC
        // device.
        //
        case MSC_EVENT_OPEN:
        {
            //
            // Proceed to the enumeration state.
            //
            g_eState = STATE_DEVICE_ENUM;

            break;
        }

        //
        // Called when the device driver has been unloaded due to error or
        // the device is no longer present.
        //
        case MSC_EVENT_CLOSE:
        {
            //
            // Go back to the "no device" state and wait for a new connection.
            //
            g_eState = STATE_NO_DEVICE;

            //
            // Re-initialize the file system.
            //
            FileInit();

            break;
        }

        default:
        {
            break;
        }
    }
}

//*****************************************************************************
//
// This is the generic callback from host stack.
//
// \param pvData is actually a pointer to a tEventInfo structure.
//
// This function will be called to inform the application when a USB event has
// occurred that is outside those related to the mass storage device.  At this
// point this is used to detect unsupported devices being inserted and removed.
// It is also used to inform the application when a power fault has occurred.
// This function is required when the g_USBGenericEventDriver is included in
// the host controller driver array that is passed in to the
// USBHCDRegisterDrivers() function.
//
// \return None.
//
//*****************************************************************************
void
USBHCDEvents(void *pvData)
{
    tEventInfo *pEventInfo;

    //
    // Cast this pointer to its actual type.
    //
    pEventInfo = (tEventInfo *)pvData;

    switch(pEventInfo->ulEvent)
    {
        //
        // An unknown device has been connected.
        //
        case USB_EVENT_UNKNOWN_CONNECTED:
        {
            //
            // An unknown device was detected.
            //
            g_eState = STATE_UNKNOWN_DEVICE;

            break;
        }

        //
        // The unknown device has been been unplugged.
        //
        case USB_EVENT_DISCONNECTED:
        {
            //
            // Unknown device has been removed.
            //
            g_eState = STATE_NO_DEVICE;

            break;
        }

        //
        // A bus power fault was detected.
        //
        case USB_EVENT_POWER_FAULT:
        {
            //
            // No power means no device is present.
            //
            g_eState = STATE_POWER_FAULT;

            break;
        }

        default:
        {
            break;
        }
    }
}
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
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
//
//*****************************************************************************
    int nStatus;
    tUSBMode eLastMode;
    char *pcString;
    unsigned long ulDriveTimeout;
unsigned long
HostMSCInit(void)
{

    //
    // Initially wait for device connection.
    //
    g_eState = STATE_NO_DEVICE;
    eLastMode = USB_MODE_OTG;

    //
    // Enable the uDMA controller and set up the control table base.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(sDMAControlTable);

   //
    // Configure the required pins for USB operation.
    //
    ROM_GPIOPinTypeUSBDigital(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    ROM_GPIOPinTypeUSBDigital(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Register the host class drivers.
    //
    USBHCDRegisterDrivers(0, g_ppHostClassDrivers, g_ulNumHostClassDrivers);

    //
    // Open an instance of the mass storage class driver.
    //
    g_ulMSCInstance = USBHMSCDriveOpen(0, MSCCallback);

    //
    // Initialize the drive timeout.
    //
    ulDriveTimeout = USBMSC_DRIVE_RETRY;

    //
    // Initialize the power configuration. This sets the power enable signal
    // to be active high and does not enable the power fault.
    //
    USBHCDPowerConfigInit(0, USBHCD_VBUS_AUTO_HIGH | USBHCD_VBUS_FILTER);

    //
    // Initialize the file system.
    //
    FileInit();
    return 0;
}
/*
*  Enter an (almost) infinite loop for reading and processing.
*/
void HostMSCMain(void)
{
    
    //
    // No command is waiting yet - go ahead and do housekeeping.
    //
    
    //
    // Tell the OTG library code how much time has passed in
    // milliseconds since the last call.
    //
    //USBOTGMain(GetTickms());

    switch(g_eState)
    {
        case STATE_DEVICE_ENUM:
            {
                //
                // Take it easy on the Mass storage device if it is slow to
                // start up after connecting.
                //     
                if(USBHMSCDriveReady(g_ulMSCInstance) != 0)
                {
                    //
                    // Wait about 500ms before attempting to check if the
                    // device is ready again.
                    //
                    SysCtlDelay(SysCtlClockGet()/(3*2));
                    //
                    // Decrement the retry count.
                    //
                    ulDriveTimeout--;

                    //
                    // If the timeout is hit then go to the
                    // STATE_TIMEOUT_DEVICE state.
                    //
                    if(ulDriveTimeout == 0)
                    {
                        g_eState = STATE_TIMEOUT_DEVICE;
                    }
                    break;
                }
                //
                // Set the Device Present flag.
                //
                ulFlags = FLAGS_DEVICE_PRESENT;
                break;
            }
            //
            // If there is no device then just wait for one.
            //
        case STATE_NO_DEVICE:
            {
                if(ulFlags == FLAGS_DEVICE_PRESENT)
                {
                    //
                    // Empty the list box on the display.
                    //
                    //
                    // Clear the Device Present flag.
                    //
                    ulFlags &= ~FLAGS_DEVICE_PRESENT;
                 }
                 break;
             }
             //
             // An unknown device was connected.
             //
        case STATE_UNKNOWN_DEVICE:
             {
                 //
                 // If this is a new device then change the status.
                 //
                 if((ulFlags & FLAGS_DEVICE_PRESENT) == 0)
                 {
                     //
                     // Clear the screen and indicate that an unknown device
                     // is present.
                     //
                 }
                 //
                 // Set the Device Present flag.
                 //
                 ulFlags = FLAGS_DEVICE_PRESENT;
                 break;
             }
             //
             // The connected mass storage device is not reporting ready.
             //
        case STATE_TIMEOUT_DEVICE:
             {
                 //
                 // If this is the first time in this state then print a
                 // message.
                 //
                 if((ulFlags & FLAGS_DEVICE_PRESENT) == 0)
                 {
                    //
                        // Clear the screen and indicate that an unknown device
                        // is present.
                        //
                 }

                    //
                    // Set the Device Present flag.
                    //
                 ulFlags = FLAGS_DEVICE_PRESENT;

                 break;
             }

                //
                // Something has caused a power fault.
                //
        case STATE_POWER_FAULT:
             {
                break;
             }

        default:
            break;
    }
}
