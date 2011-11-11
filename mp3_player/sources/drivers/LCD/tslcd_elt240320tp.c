//#include "app_config.h"
//*****************************************************************************
#include <string.h>
#include <stdio.h>
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "tslcd_elt240320tp.h"
//#include "font_thai_plugin_v1_02.h"
#include "font_engine_v1_00.h"
//#include "utils_custom_v1_02.h"
//  _____________________
// | _______________	 |
// ||<-- origin in software
// ||	---------	|[]	 |
// ||		/		|[]	 |
// || 	-------->	|[]	 |
// ||_______________|[]	 |
// |<-- LCD's origin (0x0000)
//
//define TS_ORN_PORTRAIT when your LCD is installed in vertical
//  ________________
// |<-- LCD's origin (0x0000) = origin in software
// ||--------------||
// ||			   ||
// ||	------     ||
// ||		/	   ||
// ||	  /		   ||
// ||	/		   ||
// ||	------>	   ||
// ||______________||
// | [] [] [] [] []	|
// |________________|
//


void TSLCDCharDisp(char charactor,ts_pos_t sx,ts_pos_t sy,ts_mode_t mode); //low level function to print a character on LCD
void display_buf(ts_pos_t sx,ts_pos_t sy,ts_mode_t mode);

typedef enum
{
	ENG,
	EXT
} font_mode_t;

font_mode_t  font_mode;
unsigned char font_ext_adj = 0;

extern unsigned char font_width;
extern unsigned char font_height;
extern unsigned char char_gap;

extern unsigned char first_non_zero;
extern unsigned char last_non_zero;

//#ifdef FONT_EXT_PLUGIN
unsigned char font_ext_width;
unsigned char font_ext_height;
//#endif

unsigned char print_limit = 0;
unsigned int print_limit_len;

unsigned short font_color;
unsigned short back_color;
ts_pos_t offsetx,offsety;
ts_pos_t ts_margin_xl = 0;
ts_pos_t ts_margin_xr = TS_SIZE_X - 1;
ts_pos_t ts_margin_yu = 0;
ts_pos_t ts_margin_yl = TS_SIZE_Y - 1;

#define TSLCDGetMarginXl()			ts_margin_xl
#define TSLCDGetMarginXr()			ts_margin_xr
#define TSLCDGetMarginYu()			ts_margin_yu
#define TSLCDGetMarginYl()			ts_margin_yl

void TS_LCD_WRITE_DATA8BIT(unsigned char ucdata)
{
	GPIOPinWrite(TS_LCD_DATA_PORT,TS_LCD_DATA,ucdata) ;
}
unsigned char TS_LCD_READ_DATA8BIT(void)
{
	return ((unsigned char )(GPIOPinRead(TS_LCD_DATA_PORT,TS_LCD_DATA)));
}
/***********************/
/* milisecond delay funtion*/

/***********************/
void delay_1ms(unsigned long int microsecond)
{    
        SysCtlDelay(((unsigned long )microsecond*(SysCtlClockGet() / (3*1000))));
}
void TSLCDSetFontColor(unsigned short color) //set text's color
{
	font_color = color;
}

void TSLCDSetBackColor(unsigned short color) //set back color for TS_MODE_FULL
{
	back_color = color;
}

