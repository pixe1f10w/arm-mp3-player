#ifndef _LCD_PAGES_H_
#define _LCD_PAGES_H_
typedef struct file{
  unsigned char *ucfilename;
  unsigned char *ucsinger;
  unsigned char *ucauthor;
  unsigned long ultime;
}  playing_file;
typedef struct folder{
  unsigned char *ucfoldername;
  unsigned char *ucfile1;
  unsigned char *ucfile2;
  //
  //.......
  //
}  file_tree;

unsigned char LCD_Init_Page(void);
unsigned char LCD_Starting_Page(void);
unsigned char LCD_Welcome_Page(void);
unsigned char LCD_Setting_Page(void);
unsigned char LCD_Volume_Setting_Page(unsigned char percent);
unsigned char LCD_Playing_Page(playing_file*  input);
unsigned char LCD_Browse_Page(file_tree*  input);
#endif
