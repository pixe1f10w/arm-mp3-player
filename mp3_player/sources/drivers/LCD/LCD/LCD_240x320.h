

#ifndef _LCD_240x320_H_
#define _LCD_240x320_H_

//
//hardware  defines
//
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
//
//data
//
#define  TS_LCD_DATA_PORT        GPIO_PORTH_BASE
#define  TS_LCD_D0   		  GPIO_PIN_0				
#define  TS_LCD_D1   		  GPIO_PIN_1   
#define  TS_LCD_D2   		  GPIO_PIN_2  					
#define  TS_LCD_D3    		  GPIO_PIN_3  						
#define  TS_LCD_D4   		  GPIO_PIN_4    						
#define  TS_LCD_D5   		  GPIO_PIN_5    					
#define  TS_LCD_D6	  	  GPIO_PIN_6 	
#define  TS_LCD_D7    		  GPIO_PIN_7
#define  TS_LCD_DATA          GPIO_PIN_7|GPIO_PIN_6 | GPIO_PIN_5|GPIO_PIN_4 |GPIO_PIN_3 | GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0
//	
//control
//
#define  TS_LCD_CONTROL_PORT             GPIO_PORTD_BASE
#define  TS_LCD_CS     		         GPIO_PIN_0				
#define  TS_LCD_A11    			 GPIO_PIN_1   
#define  TS_LCD_RD      		 GPIO_PIN_2  					
#define  TS_LCD_WR     			 GPIO_PIN_3  						
#define  TS_LCD_RS      	          GPIO_PIN_4    						
#define  TS_LCD_BL                       GPIO_PIN_5 
#define  TS_LCD_RST                      GPIO_PIN_6  					
//
//command define
// number of data (in-out) + command code
//
#define TS_INS_NO_OPERATION                 0x00  // none parameter
#define TS_INS_SW_RESET                     0x01  //none parameter
#define TS_INS_RD_ID		            0x04  // read 3 unsigned chars ID1,ID2,ID3
#define TS_INS_RD_DISP_STAT	            0x09  //read  5 unsigned chars current status
#define TS_INS_RD_POWER_MODE		    0x0A  // read 2 unsigned chars Power mode
#define TS_INS_RD_DISP_MODE	            0x0B  // read 2 unsigned chars current display mode
#define TS_INS_RD_PIXEL_FORM	            0x0C  // read 2 unsigned chars
#define TS_INS_RD_IMAGE_FORM	            0x0D  //read 2 unsigned chars
#define TS_INS_RD_DISP_SIG_MODE	            0x0E  //read 2 unsigned chars
#define TS_INS_RD_DISP_SELF_RES             0x0F  //read 2 unsigned chars
#define TS_INS_SLP_MODE_ON                  0x10  // none parameter
#define TS_INS_SLP_OUT                      0x11  //none parameter
#define TS_INS_PARTIAL_MODE_ON		    0x12  //
#define TS_INS_NORM_DISP_MOODE_ON           0x13  //
#define TS_INS_DISP_INVER_OFF               0x20  //
#define TS_INS_DISP_INVER_ON                0x21  //
#define TS_INS_GAMMA_SET		    0x26  //1 unsigned char data out
#define TS_INS_DISP_OFF                     0x28  //
#define TS_INS_DISP_ON                      0x29  //
#define TS_INS_COL_ADDR_SET                 0x2A  // 2 unsigned short starting and ending column  OUT
#define TS_INS_PAGE_ADDR_SET                0x2B  // 2 unsigned short starting and ending Page  OUT
#define TS_INS_ROW_ADDR_SET                 0x2B  // 2 unsigned short starting and ending Page  OUT
#define TS_INS_MEM_WR                       0x2C  // n unsigned chars send to RAM for DISPLAY
#define TS_INS_COLOR_SET                    0x2D  //128 unsigned chars out for color set  : 32 : RED , 64 GREEN, 32 BLUE
#define TS_INS_RD_MEM                       0x2E  // read n unsigned chars from RAM
#define TS_INS_PARTIAL_AREA                 0x30  // 4 unsigned chars for starting and ending row
#define TS_INS_VER_SCROLL_DEF               0x33  // 2unsigned char top fixed,2 unsigned char vertial scroll, 2 unsigned chars botton fixed area 
#define TS_INS_TEAR_EFF_OFF                 0x34  //
#define TS_INS_TEAR_EFF_ON                  0x35  //
#define TS_INS_MEM_ORDER		    0x36  // MX-MY-MV-ML-BGR-MH-X-X
#define TS_INS_VER_SCROLL_START_ADD	    0x37  //2 unsigned chars out
#define TS_INS_IDLE_MODE_OFF                0x38  
#define TS_INS_IDLE_MODE_ON                 0x39 
#define TS_INS_PIXEL_FORM                   0x3A  // 1 unsigned char out
#define TS_INS_RD_ID1                       0xDA  // read 2 unsigned chars ID1
#define TS_INS_RD_ID2                       0xDB  // read 2 unsigned chars ID2
#define TS_INS_RD_ID3                       0xDC  // read 2 unsigned chars ID3
#define TS_INS_INTERFACE_MODE		    0xB0  
#define TS_INS_FRM_RATE_CONTROL1             0xB1  // 2 unsigned chars OUT
#define TS_INS_FRM_RATE_CONTROL2             0xB2  // 2 unsigned chars OUT
#define TS_INS_FRM_RATE_CONTROL3             0xB3  // 2 unsigned chars OUT
#define TS_INS_INVER_CONTROL                 0xB4  // 2unsigned chars out
#define TS_INS_BLANKING_CONTROL              0xB5  //4 unsigned chars OUT
#define TS_INS_DISP_FUNC_CONTROL	     0xB6  //3unsigned chars OUT
#define TS_INS_ENTRY_MODE             	     0xB7  //
#define TS_INS_POWER_CTRL1                    0xC0 //
#define TS_INS_POWER_CTRL2                    0xC1 //
#define TS_INS_POWER_CTRL3                    0xC2 //
#define TS_INS_POWER_CTRL4                    0xC3 //
#define TS_INS_POWER_CTRL5                    0xC4 //
#define TS_INS_PVCOM1                         0xC5 //
#define TS_INS_PVCOM2                         0xC7 //
#define TS_INS_NV_MEM_WR 		      0xD0 //
#define TS_INS_NV_MEM_PROTECT 		      0xD1 //
#define TS_INS_NV_MEM_STATUS_RD 	      0xD2 //
#define TS_INS_RD_ID4                         0xD3 //
#define TS_INS_POS_GAM	                      0xE0 //
#define TS_INS_NEG_GAM	                      0xE1 //
//===========================================
//	Color
//===========================================
#define TS_COL_BLUE					0x001F
#define TS_COL_GREEN				0x07E0
#define TS_COL_RED					0xF800
#define TS_COL_YELLOW				0x07FF
#define TS_COL_PURPLE				0xF81F
#define TS_COL_BLACK				0x0000
#define TS_COL_WHITE				0xFFFF
#define TS_COL_AQUA					0xFFEB
//
#define ts_pos_t					int
//
//basic macros 
//
#define TS_LCD_RS_SET()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RS,TS_LCD_RS);
#define TS_LCD_RS_CLR()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RS,0);
#define TS_LCD_RST_SET()              GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RST,TS_LCD_RST);
#define TS_LCD_RST_CLR()              GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RST,0);
#define TS_LCD_CS_SET()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_CS,TS_LCD_CS);
#define TS_LCD_CS_CLR()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_CS,0);
#define TS_LCD_RD_SET()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RD,TS_LCD_RD);
#define TS_LCD_RD_CLR()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_RD,0);
#define TS_LCD_WR_SET()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_WR,TS_LCD_WR);
#define TS_LCD_WR_CLR()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_WR,0);
#define TS_LCD_BL_SET()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_BL,TS_LCD_BL);
#define TS_LCD_BL_CLR()               GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_BL,0);
#define TS_LCD_A11_SET()              GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_A11,TS_LCD_A11);
#define TS_LCD_A11_CLR()              GPIOPinWrite(TS_LCD_CONTROL_PORT,(unsigned char)TS_LCD_A11,0);
#define TS_LCD_DATA_PORT_OUT()        GPIOPinTypeGPIOOutput(TS_LCD_DATA_PORT,(unsigned char)TS_LCD_DATA);
#define TS_LCD_DATA_PORT_IN()           GPIOPinTypeGPIOInput(TS_LCD_DATA_PORT,(unsigned char)TS_LCD_DATA);
#define TS_LCD_DATA_PORT_ENABLE()     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
#define TS_LCD_CONTROL_PORT_ENABLE()  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
#define TS_LCD_CONTROL_PORT_OUT()        GPIOPinTypeGPIOOutput(TS_LCD_CONTROL_PORT,0xff);



