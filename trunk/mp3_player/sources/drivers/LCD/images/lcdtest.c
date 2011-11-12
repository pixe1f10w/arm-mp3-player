#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "utils_custom_v1_02.h"
#include "LCD_240x320.h"
#include "font_engine_v1_00.h"
#include "font_thai_plugin_v1_02.h"
#include "images/back.h"
//#include "images/smile.h"

//############################################
// bmp to header file convertor version 3.0
// Author : www.ThaiEasyElec.com
//############################################
// source bmp file : C:\Documents and Settings\MaIII\Desktop\smil46850f9ed81f3 copy.bmp
// source pixel dept : 32
// target name : C:\Documents and Settings\MaIII\Desktop\smile.h
// target pixel dept : 16
// target width : 19
// target height : 19
// ############################################
const unsigned short smile[] = 
{
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xF7FF,
	0x9EDE,0x869D,0x9EDE,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xDFBF,0x6EFF,0x7F7F,0x5F9F,0x679F,0x575F,0x773F,0x6EBE,
	0xDF9F,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0x76DE,0x97BF,0x679F,0x679F,0x679F,0x5F9F,
	0x679F,0x4F3F,0x4F3F,0x875F,0xAF1E,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x773F,0x679F,0x679F,0x679F,
	0x679F,0x679F,0x679F,0x5F9F,0x5F9F,0x4F3F,0x3EBF,0x3EBF,
	0x5E3D,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x6EFF,0x679F,
	0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x679F,0x679F,0x679F,0x5F9F,
	0x4F3F,0x4F3F,0x3EBF,0x3EBF,0xA6DE,0xFFFF,0xFFFF,0xFFFF,
	0xDFBF,0x979F,0x679F,0x5F9F,0x032F,0x032F,0x5F9F,0x679F,
	0x679F,0x679F,0x5F9F,0x032F,0x032F,0x3EBF,0x46FF,0x6EBE,
	0xD75E,0xFFFF,0xFFFF,0x6EFF,0x679F,0x679F,0x032F,0x5F9F,
	0x5F9F,0x032F,0x679F,0x679F,0x679F,0x032F,0x5F9F,0x4F3F,
	0x032F,0x3EBF,0x3EBF,0x459B,0xFFFF,0xFFFF,0x875F,0x679F,
	0x679F,0x032F,0x5F9F,0x5F9F,0x032F,0x679F,0x679F,0x5F9F,
	0x032F,0x5F9F,0x4F3F,0x032F,0x3EBF,0x3EBF,0x5E3D,0xFFFF,
	0x9EFE,0x577F,0x679F,0x0476,0x5F9F,0x5F9F,0x5F9F,0x5F9F,
	0x679F,0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x0476,
	0x365F,0x361E,0x8DD9,0x869D,0x577F,0x0476,0x679F,0x5F9F,
	0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x5F9F,0x577F,0x5F9F,0x4F3F,
	0x4F3F,0x3EBF,0x3EBF,0x0476,0x361E,0x759A,0x9EDE,0x575F,
	0x577F,0x5F9F,0x0476,0x0476,0x2D7B,0x2D7B,0x361E,0x361E,
	0x361E,0x2D7B,0x2D7B,0x0476,0x0476,0x365F,0x365F,0x2DDD,
	0x8DD9,0xD77F,0x3EBF,0x577F,0x0476,0xFFFF,0xFFFF,0xEF7D,
	0xEF7D,0xE71C,0xE71C,0xE71C,0xEF7D,0xEF7D,0xFFFF,0xEF7D,
	0x0476,0x361E,0x2D7B,0xD71D,0xFFFF,0x76BF,0x0476,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xEF7D,0xEF7D,0xEF7D,0xFFFF,
	0xFFFF,0xFFFF,0xEF7D,0xEF7D,0x0476,0x4539,0xFFFF,0xFFFF,
	0xD77E,0x0476,0xFFFF,0xFFFF,0xFFFF,0xC618,0xFFFF,0xFFFF,
	0xC618,0xFFFF,0xFFFF,0xC618,0xFFFF,0xEF7D,0xEF7D,0x0476,
	0xA63A,0xFFFF,0xFFFF,0xFFFF,0x0476,0xFFFF,0xFFFF,0xC618,
	0xC618,0xC618,0xC618,0xC618,0xC618,0xC618,0xC618,0xC618,
	0xEF7D,0xEF7D,0x4CD8,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0x0476,0xFFFF,0xFFFF,0xC618,0xFFFF,0xFFFF,0xC618,0xFFFF,
	0xFFFF,0xC618,0xEF7D,0xEF7D,0x4539,0xF7DF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0476,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xEF7D,0xEF7D,0x4CD8,0xF7DF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0x0476,0x0476,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x4539,
	0xA63A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0476,0x0476,0x0476,
	0x0476,0x0476,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF};
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Blinky (blinky)</h1>
//!
//! A very simple example that blinks the on-board LED.
//
//*****************************************************************************

