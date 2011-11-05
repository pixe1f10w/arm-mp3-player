//version 1.21 	-- #ifndef is added for FONT_BIT_WIDTH and FONT_BYTE_HEIGHT to support bigger font
//version 1.30	-- multi-font supported, bold character supported, variable width supported
//version 1.31	-- function TSLCDShowPicBack is added
//version 1.40	-- extention font supported
//version 2.00  -- vary_width is constant 1, add GPIO mode

#ifndef _TSLCD_ELT240320TP_H_
#define _TSLCD_ELT240320TP_H_

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
#define  TS_LCD_D6	  	   	  GPIO_PIN_6 	
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
#define  TS_LCD_RS     			 GPIO_PIN_4    						
#define  TS_LCD_BL                       GPIO_PIN_5   
#define  TS_LCD_RST                      GPIO_PIN_6  					
//
//
//
//#define TC_CS_DPRT		FIO2DIR
//#define TC_CS_PRTS		FIO2SET
//#define TC_CS_PRTC		FIO2CLR
//#define TC_CS_PIN		2

//#define TC_PEN_DPRT		FIO2DIR
//#define TC_PEN_PRTS		FIO2SET
//#define TC_PEN_PRTC		FIO2CLR
//#define TC_PEN_PINP		FIO2PIN
//#define TC_PEN_PIN		10

//#define LCD_RST_DPRT	FIO3DIR
//#define LCD_RST_PRTS	FIO3SET
//#define LCD_RST_PRTC	FIO3CLR
//#define LCD_RST_PIN		23
//++++++++++++++++++++++++++++++++
//#define TC_CS_DPRT		FIO2DIR
//#define TC_CS_PRTS		FIO2SET
//#define TC_CS_PRTC		FIO2CLR
//#define TC_CS_PIN		2

//#define TC_PEN_DPRT		FIO2DIR
//#define TC_PEN_PRTS		FIO2SET
//#define TC_PEN_PRTC		FIO2CLR
//#define TC_PEN_PINP		FIO2PIN
//#define TC_PEN_PIN		10

