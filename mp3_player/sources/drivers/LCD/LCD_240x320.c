
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "utils_custom_v1_02.h"
#include "LCD_240x320.h"
#include "font_engine_v1_00.h"
#include "font_thai_plugin_v1_02.h"
//#include "font_thai_plugin_v1_02.h"

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

//
//#define TS_ORN_PORTRAIT
//
/*
#ifdef TS_ORN_PORTRAIT
#define TS_SIZE_X					240
#define TS_SIZE_Y					320
#define TS_VAL_ENTRY_MOD			0x0030
#define TS_INS_GRAM_ADX				TS_INS_GRAM_HOR_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_VER_AD
#define TS_INS_ROW_ADDR_SET   			TS_INS_HOR_START_AD
#define TS_INS_END_ADX   			TS_INS_HOR_END_AD
#define TS_INS_COL_ADDR_SET   			TS_INS_VER_START_AD
#define TS_INS_END_ADY   			TS_INS_VER_END_AD
#else
#define TS_SIZE_X					320
#define TS_SIZE_Y					240
#define TS_VAL_ENTRY_MOD			0x0028
#define TS_INS_GRAM_ADX				TS_INS_GRAM_VER_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_HOR_AD
#define TS_INS_ROW_ADDR_SET   			TS_INS_VER_START_AD
#define TS_INS_END_ADX   			TS_INS_VER_END_AD
#define TS_INS_COL_ADDR_SET   			TS_INS_HOR_START_AD
#define TS_INS_END_ADY   			TS_INS_HOR_END_AD
#endif
*/

typedef enum
{
	ENG,
	EXT
} font_mode_t;

font_mode_t  font_mode;
unsigned char font_ext_adj = 0;
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

void TSLCDCharDisp(char charactor,ts_pos_t sx,ts_pos_t sy,ts_mode_t mode); 
void display_buf(ts_pos_t sx,ts_pos_t sy,ts_mode_t mode);

void TSLCDSetFontColor(unsigned short color) //set text's color
{
	font_color = color;
}

void TSLCDSetBackColor(unsigned short color) //set back color for TS_MODE_FULL
{
	back_color = color;
}
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
void delay_1us(unsigned long int microsecond)
{    
        SysCtlDelay(((unsigned long )microsecond*(SysCtlClockGet() / (3*1000000))));
}
void delay_us(unsigned long int microsecond)
{    
        SysCtlDelay(((unsigned long )microsecond*(SysCtlClockGet() / (3*1000000))));
}

void TSLCDOutDat(unsigned short dat) 
{
        TS_LCD_RD_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_SET();
        TS_LCD_DATA_PORT_OUT();
        TS_LCD_WR_SET();
	TS_LCD_WR_CLR();
	TS_LCD_WRITE_DATA8BIT((unsigned char) (dat >> 8));
        TS_LCD_WR_SET();
        TS_LCD_WR_CLR();
        TS_LCD_WRITE_DATA8BIT((unsigned char) (dat & 0x00FF));
        TS_LCD_WR_SET();
	TS_LCD_CS_SET();
        TS_LCD_DATA_PORT_IN() ;
        
}
void TSLCDOutDat8(unsigned char dat) //write data to LCD
{
        TS_LCD_RD_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_SET();
        TS_LCD_DATA_PORT_OUT();
        TS_LCD_WR_SET();
	TS_LCD_WR_CLR();
        TS_LCD_WRITE_DATA8BIT(dat);
        TS_LCD_WR_SET();
	TS_LCD_CS_SET();
        TS_LCD_DATA_PORT_IN() ;
}
void TSLCDOutDat2(unsigned char high,unsigned char low)
{
	TS_LCD_RD_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_SET();
        TS_LCD_DATA_PORT_OUT();
        TS_LCD_WR_SET();
	TS_LCD_WR_CLR();
	TS_LCD_WRITE_DATA8BIT(high);
        TS_LCD_WR_SET();
        TS_LCD_WR_CLR();
        TS_LCD_WRITE_DATA8BIT(low);
        TS_LCD_WR_SET();
	TS_LCD_CS_SET();
        //TS_LCD_DATA_PORT_IN() ;
}

