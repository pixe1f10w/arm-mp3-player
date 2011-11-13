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
const unsigned char *font_th;
unsigned char tone_shift;
unsigned char get_char_width_ignore_upper_lower_mode = 0;

void buf_or(unsigned char charactor, unsigned char new_left, unsigned char new_right, unsigned char old_left, unsigned char up_shift);

extern const unsigned char *font;
extern unsigned char char_buf[CHAR_BUF_WIDTH][CHAR_BUF_BYTE_HEIGHT];

extern unsigned char font_ext_bit_width;
extern unsigned char font_ext_byte_height;
extern int font_ext_size;
extern unsigned char font_ext_width;
extern unsigned char font_ext_height;

extern unsigned char char_gap;
extern unsigned char font_ext_adj;
extern unsigned char bold_on;
extern unsigned char first_non_zero;
extern unsigned char last_non_zero;
extern unsigned char vary_width_on;

font_thai_t TSLCDGetTypeExtFont(char c)
{
	if (check_m_n(c,0xA1,0xD0))
		return (NORMAL);
	if (c == 0xD1)
		return (UPPER);
	if (c == 0xD2)
		return (NORMAL);
	if (c == 0xD3)
		return (SPECIAL);
	if (check_m_n(c,0xD4,0xD7))
		return (UPPER);
	if (check_m_n(c,0xD8,0xDA))
		return (LOWER);
	if (check_m_n(c,0xDF,0xE6))
		return (NORMAL);
	if (check_m_n(c,0xE7,0xED))
		return (UPPER);
	if (check_m_n(c,0xF0,0xF9))
		return (NORMAL);
	return (OUT);
}

void TSLCDCfgExtFont(const unsigned char *_font, unsigned char width, unsigned char height_div_8, unsigned char _tone_shift, unsigned char _font_ext_adj)
{
	font_th = _font;
	font_ext_bit_width = width;
	font_ext_byte_height = height_div_8;
	font_ext_size = width*height_div_8;
	font_ext_width = font_ext_bit_width;
	font_ext_height = font_ext_byte_height*8;
	tone_shift = _tone_shift;
	font_ext_adj = _font_ext_adj;
}

void TSLCDSetGetCharWidthMode(unsigned char ignore_upper_lower_mode)
{
	get_char_width_ignore_upper_lower_mode = ignore_upper_lower_mode;
}

unsigned char TSLCDGetGetCharWidthMode(void)
{
	return (get_char_width_ignore_upper_lower_mode);
}

unsigned char TSLCDExtFontCheckWidth(unsigned char character)
{
	unsigned char i,j;
	font_thai_t x;
	int char_p = character*font_ext_size;

	first_non_zero = font_ext_bit_width;
	last_non_zero = 0;

	if (get_char_width_ignore_upper_lower_mode)
	{
		x = TSLCDGetTypeExtFont(character+0xA0);
		if ((x == UPPER) || (x == LOWER))
			return (0);
		else
		if (x == SPECIAL)
			character = 0xD2-0xA0; //sara-ar
	}

	for (i=0; i<font_ext_bit_width; i++)
		for (j=0; j<font_ext_byte_height; j++)
		{
			if (code(font_th[char_p]))
			{
				last_non_zero = i;
				if (first_non_zero == font_ext_bit_width)
				{
					first_non_zero = i;
				}
			}
			char_p++;
		}

	if (bold_on)
	{
		last_non_zero++;
	}

	return (last_non_zero-first_non_zero+1);
}

void buf_or(unsigned char charactor, unsigned char new_left, unsigned char new_right, unsigned char old_left, unsigned char up_shift)
{
	unsigned char i,j;
	//unsigned int buf;
	long long buf;
	int char_p = charactor*font_ext_size+new_left*font_ext_byte_height;

	for (i=0; i<new_right-new_left+1; i++)
	{
		buf = 0;
		for (j=0; j<font_ext_byte_height; j++)
		{
			buf = buf << 8;
			if (bold_on && i)
			{
				buf |= ((code(font_th[char_p]) | code(font_th[char_p-font_ext_byte_height])));
			}
			else
			{
				buf |= code(font_th[char_p]);
			}
			char_p++;
		}

		buf = buf << up_shift;

		for (j=0; j<font_ext_byte_height; j++)
		{
			char_buf[i+old_left][font_ext_byte_height-j-1] |= buf;
			buf = buf >> 8;
		}
	}
}