//#define LCD_RST_DPRT	FIO3DIR
//#define LCD_RST_PRTS	FIO3SET
//#define LCD_RST_PRTC	FIO3CLR
//#define LCD_RST_PIN		23
//
//command
//
#define TS_INS_START_OSC			0x00 //data read at this instruction should be 0x0789 --> use for test connection
#define TS_INS_DRIV_OUT_CTRL		0x01
#define TS_INS_DRIV_WAV_CTRL		0x02
#define TS_INS_ENTRY_MOD			0x03
#define TS_INS_RESIZE_CTRL			0x04
#define TS_INS_DISP_CTRL1			0x07
#define TS_INS_DISP_CTRL2			0x08
#define TS_INS_DISP_CTRL3			0x09
#define TS_INS_DISP_CTRL4			0x0A
#define TS_INS_RGB_DISP_IF_CTRL1	0x0C
#define TS_INS_FRM_MARKER_POS		0x0D
#define TS_INS_RGB_DISP_IF_CTRL2	0x0F
#define TS_INS_POW_CTRL1			0x10
#define TS_INS_POW_CTRL2			0x11
#define TS_INS_POW_CTRL3			0x12
#define TS_INS_POW_CTRL4			0x13
#define TS_INS_GRAM_HOR_AD			0x20
#define TS_INS_GRAM_VER_AD			0x21
#define TS_INS_RW_GRAM				0x22
#define TS_INS_POW_CTRL7			0x29
#define TS_INS_FRM_RATE_COL_CTRL	0x2B
#define TS_INS_GAMMA_CTRL1			0x30
#define TS_INS_GAMMA_CTRL2			0x31
#define TS_INS_GAMMA_CTRL3			0x32
#define TS_INS_GAMMA_CTRL4			0x35 
#define TS_INS_GAMMA_CTRL5			0x36
#define TS_INS_GAMMA_CTRL6			0x37
#define TS_INS_GAMMA_CTRL7			0x38
#define TS_INS_GAMMA_CTRL8			0x39
#define TS_INS_GAMMA_CTRL9			0x3C
#define TS_INS_GAMMA_CTRL10			0x3D
#define TS_INS_HOR_START_AD			0x50
#define TS_INS_HOR_END_AD			0x51
#define TS_INS_VER_START_AD			0x52
#define TS_INS_VER_END_AD			0x53
#define TS_INS_GATE_SCAN_CTRL1		0x60
#define TS_INS_GATE_SCAN_CTRL2		0x61
#define TS_INS_GATE_SCAN_CTRL3		0x6A
#define TS_INS_PART_IMG1_DISP_POS	0x80
#define TS_INS_PART_IMG1_START_AD	0x81
#define TS_INS_PART_IMG1_END_AD		0x82
#define TS_INS_PART_IMG2_DISP_POS	0x83
#define TS_INS_PART_IMG2_START_AD	0x84
#define TS_INS_PART_IMG2_END_AD		0x85
#define TS_INS_PANEL_IF_CTRL1		0x90
#define TS_INS_PANEL_IF_CTRL2		0x92
#define TS_INS_PANEL_IF_CTRL3		0x93
#define TS_INS_PANEL_IF_CTRL4		0x95
#define TS_INS_PANEL_IF_CTRL5		0x97
#define TS_INS_PANEL_IF_CTRL6		0x98
//
//color
//
#define TS_COL_RED					0x001F
#define TS_COL_GREEN				0x07E0
#define TS_COL_BLUE					0xF800
#define TS_COL_YELLOW				0x07FF
#define TS_COL_PURPLE				0xF81F
#define TS_COL_BLACK				0x0000
#define TS_COL_WHITE				0xFFFF

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
#define TS_LCD_DATA_PORT_ENABLE()     SysCtlPeripheralEnable(TS_LCD_DATA_PORT);
#define TS_LCD_CONTROL_PORT_ENABLE()  SysCtlPeripheralEnable(TS_LCD_CONTROL_PORT);
//
//
//
//
//
//how to read data from program memory, depend on each compiler
#define code(codemem)				codemem
#define code2(codemem)				codemem
#define code4(codemem)				codemem
#define code_char(var_name)			const char var_name
#define code_uchar(var_name)		const unsigned char var_name
//
#define TS_ORN_PORTRAIT
#ifdef TS_ORN_PORTRAIT
#define TS_SIZE_X					240
#define TS_SIZE_Y					320
#define TS_VAL_ENTRY_MOD			0x0030
#define TS_INS_GRAM_ADX				TS_INS_GRAM_HOR_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_VER_AD
#define TS_INS_START_ADX   			TS_INS_HOR_START_AD
#define TS_INS_END_ADX   			TS_INS_HOR_END_AD
#define TS_INS_START_ADY   			TS_INS_VER_START_AD
#define TS_INS_END_ADY   			TS_INS_VER_END_AD
#else
#define TS_SIZE_X					320
#define TS_SIZE_Y					240
#define TS_VAL_ENTRY_MOD			0x0028
#define TS_INS_GRAM_ADX				TS_INS_GRAM_VER_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_HOR_AD
#define TS_INS_START_ADX   			TS_INS_VER_START_AD
#define TS_INS_END_ADX   			TS_INS_VER_END_AD
#define TS_INS_START_ADY   			TS_INS_HOR_START_AD
#define TS_INS_END_ADY   			TS_INS_HOR_END_AD
#endif
//
//
typedef enum
{
	TS_MODE_NORMAL,
	TS_MODE_INVERSE,
	TS_MODE_FULL,
} ts_mode_t;
//font configuration
extern const unsigned char font_vrinda11x16[];
extern const unsigned char font_vrinda18x16[];
extern const unsigned char font_angsana14x32[];
extern const unsigned char font_angsana32x64[];
extern const unsigned char font_angsana_eng32x48[];
void TSLCDOutIns(unsigned short ins); //write instruction to LCD
void TSLCDOutDat(unsigned short dat); //write data to LCD
void TSLCDOutDat2(unsigned char dath,unsigned char datl); //write data to LCD
unsigned short TSLCDInIns(void); //read data from LCD
unsigned short TSLCDInDat(void); //read data from LCD

void TSLCDRst(void); //pulse reset signal to LCD
void TSLCDInit(void); //initial LCD
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
void delay_1ms(unsigned long int microsecond);
#endif