/*
#ifndef LCD_GPIO_MODE
void TSLCDOutDat(unsigned short dat) //write data to LCD
{
	TS_XDAT = dat >> 8;
	TS_XDAT = dat;
}

void TSLCDOutDat2(unsigned char dath,unsigned char datl) //write data to LCD
{
	TS_XDAT = dath;
	TS_XDAT = datl;
}

void TSLCDOutIns(unsigned short ins) //write instruction to LCD
{
	TS_XINS = ins >> 8;
	TS_XINS = ins;
}

unsigned short TSLCDInDat(void) //read data from LCD
{
	unsigned short dat = TS_XDAT << 8;
	dat |= TS_XDAT;
	return (dat);
}

unsigned short TSLCDInIns(void) //read data from LCD
{
	unsigned short ins = TS_XINS << 8;
	ins |= TS_XINS;
	return (ins);
}
#else
*/
void TSLCDOutDat(unsigned short dat) //write data to LCD
{
        TS_LCD_DATA_PORT_OUT() ;
	//TS_XDAT = dat >> 8;
	//TS_XDAT = dat;
	//Setb(LCD_RS_PRTS,LCD_RS_PIN);
	TS_LCD_RS_SET();
	//Setb(LCD_RD_PRTS,LCD_RD_PIN);
	TS_LCD_RD_SET();
	//Clrb(LCD_WR_PRTC,LCD_WR_PIN);
	TS_LCD_WR_CLR();
	//LCD_DB_DPRT = 0xFF;
	TS_LCD_WRITE_DATA8BIT(0xFF);
	//LCD_DB_PORT = dat >> 8;
	TS_LCD_WRITE_DATA8BIT((unsigned char)(dat>>8));

	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();

	//LCD_DB_PORT = dat;
        TS_LCD_WRITE_DATA8BIT((unsigned char)(dat & 0x00FF));
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();
	
	//Setb(LCD_WR_PRTS,LCD_WR_PIN);
	TS_LCD_WR_SET();
	//LCD_DB_DPRT = 0;
	TS_LCD_WRITE_DATA8BIT(0x00);
}

void TSLCDOutDat2(unsigned char high,unsigned char low) //write data to LCD high and low nibble
{
	TS_LCD_DATA_PORT_OUT();
	//TS_XDAT = dath;
	//TS_XDAT = datl;
	//Setb(LCD_RS_PRTS,LCD_RS_PIN);
        TS_LCD_RS_SET();
	//Setb(LCD_RD_PRTS,LCD_RD_PIN);
	TS_LCD_RD_SET();
	//Clrb(LCD_WR_PRTC,LCD_WR_PIN);
	TS_LCD_WR_CLR();
	
	//LCD_DB_DPRT = 0xFF;
	TS_LCD_WRITE_DATA8BIT(0xFF);
	//LCD_DB_PORT = dath;
	TS_LCD_WRITE_DATA8BIT(high);

	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();

	//LCD_DB_PORT = datl;
        TS_LCD_WRITE_DATA8BIT(low);
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();

	//Setb(LCD_WR_PRTS,LCD_WR_PIN);
        TS_LCD_WR_SET();
	//LCD_DB_DPRT = 0;
	TS_LCD_WRITE_DATA8BIT(0x00);
}

void TSLCDOutIns(unsigned short ins) //write instruction to LCD
{
	TS_LCD_DATA_PORT_OUT() ;
	//TS_XINS = ins >> 8;
	//TS_XINS = ins;
	//Clrb(LCD_RS_PRTC,LCD_RS_PIN);
        TS_LCD_RS_CLR();
	//Setb(LCD_RD_PRTS,LCD_RD_PIN);
	TS_LCD_RD_SET();
	//Clrb(LCD_WR_PRTC,LCD_WR_PIN);
	TS_LCD_WR_CLR();

	//LCD_DB_DPRT = 0xFF;
	TS_LCD_WRITE_DATA8BIT(0xFF);
	//LCD_DB_PORT = ins >> 8;
	TS_LCD_WRITE_DATA8BIT((unsigned char) (ins >> 8));

	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();
	//LCD_DB_PORT = ins;
	TS_LCD_WRITE_DATA8BIT((unsigned char) (ins & 0x00FF));
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
//	Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();

	//Setb(LCD_WR_PRTS,LCD_WR_PIN);
	TS_LCD_WR_SET();

	//LCD_DB_DPRT = 0;
	TS_LCD_WRITE_DATA8BIT(0x00);
}