void TSLCDOutIns(unsigned short ins) //write instruction to LCD
{
	TS_LCD_RD_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_CLR();
        TS_LCD_DATA_PORT_OUT();
        TS_LCD_WR_SET();
	TS_LCD_WR_CLR();
	TS_LCD_WRITE_DATA8BIT((unsigned char) (ins >> 8));
        TS_LCD_WR_SET();
        TS_LCD_WR_CLR();
        TS_LCD_WRITE_DATA8BIT((unsigned char) (ins & 0x00FF));
        TS_LCD_WR_SET();
	TS_LCD_CS_SET();
        TS_LCD_RS_SET();
        //TS_LCD_DATA_PORT_IN() ;
}
void TSLCDOutIns8(unsigned char ins) //write instruction to LCD
{
	TS_LCD_RD_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_CLR();
        TS_LCD_DATA_PORT_OUT();
        TS_LCD_WR_SET();
	TS_LCD_WR_CLR();
        TS_LCD_WRITE_DATA8BIT(ins );
        TS_LCD_WR_SET();
	TS_LCD_CS_SET();
        TS_LCD_RS_SET();
}
unsigned short TSLCDInDat(void) //read data from LCD
{      
        unsigned short dat;
        TS_LCD_WR_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_SET();
        TS_LCD_DATA_PORT_IN();
        //TS_LCD_RD_CLR();
        //TS_LCD_RD_SET();
        TS_LCD_RD_SET();
	TS_LCD_RD_CLR();
	dat = (unsigned short)TS_LCD_READ_DATA8BIT();
        TS_LCD_RD_SET();
        TS_LCD_RD_CLR();
        dat = dat<< 8;
        dat |= (unsigned short)TS_LCD_READ_DATA8BIT();
        //delay_1ms(1);
        TS_LCD_RD_SET();
	TS_LCD_CS_SET();
        TS_LCD_RS_SET();
        //TS_LCD_DATA_PORT_IN() ;
	return (dat);
}
unsigned long TSLCDInDat32(void) //read data from LCD
{
  
        unsigned long dat;
        TS_LCD_WR_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_SET();
        TS_LCD_DATA_PORT_IN();
        //TS_LCD_RD_CLR();
        //TS_LCD_RD_SET();
        TS_LCD_RD_SET();
	TS_LCD_RD_CLR();
        
	dat = (unsigned short)TS_LCD_READ_DATA8BIT() <<24;
        TS_LCD_RD_SET();
        TS_LCD_RD_CLR();
       
        dat |= (unsigned short)TS_LCD_READ_DATA8BIT()<<16;
         TS_LCD_RD_SET();
        TS_LCD_RD_CLR();
        
        dat |= (unsigned short)TS_LCD_READ_DATA8BIT()<<8;
        TS_LCD_RD_SET();
        TS_LCD_RD_CLR();
        
        dat |= (unsigned short)TS_LCD_READ_DATA8BIT();
        //delay_1ms(1);
        TS_LCD_RD_SET();
	TS_LCD_CS_SET();
        TS_LCD_RS_SET();
        //TS_LCD_DATA_PORT_IN() ;
	return (dat);
}


unsigned short TSLCDInIns(void) //read data from LCD
{  
        unsigned short ins;
         
        TS_LCD_WR_SET();
        TS_LCD_CS_SET();
        TS_LCD_CS_CLR();
	TS_LCD_RS_CLR();
        TS_LCD_DATA_PORT_IN();
        //TS_LCD_RD_CLR();
        //TS_LCD_RD_SET();
        TS_LCD_RD_SET();
	TS_LCD_RD_CLR();
	ins = (unsigned short)TS_LCD_READ_DATA8BIT();
        TS_LCD_RD_SET();
        TS_LCD_RD_CLR();
        ins = ins << 8;
        ins |= (unsigned short)TS_LCD_READ_DATA8BIT();
        //delay_1ms(1);
        TS_LCD_RD_SET();
	TS_LCD_CS_SET();
        TS_LCD_RS_SET();
        //TS_LCD_DATA_PORT_IN() ;
	return (ins);
}

