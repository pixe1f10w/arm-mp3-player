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
#include "images/background.h"
#include "images/mp3.h"
#include "images/wav.h"
#include "images/play.h"
#include "images/stop.h"
#include "images/next.h"
#include "images/speaker_on.h"
#include "images/speaker_off.h"
#include "images/logo.h"
#include "images/previous.h"
#include "images/folder.h"
//
//
//
#include "LCD_PAGES.H"
playing_file current_file;
file_tree   current_folder;

unsigned char LCD_Init_Page(void)
{
    TS_LCD_DATA_PORT_ENABLE() ;   
    TS_LCD_CONTROL_PORT_ENABLE() ;
    TS_LCD_CONTROL_PORT_OUT();
    TS_LCD_BL_SET();
    TS_LCD_BL_CLR();
    TSLCDInit();
    delay_1ms(1000);
    return 0;
}
unsigned char LCD_Starting_Page(void)
{
  //
  //note that  height first when show picture
  //
  TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
  //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(0,20,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDCfgFont(font_vrinda18x16,18,2,1);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("PANDORA MP3")<< 10),0);
  TSLCDPrintStr(19,5,"PANDORA MP3",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("SETTING")<< 10),0);
  TSLCDPrintStr(5,0,"SETTING",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BROWSE")<<10),0);
  TSLCDPrintStr(5,13,"BROWSE",TS_MODE_FULL);
  return (0);
}
unsigned char LCD_Welcome_Page(void)
{
    TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
    //TSLCDShowPic(0,111,0,319,logo,TS_MODE_NORMAL);
    TSLCDSetFontColor(TS_COL_BLUE);
    TSLCDSetBackColor(TS_COL_WHITE);
    TSLCDSetBold(1);
    TSLCDCfgFont(font_angsana_eng32x48,32,6,1);
    TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("PANDORA")),0);
    TSLCDPrintStr( 4,0,"PANDORA",TS_MODE_FULL);
    TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("MP3 PLAYER")),0);
    TSLCDPrintStr( 3,0," MP3 PLAYER",TS_MODE_FULL);
    delay_1ms(2000);
    TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL); 
    //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
    TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("WELCOME")),0);
    TSLCDPrintStr( 4,0,"WELCOME",TS_MODE_FULL);
    delay_1ms(1000);
   // TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
    return (0);
}
unsigned char LCD_Setting_Page(void)
{
  TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
  //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(0,20,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDCfgFont(font_vrinda18x16,18,2,1);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("SETTING")<< 10),0);
  TSLCDPrintStr(19,7,"SETTING",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BACK")<< 10),0);
  TSLCDPrintStr(5,0,"BACK",TS_MODE_FULL);
  //TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BROWSE")<<10),0);
  //TSLCDPrintStr(5,13,"BROWSE",TS_MODE_FULL);
  //
  // setting name here
  //
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("Volume")<< 10),0);
  TSLCDPrintStr(15,0,"Volume",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("....")<< 10),0);
  TSLCDPrintStr(12,0,"....",TS_MODE_FULL);

  return (0);
}
unsigned char LCD_Volume_Setting_Page(unsigned char percent)
{
  
  if( percent >100)
     percent =100;
  TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
  //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(0,20,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDCfgFont(font_vrinda18x16,18,2,1);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("VOLUME")<< 10),0);
  TSLCDPrintStr(19,7,"VOLUME",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BACK")<< 10),0);
  TSLCDPrintStr(5,0,"BACK",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("Percent")<< 10),0);
  TSLCDPrintStr(12,7,"Percent",TS_MODE_FULL);
  
  TSLCDFillRect(150,160,90,250,TS_COL_RED,TS_MODE_NORMAL);
  TSLCDFillRect(153,157,93,247,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(153,157,93,93 + (247-93)*percent /100,TS_COL_RED,TS_MODE_NORMAL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("Percent")<< 10),0);
  return (0);
}
unsigned char LCD_Playing_Page(playing_file*  input)
{
  TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
  //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(0,20,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDCfgFont(font_vrinda18x16,18,2,1);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("PLAYING")<< 10),0);
  TSLCDPrintStr(19,7,"PLAYING",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth((char*)input->ucfilename)<< 10),0);
  TSLCDPrintStr(16,5,(char*)input->ucfilename,TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth((char*)input->ucsinger)<< 10),0);
  TSLCDPrintStr(14,5,(char*)input->ucsinger,TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth((char*)input->ucauthor)<< 10),0);
  TSLCDPrintStr(12,5,(char*)input->ucauthor,TS_MODE_FULL);
  TSLCDFillRect(150,170,50,270,TS_COL_RED,TS_MODE_NORMAL);
  TSLCDFillRect(153,167,53,267,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(153,167,53,53 + (267-53)* input->ultime /100,TS_COL_RED,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BACK")<< 10),0);
  TSLCDPrintStr(5,0,"BACK",TS_MODE_FULL);
  //
  //
  //show pic
  //
  TSLCDShowPic(200,229,145,175,play,TS_MODE_NORMAL);
  TSLCDShowPic(200,229,176,205,next,TS_MODE_NORMAL);
  TSLCDShowPic(200,229,116,144,previous,TS_MODE_NORMAL);
  return (0);
}

unsigned char LCD_Browse_Page(file_tree*  input)
{
  TSLCDShowPic(0,239,0,319,background,TS_MODE_NORMAL);
  //TSLCDFillRect(0,TS_SIZE_X-1,0,TS_SIZE_Y-1,TS_COL_WHITE,TS_MODE_NORMAL);
  TSLCDFillRect(0,20,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDCfgFont(font_vrinda18x16,18,2,1);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BROWSE")<< 10),0);
  TSLCDPrintStr(19,7,"BROWSE",TS_MODE_FULL);
  TSLCDSetOffset((TS_SIZE_X-TSLCDGetStringWidth("BACK")<< 10),0);
  TSLCDPrintStr(5,0,"BACK",TS_MODE_FULL);
  
  //
  //
  //
  TSLCDShowPic(100,119,100,118,folder,TS_MODE_NORMAL);
  TSLCDShowPic(140,159,120,139,mp3,TS_MODE_NORMAL);
  TSLCDShowPic(180,199,120,139,wav,TS_MODE_NORMAL);
  //
  //
  
  return 0;
}