unsigned short TSLCDInDat(void) //read data from LCD
{
//	unsigned short dat = TS_XDAT << 8;
//	dat |= TS_XDAT;
	unsigned short dat = 0;
     
	//LCD_DB_DPRT = 0;
	TS_LCD_DATA_PORT_IN();
	TS_LCD_WRITE_DATA8BIT(0x00);

	//Setb(LCD_RS_PRTS,LCD_RS_PIN);
	TS_LCD_RS_SET();

	//Setb(LCD_WR_PRTS,LCD_WR_PIN);
	TS_LCD_WR_SET();
	//Clrb(LCD_RD_PRTC,LCD_RD_PIN);
	TS_LCD_RD_CLR();

	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_WR_CLR();
	//dat = LCD_DB_PINP;
	dat = (unsigned short)TS_LCD_READ_DATA8BIT();
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();
	dat <<= 8;

	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//dat |= LCD_DB_PINP;
	dat |= (unsigned short)TS_LCD_READ_DATA8BIT();
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
        TS_LCD_CS_SET();
	//Setb(LCD_RD_PRTS,LCD_RD_PIN);
	TS_LCD_RD_CLR();
	return (dat);
}

unsigned short TSLCDInIns(void) //read data from LCD
{
//	unsigned short ins = TS_XINS << 8;
//	ins |= TS_XINS;
	unsigned short ins = 0;
        TS_LCD_DATA_PORT_IN();
	//LCD_DB_DPRT = 0;
        TS_LCD_WRITE_DATA8BIT(0x00);
	//Clrb(LCD_RS_PRTC,LCD_RS_PIN);
        TS_LCD_RS_CLR();
	//Setb(LCD_WR_PRTS,LCD_WR_PIN);
	TS_LCD_WR_SET();
	//Clrb(LCD_RD_PRTC,LCD_RD_PIN);
        TS_LCD_RD_CLR(); 
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);'
	TS_LCD_CS_CLR();
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//ins = LCD_DB_PINP;
	ins = (unsigned short)TS_LCD_READ_DATA8BIT();
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
	TS_LCD_CS_SET();

	ins <<= 8;
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//Clrb(LCD_CS_PRTC,LCD_CS_PIN);
	TS_LCD_CS_CLR();
	//ins |= LCD_DB_PINP;
	ins |= (unsigned short)TS_LCD_READ_DATA8BIT();
	//Setb(LCD_CS_PRTS,LCD_CS_PIN);
        TS_LCD_CS_SET();
	//Setb(LCD_RD_PRTS,LCD_RD_PIN);
	TS_LCD_RD_SET();
	return (ins);
}
//#endif

void TSLCDRst(void) //pulse reset signal to LCD
{
	//Orb(LCD_RST_DPRT,LCD_RST_PIN);
	//TS_LCD_WRITE_DATA8BIT();
	TS_LCD_RS_SET();
	//Clrb(LCD_RST_PRTC,LCD_RST_PIN);
	TS_LCD_RS_CLR();
	delay_1ms(50);
	//Setb(LCD_RST_PRTS,LCD_RST_PIN);
	TS_LCD_RS_SET();
}