//===========================================
//	Define Data Type
//===========================================

typedef enum
{
	TS_MODE_NORMAL,
	TS_MODE_INVERSE,
	TS_MODE_FULL,
} ts_mode_t;

//===========================================
//	LCD Constance 
//===========================================

#define TSLCDGetMarginXl()			ts_margin_xl
#define TSLCDGetMarginXr()			ts_margin_xr
#define TSLCDGetMarginYu()			ts_margin_yu
#define TSLCDGetMarginYl()			ts_margin_yl

#define FONT_BIT_WIDTH 8
#define FONT_BYTE_HIGHT 2
#define FONT_SIZE FONT_BIT_WIDTH*FONT_BYTE_HIGHT

#define FONT_WIDTH FONT_BIT_WIDTH
#define FONT_HEIGHT FONT_BYTE_HIGHT*8
//#define TS_ORN_PORTRAIT
//#ifdef TS_ORN_PORTRAIT
//#define TS_SIZE_X					240
//#define TS_SIZE_Y					320
//#define TS_INS_GRAM_ADX				TS_INS_COL_ADDR_SET
//#define TS_INS_GRAM_ADY				TS_INS_ROW_ADDR_SET
//#else
#define TS_SIZE_X					240
#define TS_SIZE_Y					320
//#endif
#define code(codemem)				codemem
#define code2(codemem)				codemem
#define code4(codemem)				codemem
#define code_char(var_name)			const char var_name
#define code_uchar(var_name)		const unsigned char var_name
//font configuration
extern const unsigned char font_vrinda11x16[];
extern const unsigned char font_vrinda18x16[];
extern const unsigned char font_angsana14x32[];
extern const unsigned char font_angsana32x64[];
extern const unsigned char font_angsana_eng32x48[];
#define CHAR_BUF_WIDTH				60
#define CHAR_BUF_BYTE_HEIGHT		8
# define FONT_EXT_PLUGIN