unsigned char TSLCDStoreExtFont(char *c)
{
	font_thai_t type;
	char ch;
	unsigned char width,new_width;
	unsigned char left_shift = 0;
	unsigned char first_upper = 0;
	unsigned char buf;

	buf = get_char_width_ignore_upper_lower_mode;
	get_char_width_ignore_upper_lower_mode = 0;

	type = TSLCDGetTypeExtFont(c[0]);
	ch = c[0];
	if ((type != NORMAL) && (type != SPECIAL))
	{
		get_char_width_ignore_upper_lower_mode = buf;
		return(0);
	}

	buf_clear();
	if (type == SPECIAL)
	{
		ch = 0xD2; //sara-ar
		type = OUT;
	}
	else
	{
		type = TSLCDGetTypeExtFont(c[1]);
	}

	if ((ch == 0xBB) || (ch == 0xBD) || (ch == 0xBF)) //NORMAL with high tail -- por-plar,phor-phar,phor-phun
	{
		if (type == UPPER)
			left_shift = 2;
	}
	if ((c[0] == 0xAD) && (type == LOWER)) //yor-ying with sara-oo
		ch = 0xFF;
	else
	if ((c[0] == 0xB0) && (type == LOWER)) //thor-tharn with sara-oo
		ch = 0xFE;
	width = TSLCDExtFontCheckWidth(ch-0xA0);
	buf_or(ch-0xA0,first_non_zero,last_non_zero,font_ext_width-width,0); //right justify

	if ((type == UPPER) || (type == LOWER) || (type == SPECIAL))
	{
		first_upper = (type == UPPER);
		if (type == SPECIAL)
		{
			new_width = TSLCDExtFontCheckWidth(0xED-0xA0);
			width = (new_width+left_shift > width)? new_width+left_shift : width;
			buf_or(0xED-0xA0,first_non_zero,last_non_zero,font_ext_width-new_width-left_shift,0); //right justify
		}
		else
		{
			type = TSLCDGetTypeExtFont(c[2]);
			if (type == SPECIAL)
			{
				new_width = TSLCDExtFontCheckWidth(0xED-0xA0);
				width = (new_width+left_shift > width)? new_width+left_shift : width;
				buf_or(0xED-0xA0,first_non_zero,last_non_zero,font_ext_width-new_width-left_shift,0); //right justify
				first_upper = 1;
				new_width = TSLCDExtFontCheckWidth(c[1]-0xA0);
				width = (new_width+left_shift > width)? new_width+left_shift : width;
				buf_or(c[1]-0xA0,first_non_zero,last_non_zero,font_ext_width-new_width-left_shift,tone_shift); //right justify
			}
			else
			{
				new_width = TSLCDExtFontCheckWidth(c[1]-0xA0);
				width = (new_width+left_shift > width)? new_width+left_shift : width;
				buf_or(c[1]-0xA0,first_non_zero,last_non_zero,font_ext_width-new_width-left_shift,0); //right justify
			}

			if (type == UPPER)
			{
				new_width = TSLCDExtFontCheckWidth(c[2]-0xA0);
				width = (new_width+left_shift > width)? new_width+left_shift : width;
				buf_or(c[2]-0xA0,first_non_zero,last_non_zero,font_ext_width-new_width-left_shift,first_upper*tone_shift); //right justify
			}
		}
	}
	first_non_zero = font_ext_width-width;//_first_non_zero;
	last_non_zero = font_ext_width-1;//_last_non_zero;

	get_char_width_ignore_upper_lower_mode = buf;
	return(1);
}