void TSLCDInit(void) //initial LCD
{
	unsigned short driver_code;
	TS_LCD_DATA_PORT_ENABLE();
	TS_LCD_CONTROL_PORT_ENABLE();
	delay_1ms(100);
	TSLCDOutIns(TS_INS_START_OSC);
	driver_code = TSLCDInDat();
	if (driver_code == 0x9320) //ILI9320
	{
		TSLCDOutIns(0x00E5);
		TSLCDOutDat(0x8000); 					//set the internal vcore voltage
		TSLCDOutIns(TS_INS_START_OSC);
		TSLCDOutDat(0x0001); 					//start oscillator
		delay_1ms(50);

		TSLCDOutIns(TS_INS_DRIV_OUT_CTRL);
		TSLCDOutDat(0x0100); 					//set SS, SM
		TSLCDOutIns(TS_INS_DRIV_WAV_CTRL);
		TSLCDOutDat(0x0700); 					//set 1 line inversion

		TSLCDOutIns(TS_INS_ENTRY_MOD);
		TSLCDOutDat(TS_VAL_ENTRY_MOD);			//set GRAM write direction, BGR=0

		TSLCDOutIns(TS_INS_RESIZE_CTRL);
		TSLCDOutDat(0x0000); 					//no resizing

		TSLCDOutIns(TS_INS_DISP_CTRL2);
		TSLCDOutDat(0x0202); 					//front & back porch periods = 2
		TSLCDOutIns(TS_INS_DISP_CTRL3);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_DISP_CTRL4);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_RGB_DISP_IF_CTRL1);
		TSLCDOutDat(0x0000); 					//select system interface
		TSLCDOutIns(TS_INS_FRM_MARKER_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_RGB_DISP_IF_CTRL2);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_POW_CTRL1);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_POW_CTRL2);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_POW_CTRL3);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_POW_CTRL4);
		TSLCDOutDat(0x0000);
		delay_1ms(200);

		TSLCDOutIns(TS_INS_POW_CTRL1);
		TSLCDOutDat(0x17B0);
		TSLCDOutIns(TS_INS_POW_CTRL2);
		TSLCDOutDat(0x0137);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_POW_CTRL3);
		TSLCDOutDat(0x013C);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_POW_CTRL4);
		TSLCDOutDat(0x1400);
		TSLCDOutIns(TS_INS_POW_CTRL7);
		TSLCDOutDat(0x0007);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_GRAM_HOR_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_GRAM_VER_AD);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_GAMMA_CTRL1);
		TSLCDOutDat(0x0007);
		TSLCDOutIns(TS_INS_GAMMA_CTRL2);
		TSLCDOutDat(0x0504);
		TSLCDOutIns(TS_INS_GAMMA_CTRL3);
		TSLCDOutDat(0x0703);
		TSLCDOutIns(TS_INS_GAMMA_CTRL4);
		TSLCDOutDat(0x0002);
		TSLCDOutIns(TS_INS_GAMMA_CTRL5);
		TSLCDOutDat(0x0707);
		TSLCDOutIns(TS_INS_GAMMA_CTRL6);
		TSLCDOutDat(0x0406);
		TSLCDOutIns(TS_INS_GAMMA_CTRL7);
		TSLCDOutDat(0x0006);
		TSLCDOutIns(TS_INS_GAMMA_CTRL8);
		TSLCDOutDat(0x0404);
		TSLCDOutIns(TS_INS_GAMMA_CTRL9);
		TSLCDOutDat(0x0700);
		TSLCDOutIns(TS_INS_GAMMA_CTRL10);
		TSLCDOutDat(0x0A08);

		TSLCDOutIns(TS_INS_HOR_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_HOR_END_AD);
		TSLCDOutDat(0x00EF);
		TSLCDOutIns(TS_INS_VER_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_VER_END_AD);
		TSLCDOutDat(0x013F);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL1);
		TSLCDOutDat(0x2700);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL2);
		TSLCDOutDat(0x0001);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL3);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_PART_IMG1_DISP_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG1_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG1_END_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_DISP_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_END_AD);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_PANEL_IF_CTRL1);
		TSLCDOutDat(0x0010);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL2);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL3);
		TSLCDOutDat(0x0003);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL4);
		TSLCDOutDat(0x0110);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL5);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL6);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_DISP_CTRL1);
		TSLCDOutDat(0x0173);
	}
	if ((driver_code == 0x9325) || (driver_code == 0x9328)) //ILI9325 orILI9328

	{
		TSLCDOutIns(0x00E3);
		TSLCDOutDat(0x3008); 					//set the internal timing
		TSLCDOutIns(0x00E7);
		TSLCDOutDat(0x0012); 					//set the internal timing
		TSLCDOutIns(0x00EF);
		TSLCDOutDat(0x1231); 					//set the internal timing
		TSLCDOutIns(TS_INS_START_OSC);
		TSLCDOutDat(0x0001); 					//start oscillator
		delay_1ms(50);

		TSLCDOutIns(TS_INS_DRIV_OUT_CTRL);
		TSLCDOutDat(0x0100); 					//set SS, SM
		TSLCDOutIns(TS_INS_DRIV_WAV_CTRL);
		TSLCDOutDat(0x0700); 					//set 1 line inversion

		TSLCDOutIns(TS_INS_ENTRY_MOD);
		TSLCDOutDat(TS_VAL_ENTRY_MOD);			//set GRAM write direction, BGR=0

		TSLCDOutIns(TS_INS_RESIZE_CTRL);
		TSLCDOutDat(0x0000); 					//no resizing

		TSLCDOutIns(TS_INS_DISP_CTRL2);
		TSLCDOutDat(0x0202); 					//front & back porch periods = 2
		TSLCDOutIns(TS_INS_DISP_CTRL3);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_DISP_CTRL4);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_RGB_DISP_IF_CTRL1);
		TSLCDOutDat(0x0000); 					//select system interface
		TSLCDOutIns(TS_INS_FRM_MARKER_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_RGB_DISP_IF_CTRL2);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_POW_CTRL1);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_POW_CTRL2);
		TSLCDOutDat(0x0007);
		TSLCDOutIns(TS_INS_POW_CTRL3);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_POW_CTRL4);
		TSLCDOutDat(0x0000);
		delay_1ms(200);

		TSLCDOutIns(TS_INS_POW_CTRL1);
		TSLCDOutDat(0x1690);
		TSLCDOutIns(TS_INS_POW_CTRL2);
		TSLCDOutDat(0x0227); //TSLCDOutDat(0x0137);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_POW_CTRL3);
		TSLCDOutDat(0x001A); //TSLCDOutDat(0x013C);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_POW_CTRL4);
		TSLCDOutDat(0x1800); //TSLCDOutDat(0x1400);
		TSLCDOutIns(TS_INS_POW_CTRL7);
		TSLCDOutDat(0x002A); //TSLCDOutDat(0x0007);
		delay_1ms(50);

		TSLCDOutIns(TS_INS_GRAM_HOR_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_GRAM_VER_AD);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_GAMMA_CTRL1);
		TSLCDOutDat(0x0007);
		TSLCDOutIns(TS_INS_GAMMA_CTRL2);
		TSLCDOutDat(0x0605);
		TSLCDOutIns(TS_INS_GAMMA_CTRL3);
		TSLCDOutDat(0x0106);
		TSLCDOutIns(TS_INS_GAMMA_CTRL4);
		TSLCDOutDat(0x0206);
		TSLCDOutIns(TS_INS_GAMMA_CTRL5);
		TSLCDOutDat(0x0808);
		TSLCDOutIns(TS_INS_GAMMA_CTRL6);
		TSLCDOutDat(0x0007);
		TSLCDOutIns(TS_INS_GAMMA_CTRL7);
		TSLCDOutDat(0x0201);
		TSLCDOutIns(TS_INS_GAMMA_CTRL8);
		TSLCDOutDat(0x0007);
		TSLCDOutIns(TS_INS_GAMMA_CTRL9);
		TSLCDOutDat(0x0602);
		TSLCDOutIns(TS_INS_GAMMA_CTRL10);
		TSLCDOutDat(0x0808);

		TSLCDOutIns(TS_INS_HOR_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_HOR_END_AD);
		TSLCDOutDat(0x00EF);
		TSLCDOutIns(TS_INS_VER_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_VER_END_AD);
		TSLCDOutDat(0x013F);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL1);
		TSLCDOutDat(0xA700);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL2);
		TSLCDOutDat(0x0001);
		TSLCDOutIns(TS_INS_GATE_SCAN_CTRL3);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_PART_IMG1_DISP_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG1_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG1_END_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_DISP_POS);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_START_AD);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PART_IMG2_END_AD);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_PANEL_IF_CTRL1);
		TSLCDOutDat(0x0010);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL2);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL3);
		TSLCDOutDat(0x0003);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL4);
		TSLCDOutDat(0x0110);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL5);
		TSLCDOutDat(0x0000);
		TSLCDOutIns(TS_INS_PANEL_IF_CTRL6);
		TSLCDOutDat(0x0000);

		TSLCDOutIns(TS_INS_DISP_CTRL1);
		TSLCDOutDat(0x0133);
	}
}