extern unsigned char font_width;
extern unsigned char font_height;
extern unsigned char char_gap;

extern unsigned char first_non_zero;
extern unsigned char last_non_zero;

#ifdef FONT_EXT_PLUGIN
extern unsigned char font_ext_width;
extern unsigned char font_ext_height;
#endif
//
//
//
void TSLCDOutIns(unsigned short ins); //write instruction to LCD
void TSLCDOutDat(unsigned short dat); //write data to LCD
void TSLCDOutDat2(unsigned char dath,unsigned char datl); //write data to LCD
unsigned short TSLCDInIns(void); //read data from LCD
unsigned short TSLCDInDat(void); //read data from LCD

void TSLCDRst(void); //pulse reset signal to LCD
void TSLCDInit(void); //initial LCD
void TSLCDShowPic(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,ts_mode_t mode);
void TSLCDShowPic2(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,ts_mode_t mode);
//show picture from code memory with specific size
void TSLCDFillRect(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,unsigned short color,ts_mode_t mode); //draw a rectangular
void TSLCDFillCirc(ts_pos_t cx,ts_pos_t cy,ts_pos_t rad,unsigned short color, ts_mode_t mode); //draw a circle
void TSLCDSetMargins(ts_pos_t xl,ts_pos_t xr,ts_pos_t yu,ts_pos_t yl); //set margins for FillRect,FillCirc
void TSLCDSetMarginsDefault(void); //reset margins to default value

void TSLCDShowPicBack(ts_pos_t sx,ts_pos_t ex,ts_pos_t sy,ts_pos_t ey,const unsigned short *pic,unsigned int total_x,unsigned int total_y,ts_mode_t mode);
//show picture from code memory with specific range and size
void TSLCDSetFontColor(unsigned short color); //set text's color
void TSLCDSetBackColor(unsigned short color); //set back color for TS_MODE_FULL
void TSLCDSetOffset(ts_pos_t x,ts_pos_t y); //set LCD offset for character display
void TSLCDPrintStr(unsigned char line,unsigned char column,char *str,ts_mode_t mode); //print string on LCD
void TSLCDPrintTxt(unsigned char line,unsigned char column,const char *txt,ts_mode_t mode); //print text from code memory
void TSLCDPrintCh(unsigned char line,unsigned char column,char c,ts_mode_t mode); //print a character on LCD
void TSLCDPrintLimit(unsigned char on,int limit); //set limit for printstr (in pixel)
void TSLCDPrintStr(unsigned char line,unsigned char column,char *str,ts_mode_t mode); //print string
void delay_1ms(unsigned long int microsecond);
void TSLCDOutIns8(unsigned char ins);
void TSLCDOutDat8(unsigned char dat) ;//write data to LCD
#endif

