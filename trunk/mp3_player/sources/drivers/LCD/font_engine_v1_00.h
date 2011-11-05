#ifndef _FONT_ENGINE_H_
#define _FONT_ENGINE_H_

void buf_store(unsigned char charactor);
void buf_clear(void);
unsigned char buf_read(unsigned char column,unsigned char row);

unsigned int TSLCDGetCharWidth(char c); //get printed width of character
unsigned int TSLCDGetStringWidth(char *str); //get printed width of string
void TSLCDSetBold(unsigned char on); //set bold character mode
unsigned char TSLCDGetBold(void); //get bold character mode
void TSLCDSetVaryWidth(unsigned char on); //enable variable character width
unsigned char TSLCDGetVaryWidth(void); //get variable character width enabling
void TSLCDCfgFont(const unsigned char *_font, unsigned char width, unsigned char height_div_8,unsigned char gap); //set font, font size

#endif