void TSLCDShowPic2(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,ts_mode_t mode)
//show picture from code memory with specific size
{
	unsigned long k = 0;
	unsigned short color;
	unsigned int x,y;
	unsigned int i,j;
	if (sx < ts_margin_xl)
		sx = ts_margin_xl;
	if (ex > ts_margin_xr)
		ex = ts_margin_xr;
	if (sy < ts_margin_yu)
		sy = ts_margin_yu;
	if (ey > ts_margin_yl)
		ey = ts_margin_yl;

	TSLCDOutIns(TS_INS_START_ADX);
	TSLCDOutDat(sx);
	TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	TSLCDOutIns(TS_INS_GRAM_ADX);
	TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_RW_GRAM);

	if (mode == TS_MODE_FULL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				TSLCDOutDat(code2(pic[k]));
				k++;
			}
	}
	else
	if (mode == TS_MODE_NORMAL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				if (code2(pic[k]) == TS_COL_WHITE)
				{
					color = TSLCDInDat(); 		// ignore invalid data
					color = TSLCDInDat();
					TSLCDOutDat(color);
				}
				else
				{
					TSLCDOutDat(code2(pic[k]));
				}
				k++;
			}
	}
}

//show picture from code memory, suited for showing background image
void TSLCDShowPicBack(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,unsigned int total_x,unsigned int total_y,ts_mode_t mode)
{
	unsigned long k,p;
	unsigned short color;
	unsigned int x,y;
	unsigned int i,j;

	p=sy*total_x+sx;
	k=p;
	if (sx < ts_margin_xl)
		sx = ts_margin_xl;
	if (ex > ts_margin_xr)
		ex = ts_margin_xr;
	if (sy < ts_margin_yu)
		sy = ts_margin_yu;
	if (ey > ts_margin_yl)
		ey = ts_margin_yl;

	TSLCDOutIns(TS_INS_START_ADX);
	TSLCDOutDat(sx);
	TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	TSLCDOutIns(TS_INS_GRAM_ADX);
	TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_RW_GRAM);

	if (mode == TS_MODE_FULL)
	{


		for (j=sy; j<(sy+y); j++)
			for (i=p; i<p+x; i++)
			{
				TSLCDOutDat(code2(pic[k]));
				k++;

			}
	}
	else
	if (mode == TS_MODE_NORMAL)
	{
		for (j=sy; j<(sy+y); j++)
		{
			for (i=p; i<(p+x); i++)
			{
				if (code2(pic[k]) == TS_COL_WHITE)
				{
					color = TSLCDInDat(); 		// ignore invalid data
					color = TSLCDInDat();
					TSLCDOutDat(color);
				}
				else
				{
					TSLCDOutDat(code2(pic[k]));
				}
				k++;
			}
			k+=total_x-x;
		}
	}
}

