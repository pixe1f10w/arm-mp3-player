//version 1.02	--int_to_str function added
#ifndef _UTILS_CUSTOM_H_
#define _UTILS_CUSTOM_H_

char hextable(unsigned char hex);
char to_upper(char c);
unsigned char check_m_n(char c, char m, char n);
unsigned char to_bcd(char c0, char c1);
void to_num(unsigned char hex, unsigned char *num_buf);
unsigned char int_to_str(unsigned short val,unsigned char *str);

#endif