void TSLCDRst(void) //pulse reset signal to LCD
{
	        TS_LCD_RST_SET();
                delay_1ms(1);
	        TS_LCD_RST_CLR();
	        delay_1ms(10);
	        TS_LCD_RST_SET();
                delay_1ms(50);
}

void TSLCDInit(void) //initial LCD
{
		
             
                TS_LCD_RST_SET();
                delay_1ms(1);
	        TS_LCD_RST_CLR();
	        delay_1ms(10);
	        TS_LCD_RST_SET();
                delay_1ms(50);
                //sleep out
                TSLCDOutIns8(0x11);
                delay_1ms(200);

            /*    //normal mode
                TSLCDOutIns8(0x13);
                //AP [2:0}
                TSLCDOutIns8(0xCB);
		TSLCDOutDat8(0x01); 
                 delay_us(1);
                //power control 1
		TSLCDOutIns8(0xC0);
                TSLCDOutDat8(0x26);
		TSLCDOutDat8(0x01);
                //power control 2
		TSLCDOutIns8(0xC1);
		TSLCDOutDat8(0x10); 
                //VCOM control 1
		TSLCDOutIns8(0xC5);
		TSLCDOutDat8(0x16); 
                TSLCDOutDat8(0x28); 
                 delay_us(1);
                //TSLCDOutDat(0x0016); 
                //TSLCDOutDat(0x0028); 
                //GAMA set
		TSLCDOutIns8(0x26);
		TSLCDOutDat8(0x01);
                //possitive Gama correction
		 TSLCDOutIns8(0xE0);
                 TSLCDOutDat8(0x0f);
                 TSLCDOutDat8(0x22);
                 TSLCDOutDat8(0x1b);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x08);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x49);
                 TSLCDOutDat8(0x41); 
                 TSLCDOutDat8(0x3d);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x17);
                 TSLCDOutDat8(0x04);
                 TSLCDOutDat8(0x13);
                 TSLCDOutDat8(0x0e);
                 TSLCDOutDat8(0x00);
                  delay_1ms(100);
                 //negative Gama correction
		 TSLCDOutIns8(0xE1);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x23);
                 TSLCDOutDat8(0x22);
                 TSLCDOutDat8(0x05);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x39);
                 TSLCDOutDat8(0x20);
                 TSLCDOutDat8(0x49); 
                 TSLCDOutDat8(0x03);
                 TSLCDOutDat8(0x0b);
                 TSLCDOutDat8(0x0b);
                 TSLCDOutDat8(0x33);
                 TSLCDOutDat8(0x37);
                 TSLCDOutDat8(0x0f);
                  delay_1ms(100);
                 //display on
                 //TSLCDOutIns(0x0029);
                //columm address set
                 TSLCDOutIns8(0x29);
                 TSLCDOutIns8(0x2B);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0xEF);
                  delay_us(100);
                 ///TSLCDOutDat(0x0000);
                 //TSLCDOutDat(0x00EF);
                 //page address set
                 TSLCDOutIns8(0x2A);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 //TSLCDOutDat(0x0000);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x3F);
                  delay_1ms(100);
                  TSLCDOutIns8(0x2B);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0xEF);
                  delay_us(100);
                 ///TSLCDOutDat(0x0000);
                 //TSLCDOutDat(0x00EF);
                 //page address set
                 TSLCDOutIns8(0x2A);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 //TSLCDOutDat(0x0000);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x3F);
                  delay_1ms(100);
                 //memory access control
                 TSLCDOutIns8(0x36);
                 TSLCDOutDat8(0x08);
                  delay_1ms(1);
                 //TSLCDOutDat(0x00E8);
                 //pixel format
                 TSLCDOutIns8(0x3A);
                 TSLCDOutDat8(0x05);
                  delay_1ms(1);
                 // memory write
                 TSLCDOutIns8(0x2C);
                 */
                
                 //
                 //
                //normal mode
               TSLCDOutIns8(0x13);
                //AP [2:0}
                TSLCDOutIns8(0xCB);
		TSLCDOutDat8(0x01); 
                //power control 1
		TSLCDOutIns8(0xC0);
                TSLCDOutDat8(0x26);
		TSLCDOutDat8(0x01);
                //power control 2
		TSLCDOutIns8(0xC1);
		TSLCDOutDat8(0x10); 
                //VCOM control 1
		TSLCDOutIns8(0xC5);
		TSLCDOutDat8(0x10); 
                TSLCDOutDat8(0x52); 
                 delay_us(1);
                //TSLCDOutDat(0x0016); 
                //TSLCDOutDat(0x0028); 
                //GAMA set
		TSLCDOutIns8(0x26);
		TSLCDOutDat8(0x01);
                //possitive Gama correction
		 TSLCDOutIns8(0xE0);
                 TSLCDOutDat8(0x10);
                 TSLCDOutDat8(0x10);
                 TSLCDOutDat8(0x10);
                 TSLCDOutDat8(0x08);
                 TSLCDOutDat8(0x0e);
                 TSLCDOutDat8(0x06);
                 TSLCDOutDat8(0x42);
                 TSLCDOutDat8(0x28); 
                 TSLCDOutDat8(0x36);
                 TSLCDOutDat8(0x03);
                 TSLCDOutDat8(0x0e);
                 TSLCDOutDat8(0x04);
                 TSLCDOutDat8(0x13);
                 TSLCDOutDat8(0x0e);
                 TSLCDOutDat8(0x0c);
                  delay_1ms(100);
                 //negative Gama correction
		 TSLCDOutIns8(0xE1);
                 TSLCDOutDat8(0x0c);
                 TSLCDOutDat8(0x23);
                 TSLCDOutDat8(0x26);
                 TSLCDOutDat8(0x04);
                 TSLCDOutDat8(0x0c);
                 TSLCDOutDat8(0x04);
                 TSLCDOutDat8(0x39);
                 TSLCDOutDat8(0x24);
                 TSLCDOutDat8(0x4b); 
                 TSLCDOutDat8(0x03);
                 TSLCDOutDat8(0x0b);
                 TSLCDOutDat8(0x0b);
                 TSLCDOutDat8(0x33);
                 TSLCDOutDat8(0x37);
                 TSLCDOutDat8(0x0f);
                  delay_1ms(100);
                  
                  //
                  // memory order
                  //
                  
                  TSLCDOutIns8(0x36);
                  TSLCDOutIns8(0x00);
                 //display on
                 //TSLCDOutIns(0x0029);
                //columm address set
                 //TSLCDOutIns8(0x29);
                 TSLCDOutIns8(0x2B);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0xEF);
                  delay_us(100);
                 ///TSLCDOutDat(0x0000);
                 //TSLCDOutDat(0x00EF);
                 //page address set
                 TSLCDOutIns8(0x2A);
                 TSLCDOutDat8(0x00);
                 TSLCDOutDat8(0x00);
                 //TSLCDOutDat(0x0000);
                 TSLCDOutDat8(0x01);
                 TSLCDOutDat8(0x3F);
                  delay_1ms(100);
                 //memory access control
                 TSLCDOutIns8(0x36);
                 TSLCDOutDat8(0xe8);
                  delay_1ms(1);
                 //TSLCDOutDat(0x00E8);
                 //pixel format
                 TSLCDOutIns8(0x3A);
                 TSLCDOutDat8(0x05);
                 //TSLCDOutDat(0x0005);
                // TSLCDOutIns (TS_INS_RD_DISP_MODE);
                 //display on
                 TSLCDOutIns8(0x29);
                  delay_1ms(10);
                 // memory write
                 TSLCDOutIns8(0x2C);
                
                 
                 
                 
}