void TSLCDFillRect(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,unsigned short color,ts_mode_t mode) //draw a rectangular
{
	unsigned int x,y;
	unsigned int i,j;
	if (sx < ts_margin_xl)
		sx = ts_margin_xl;
	if (ex > ts_margin_xr)
		ex = ts_margin_xr;
	if (sy < ts_margin_yu)
		sy = ts_margin_yu;
	if (ey > ts_margin_yl)
		ey = ts_margin_yl;

	TSLCDOutIns(TS_INS_START_ADX);
	TSLCDOutDat(sx);
	TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	TSLCDOutIns(TS_INS_GRAM_ADX);
	TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_RW_GRAM);

	if ((mode == TS_MODE_NORMAL) || (mode == TS_MODE_FULL))
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				TSLCDOutDat(color);
			}
	}
	else
	if (mode == TS_MODE_INVERSE)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				color = TSLCDInDat(); 		// ignore invalid data
				color = TSLCDInDat();
				TSLCDOutDat(~color);
			}
	}
}

void TSLCDFillCirc(ts_pos_t cx,ts_pos_t cy,ts_pos_t rad,unsigned short color, ts_mode_t mode) //draw a circle
{
#ifndef TS_ORN_PORTRAIT
	int sy_buf,ey_buf;
#endif
	int sx,sy,ex,ey;
	int i,j;
	unsigned short color_buf;
	unsigned short rad2 = rad*rad;
	sx = cx - rad;
	ex = cx + rad;
	sy = cy - rad;
	ey = cy + rad;

	if (sx < ts_margin_xl)
		sx = ts_margin_xl;
	if (ex > ts_margin_xr)
		ex = ts_margin_xr;
	if (sy < ts_margin_yu)
		sy = ts_margin_yu;
	if (ey > ts_margin_yl)
		ey = ts_margin_yl;

	TSLCDOutIns(TS_INS_START_ADX);
	TSLCDOutDat(sx);
	TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	TSLCDOutIns(TS_INS_GRAM_ADX);
	TSLCDOutDat(sx);
//	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy_buf = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey_buf = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(ey_buf);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy_buf);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy_buf);//fix from bug of v1_00
//	y = sy_buf - ey_buf + 1;
#else
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
//	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_RW_GRAM);

	if (mode == TS_MODE_NORMAL)
	{
		for (j=sy-cy; j<=ey-cy; j++)
			for (i=sx-cx; i<=ex-cx; i++)
			{
				if ((i)*(i) + (j)*(j) < rad2)
				{
					TSLCDOutDat(color);
				}
				else
				{
					color_buf = TSLCDInDat(); 		// ignore invalid data
					color_buf = TSLCDInDat();
					TSLCDOutDat(color_buf);
				}
			}
	}
	else
	if (mode == TS_MODE_INVERSE)
	{
		for (j=sy-cy; j<=ey-cy; j++)
			for (i=sx-cx; i<=ex-cx; i++)
			{
				if ((i)*(i) + (j)*(j) < rad2)
				{
					color_buf = TSLCDInDat(); 		// ignore invalid data
					color_buf = TSLCDInDat();
					TSLCDOutDat(~color_buf);
				}
				else
				{
					color_buf = TSLCDInDat(); 		// ignore invalid data
					color_buf = TSLCDInDat();
					TSLCDOutDat(color_buf);
				}
			}
	}
	else
	if (mode == TS_MODE_FULL)
	{
		for (j=sy-cy; j<=ey-cy; j++)
			for (i=sx-cx; i<=ex-cx; i++)
			{
				if ((i)*(i) + (j)*(j) < rad2)
				{
					TSLCDOutDat(color);
				}
				else
				{
					TSLCDOutDat(back_color);
				}
			}
	}
}

