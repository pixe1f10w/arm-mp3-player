//extension font plugin
//limitation for V1.00 in function buf_or, limits the height of font to 32 pixels 
//version 1.01 -- breaks limitation above, supports upto 64 pixels
//version 1.02 -- add code() for campatible with AVR (see app_config.h)

#ifndef _FONT_THAI_PLUGIN_H_
#define	_FONT_THAI_PLUGIN_H_

#define FONT_EXT_PLUGIN

typedef enum
{
	NORMAL = 0,
	UPPER,
	LOWER,
	SPECIAL,
	OUT
} font_thai_t;

font_thai_t TSLCDGetTypeExtFont(char c);
unsigned char TSLCDStoreExtFont(char *c);
void TSLCDCfgExtFont(const unsigned char *_font, unsigned char width, unsigned char height_div_8, unsigned char _tone_shift, unsigned char _font_ext_adj);
unsigned char TSLCDExtFontCheckWidth(unsigned char character);
void TSLCDSetGetCharWidthMode(unsigned char ignore_upper_lower_mode);
unsigned char TSLCDGetGetCharWidthMode(void);

#endif

