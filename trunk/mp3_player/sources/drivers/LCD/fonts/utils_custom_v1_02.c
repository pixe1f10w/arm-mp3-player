//#include "app_config.h"

char hextable(unsigned char hex)
{
	if (hex < 10)
		return (hex + 0x30);
	else
		return (hex - 10 + 'A');
}

char to_upper(char c)
{
	if ((c >= 'a')&&(c <= 'z'))
		return (c - 0x20);
	else
		return (c);
}

unsigned char check_m_n(char c, char m, char n)
{
	if ((c >= m)&&(c <= n))
		return (1);
	else
		return (0);
}

unsigned char to_bcd(char c0, char c1)
{
	c0 = c0 - 0x30;
	c1 = c1 - 0x30;
	c0 = c0 << 4;
	c0 |= c1;
	return (c0);
}

void to_num(unsigned char hex, unsigned char *num_buf)
{
	num_buf[0] = hex / 100;
	hex = hex % 100;
	num_buf[1] = hex / 10;
	hex = hex % 10;
	num_buf[2] = hex;
}

unsigned char int_to_str(unsigned short val,unsigned char *str)
{
	unsigned char i = 0,j = 0;
	unsigned short div = 10000;
	while(div > 1)
	{
		if ((val >= div) || i)
		{
			str[i] = val/div;
			val -= str[i]*div;
			i++;
		}
		div = div/10;
	}
	str[i] = val;
	i++;
	str[i] = 0;
	for (j=0;j<i;j++)
		str[j] += '0';
	return (i);
}