void TSLCDSetMargins(ts_pos_t xl,ts_pos_t xr,ts_pos_t yu,ts_pos_t yl) //set margins for FillRect,FillCirc
{
	ts_margin_xl = xl;
	ts_margin_xr = xr;
	ts_margin_yu = yu;
	ts_margin_yl = yl;
}

void TSLCDSetMarginsDefault(void) //Reset margins to default value
{
	ts_margin_xl = 0;
	ts_margin_xr = TS_SIZE_X - 1;
	ts_margin_yu = 0;
	ts_margin_yl = TS_SIZE_Y - 1;
}

void display_buf(ts_pos_t sx,ts_pos_t sy,ts_mode_t mode)
{
	unsigned int x,y;
	unsigned char height;
	unsigned char i,j;
	ts_pos_t ex,ey;
	unsigned short c;

	if (font_mode == ENG)
		height = font_height;
	else
		height = font_ext_height;

	ex = sx + (last_non_zero - first_non_zero) + char_gap ;
	ey = sy + height - 1;

	TSLCDOutIns(TS_INS_START_ADX);
	TSLCDOutDat(sx);
	TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	TSLCDOutIns(TS_INS_GRAM_ADX);
	TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_START_ADY);
	TSLCDOutDat(sy);
	TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_RW_GRAM);

	if (mode == TS_MODE_NORMAL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				if (buf_read(i,j))
				{
					TSLCDOutDat(font_color);
				}
				else
				{
					c = TSLCDInDat(); 		// ignore invalid data
					c = TSLCDInDat();
					TSLCDOutDat(c);
				}
			}
	}
	else
	if (mode == TS_MODE_INVERSE)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				c = TSLCDInDat(); 			// ignore invalid data
				c = TSLCDInDat();
				if (buf_read(i,j))
				{
					TSLCDOutDat(~c);
				}
				else
				{
					TSLCDOutDat(c);
				}
			}
	}
	else
	if (mode == TS_MODE_FULL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				if (buf_read(i,j))
				{
					TSLCDOutDat(font_color);
				}
				else
				{
					TSLCDOutDat(back_color);
				}
			}
	}
}