//*****************************************************************************
//
// Blink the on-board LED.
//
//*****************************************************************************
int
main(void)
{
    volatile unsigned long ulLoop;

   //
    //! Set the clocking to run at 80 MHz from the PLL.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
    //
    //! Enable all GPIO banks.
    //
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,0xff);
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE,0xff);

    TS_LCD_CONTROL_PORT_OUT();
    TS_LCD_BL_SET();
    TS_LCD_BL_CLR();
    TSLCDInit();
    delay_1ms(1000);
    while(1)
    {
    //+++++++++++++++++++++++++++++
    //LCD test
    //++++++++++++++++++++++++++++++
	TSLCDSetFontColor(TS_COL_WHITE);
//	TSLCDSetBackColor(COL_BACK);
	TSLCDSetBackColor(TS_COL_BLUE);
//	TSLCDCfgFont(font_angsana_eng32x48,32,6,2);
//  TSLCDCfgExtFont(font_angsana32x64,32,8,15,16);
  	//TSLCDCfgExtFont(font_angsana14x32,14,4,5,9);
	//TSLCDCfgFont(font_vrinda11x16,11,2,2);
	//TSLCDSetVaryWidth(1);
	//TSLCDSetBold(0);

	TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_BLUE,TS_MODE_NORMAL);
	//TSLCDSetBold(1);
//	TSLCDCfgFont(font_vrinda11x16,11,2,2);
        TSLCDCfgFont(font_angsana_eng32x48,32,6,2);
	TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("LCD TEST")) >> 2,0);
	TSLCDPrintStr( 2,2,"LCD TEST",TS_MODE_FULL);
	delay_1ms(3000);
	TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_BLUE,TS_MODE_NORMAL);
	TSLCDCfgFont(font_vrinda11x16,11,2,2);
	TSLCDSetBold(1);
	TSLCDCfgExtFont(font_angsana32x64,32,64,45,16);
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_BLUE,TS_MODE_NORMAL);
        TSLCDCfgFont(font_angsana_eng32x48,32,6,2);
	TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("MUI LE VAN")) >> 1,0);
	TSLCDPrintStr( 3,2,"MUI LE VAN",TS_MODE_FULL);
	TSLCDCfgExtFont(font_angsana14x32,14,4,5,9);
	delay_1ms(1000);
	//TSLCDSetBold(1);
	//TSLCDCfgFont(font_vrinda18x16,18,2,2);
	//TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("Available NOW!!!")) >> 1,0);
	//TSLCDPrintStr( 1,0,"Available NOW!!!",TS_MODE_FULL);
	//delay_1ms(1000);
	//TSLCDSetBold(0);
	//TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("at www.ThaiEasyElec.com")) >> 1,0);
	//TSLCDPrintStr( 12,0,"at www.ThaiEasyElec.com",TS_MODE_FULL);
	//delay_1ms(2000);

	//TSLCDCfgFont(font_vrinda11x16,11,2,2);
	//TSLCDCfgExtFont(font_angsana14x32,14,4,5,11);
	//TSLCDSetVaryWidth(1);
	//TSLCDSetBold(0);
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
       // TSLCDShowPic(5,23,5,23,smile,TS_MODE_FULL);
        //delay_1ms(2000);
        //TSLCDShowPic(1,220,1,201,smile,TS_MODE_FULL);
        //delay_1ms(2000);
        //TSLCDShowPic2(1,220,1,201,smile,TS_MODE_FULL);
        //delay_1ms(2000);
       // TSLCDOutIns8(0x36);
       // TSLCDOutIns8(0x20);
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
        //TSLCDShowPic2(1,200,1,300,muile,TS_MODE_FULL);
       // delay_1ms(2000);
    //while (1);

        
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
        //TSLCDShowPic2(0,319,0,239,background2,TS_MODE_FULL);
        //delay_1ms(2000);
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
        //TSLCDShowPicBack(0,319,0,239,background2,320,240,TS_MODE_FULL);
        //delay_1ms(2000);
        //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
        //TSLCDShowPic2(1,240,1,201,smile,TS_MODE_FULL);
        //delay_1ms(2000);
         TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
         TSLCDShowPicBack(0,239,0,299,back,240,320,TS_MODE_FULL);
         delay_1ms(2000);
         TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
         TSLCDShowPic(0,239,0,299,back,TS_MODE_FULL);
         delay_1ms(2000);
         // TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
         //TSLCDShowPic(1,220,1,201,flower,TS_MODE_FULL);
}
}