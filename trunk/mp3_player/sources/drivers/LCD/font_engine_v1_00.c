//#include "app_config.h"
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
const unsigned char *font;
unsigned char font_bit_width = 9;
unsigned char font_byte_height = 2;
int font_size = 18;
unsigned char font_width = 9;
unsigned char font_height = 16;
unsigned char char_gap = 2;
#define FONT_EXT_PLUGIN
#ifdef FONT_EXT_PLUGIN
unsigned char font_ext_bit_width;
unsigned char font_ext_byte_height;
int font_ext_size;
unsigned char font_ext_width;
unsigned char font_ext_height;
#endif

#ifndef CHAR_BUF_WIDTH
#define CHAR_BUF_WIDTH			24
#endif
#ifndef CHAR_BUF_BYTE_HEIGHT
#define CHAR_BUF_BYTE_HEIGHT	4
#endif

unsigned char char_buf[CHAR_BUF_WIDTH][CHAR_BUF_BYTE_HEIGHT];

unsigned char bold_on = 0;
unsigned char first_non_zero;
unsigned char last_non_zero;
//unsigned char vary_width_on = 0;

void buf_store(unsigned char character)
{
	unsigned char i,j;
	int char_p = character*font_size;

	first_non_zero = font_bit_width;
	last_non_zero = 0;

	for (i=0; i<font_bit_width+1; i++) //+1 for bold supported
		for (j=0; j<font_byte_height; j++)
		{
			if (code(font[char_p]))
			{
				last_non_zero = i;
				if (first_non_zero == font_bit_width)
					first_non_zero = i;
			}
			if (bold_on && i)
				char_buf[i][j] = code(font[char_p]) | code(font[char_p-font_byte_height]);
			else
				char_buf[i][j] = code(font[char_p]);
			char_p++;
		}

	if (bold_on)
		last_non_zero++;

	if (character == ' '-0x20)
	{	  
		first_non_zero = 0;
		last_non_zero = font_bit_width/2;
	}
}

void buf_clear(void)
{
	unsigned char i,j;

	for (i=0; i<CHAR_BUF_WIDTH; i++)
		for (j=0; j<CHAR_BUF_BYTE_HEIGHT; j++)
		{
			char_buf[i][j] = 0;
		}
}

unsigned char buf_read(unsigned char column,unsigned char row)
{
	unsigned char read_pixel;
	unsigned char row_byte = row >> 3;
	row &= 7;

	column += first_non_zero;	
	read_pixel = (char_buf[column][row_byte] >> (7-row)) & 0x01;
	return (read_pixel);
}

unsigned int TSLCDGetCharWidth(char c)
{
	unsigned int res;
#ifdef FONT_EXT_PLUGIN
	if (TSLCDGetTypeExtFont(c) != OUT)
	{
		res = TSLCDExtFontCheckWidth(c-0xA0) + char_gap;
		return (res);
	}
	else
#endif
	{
		buf_store(c-0x20);
		res = last_non_zero - first_non_zero + 1 + char_gap;
		return (res);
	}
}

unsigned int TSLCDGetStringWidth(char *str)
{
	unsigned int i = 0;
	unsigned int res = 0;

	while(str[i])
	{
#ifdef FONT_EXT_PLUGIN
		if (TSLCDGetTypeExtFont(str[i]) != OUT)
		{
			if (TSLCDStoreExtFont(str+i))		   
				res += last_non_zero-first_non_zero+1+char_gap;
		}
		else
#endif
		{
			res += TSLCDGetCharWidth(str[i]);
		}
		i++;
	}
	return (res);
}

void TSLCDSetBold(unsigned char on)
{
	bold_on = on;
}

unsigned char TSLCDGetBold(void)
{
	return (bold_on);
}

void TSLCDSetVaryWidth(unsigned char on)
{
	//vary_width_on = on;
}

unsigned char TSLCDGetVaryWidth(void)
{
	//return (vary_width_on);
	return(1);
}

void TSLCDCfgFont(const unsigned char *_font, unsigned char width, unsigned char height_div_8, unsigned char gap)
{
	font = _font;
	font_bit_width = width;
	font_byte_height = height_div_8;
	font_size = width*height_div_8;
	font_width = font_bit_width;
	font_height = font_byte_height*8;
	char_gap = gap;
	buf_clear();
}