void TSLCDCharDisp(char character,ts_pos_t sx,ts_pos_t sy,ts_mode_t mode) //low level function to print a character on LCD
{
/*
#ifdef FONT_EXT_PLUGIN
	char str[2];
	str[1] = 0;
	if (TSLCDGetTypeExtFont(character) != OUT)
	{
		str[0] = character;
		TSLCDStoreExtFont(str);
		font_mode = EXT;
		display_buf(sx,sy - font_ext_adj,mode);
	}
	else
#endif
  */
	{
		font_mode = ENG;
		buf_store(character - 0x20);
		display_buf(sx,sy,mode);
	}
}

void TSLCDSetOffset(ts_pos_t x,ts_pos_t y) //set LCD offset for character display
{
	offsetx = x;
	offsety = y;
}

void TSLCDPrintStr(unsigned char line,unsigned char column,char *str,ts_mode_t mode) //print string on LCD
{
	int i = 0;

	ts_pos_t posx,posy;
	posx = offsetx + column*font_width;
	posy = offsety + line*font_height;


	while(str[i])
	{
/*#ifdef FONT_EXT_PLUGIN
		if (TSLCDGetTypeExtFont(str[i]) != OUT)
		{
			if (TSLCDStoreExtFont(str+i))
			{
				font_mode = EXT;
				display_buf(posx,posy - font_ext_adj,mode);
				posx += last_non_zero-first_non_zero+1+char_gap;
				buf_clear();
			}
			i++;
		}
		else
#endif
          */
		{
			font_mode = ENG;
			buf_store(str[i] - 0x20);
			display_buf(posx,posy,mode);

			posx += last_non_zero-first_non_zero+1+char_gap;
			i++;
		}

		if ((print_limit) && (posx > print_limit_len))
			return;
	}
}

void TSLCDPrintTxt(unsigned char line,unsigned char column,const char *txt,ts_mode_t mode) //print text from code memory
{
	TSLCDPrintStr(line,column,(char *)txt,mode);
}

void TSLCDPrintCh(unsigned char line,unsigned char column,char c,ts_mode_t mode) //print a character on LCD
{
	ts_pos_t posx,posy;
	posx = offsetx + column*font_width;
	posy = offsety + line*font_height;

	TSLCDCharDisp(c,posx,posy,mode);
}

void TSLCDPrintLimit(unsigned char on,int limit) //set limit for printstr (in pixel)
{
	print_limit = on;
	print_limit_len  = limit;
}