void TSLCDShowPic(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,ts_mode_t mode)
{
	unsigned long k = 0;
	unsigned short color;
	unsigned int x,y;
	unsigned int i,j;
#ifndef TS_ORN_PORTRAIT
        if (sx < 0)
		sx = 0;
	if (ex > 239)
		ex = 239;
	if (sy < 0)
		sy = 0;
	if (ey > 319)
		ey =319;
         TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	 TSLCDOutDat(sx);
	 TSLCDOutDat(ex);
	 x = ex - sx + 1;
	 TSLCDOutIns(TS_INS_COL_ADDR_SET);
	 TSLCDOutDat(sy);
	 TSLCDOutDat(ey);
	 y = ey - sy + 1;
#else 
        if (sx < 0)
		sx = 0;
	if (ex > 320)
		ex = 320;
	if (sy < 0)
		sy = 0;
	if (ey > 219)
		ey =219;
        TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sx);
	TSLCDOutDat(ex);
	x = ex - sx + 1;
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sy);
	TSLCDOutDat(ey);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

	if (mode == TS_MODE_FULL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				TSLCDOutDat(pic[k]);
				k++;
			}
	}
	else
	if (mode == TS_MODE_NORMAL)
	{
		for (j=0; j<y; j++)
			for (i=0; i<x; i++)
			{
				if (pic[k] == TS_COL_WHITE)
				{
					color = TSLCDInDat(); 		// ignore invalid data
					color = TSLCDInDat();
					TSLCDOutDat(color);
				}
				else
				{
					TSLCDOutDat(pic[k]);
				}
				k++;
			}
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

	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sx);
	//TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	//TSLCDOutIns(TS_INS_GRAM_ADX);
	//TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(ey);
	///LCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	//LCDOutIns(TS_INS_GRAM_ADY);
	//LCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

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

	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sx);
	//TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	//TSLCDOutIns(TS_INS_GRAM_ADX);
	//TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

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

#ifndef TS_ORN_PORTRAIT
        if (sx < 0)
		sx = 0;
	if (ex > 239)
		ex = 239;
	if (sy < 0)
		sy = 0;
	if (ey > 319)
		ey =319;
         TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	 TSLCDOutDat(sx);
	 TSLCDOutDat(ex);
	 x = ex - sx + 1;
	 TSLCDOutIns(TS_INS_COL_ADDR_SET);
	 TSLCDOutDat(sy);
	 TSLCDOutDat(ey);
	 y = ey - sy + 1;
#else 
        if (sx < 0)
		sx = 0;
	if (ex > 320)
		ex = 320;
	if (sy < 0)
		sy = 0;
	if (ey > 219)
		ey =219;
        TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sx);
	TSLCDOutDat(ex);
	x = ex - sx + 1;
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sy);
	TSLCDOutDat(ey);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

	if ((mode == TS_MODE_NORMAL) || (mode == TS_MODE_FULL))
	{
		for (j=0; j<x; j++)
			for (i=0; i<y; i++)
			{
				TSLCDOutDat(color);
			}
	}
	else
	if (mode == TS_MODE_INVERSE)
	{
		for (j=0; j<x; j++)
			for (i=0; i<y; i++)
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

	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sx);
	//TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	//TSLCDOutIns(TS_INS_GRAM_ADX);
	//TSLCDOutDat(sx);
//	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy_buf = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey_buf = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(ey_buf);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy_buf);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy_buf);//fix from bug of v1_00
//	y = sy_buf - ey_buf + 1;
#else
	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy);
//	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

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

	TSLCDOutIns(TS_INS_COL_ADDR_SET);
	TSLCDOutDat(sx);
	//TSLCDOutIns(TS_INS_END_ADX);
	TSLCDOutDat(ex);
	//TSLCDOutIns(TS_INS_GRAM_ADX);
	//TSLCDOutDat(sx);
	x = ex - sx + 1;

#ifndef TS_ORN_PORTRAIT
	sy = TS_SIZE_Y - 1 - sy; 	// mirror start y address
	ey = TS_SIZE_Y - 1 - ey; 	// mirror end y address
	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy);//fix from bug of v1_00
	y = sy - ey + 1;
#else
	TSLCDOutIns(TS_INS_ROW_ADDR_SET);
	TSLCDOutDat(sy);
	//TSLCDOutIns(TS_INS_END_ADY);
	TSLCDOutDat(ey);
	//TSLCDOutIns(TS_INS_GRAM_ADY);
	//TSLCDOutDat(sy);
	y = ey - sy + 1;
#endif

	TSLCDOutIns(TS_INS_MEM_WR);

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
#ifdef FONT_EXT_PLUGIN
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
