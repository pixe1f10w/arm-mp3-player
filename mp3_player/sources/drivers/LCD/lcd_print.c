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
#include "utils.h"
#include "file_management.h"
#include "audio_codec.h"
#include "lcd_print.h"


extern unsigned char ChooseItem;
#define ITEM_NUM    4
const char MenuItems[ITEM_NUM][9]={"NowPlay","Browse","Settings","Info"};

void DisplayMenu(unsigned char iii)
{
    unsigned char i;
    if(iii&MENU_INIT_BACK)
    {
        //Fill background
        TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
        //Fill title
        TSLCDFillRect(0,30,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
        TSLCDSetFontColor(TS_COL_WHITE);
        TSLCDSetBackColor(TS_COL_BLUE);
        TSLCDPrint((320-TSLCDGetStringWidth("MP3 PLAYER"))>>1,215,"MP3 PLAYER",TS_MODE_FULL);
    }
    if(iii&MENU_SELECT_UPDATE)
    {
        //Nowplay
        TSLCDSetBackColor(TS_COL_BLACK);
        for(i=0; i < ITEM_NUM; i++)
        {
            if(i == ChooseItem)
                TSLCDSetFontColor(TS_COL_RED);
            else
                TSLCDSetFontColor(TS_COL_WHITE);
                
            TSLCDPrint((320-TSLCDGetStringWidth((char*)MenuItems[i]))>>1,
                    160-(i*30),MenuItems[i],TS_MODE_FULL);
        }
    }
}
static char VolumeStr[4]="100";
static char FileStateStr[]="00/00";
#define  DISPLAY_FILE_MAX    8
//static char * Files[32];
//static unsigned char FileNum;
//static unsigned char SelectedFile;
static char PlayTime[]="00:00/00:00";

const char PauseStr[]="|| ";
const char PlayStr[]=">";
unsigned char PlayState;
static char *PlayStateStr; //">" or "||"
static char PlayedPercent=0;

void NowPlay(unsigned char iii)
{
    unsigned char i,tmp;
    if(iii&INITIAL_BACK)//Intialize background
    {
        //Fill background
        TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
        //Fill title
        TSLCDFillRect(0,30,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
        TSLCDSetFontColor(TS_COL_WHITE);
        TSLCDSetBackColor(TS_COL_BLUE);
        TSLCDPrint((320-TSLCDGetStringWidth("NOWPLAY"))>>1,215,"NOWPLAY",TS_MODE_FULL);
        //Time line
        TSLCDFillRect(228,232,20,319,TS_COL_WHITE,TS_MODE_NORMAL);
    }
    if(iii&VOLUME_UPDATE)
    {
        //Update volume
        TSLCDSetFontColor(TS_COL_WHITE);
        TSLCDSetBackColor(TS_COL_BLUE);
        NumToString(VolumeValue,VolumeStr,3);
        TSLCDPrint(0,215,VolumeStr,TS_MODE_FULL);
    }
    if(iii&SELECTED_UPDATE)
    {
        TSLCDSetBackColor(TS_COL_BLUE);
        TSLCDSetFontColor(TS_COL_WHITE);
        //Total songs in List
        NumToString(ucFileCount,&FileStateStr[3],2);
        //The order of the song in List
        NumToString(ucFileIndex+1,FileStateStr,2);
        TSLCDPrint((320-TSLCDGetStringWidth(FileStateStr)),215,FileStateStr,TS_MODE_FULL);
        //List Files
        TSLCDSetBackColor(TS_COL_BLACK);
        if(ucFileIndex >= DISPLAY_FILE_MAX)
            tmp =(ucFileIndex - (DISPLAY_FILE_MAX-1));
        else
            tmp =0;
        
        for(i=tmp;  (i-tmp) < DISPLAY_FILE_MAX; i++)
        {
            if(i == ucFileIndex)
                TSLCDSetFontColor(TS_COL_RED);
            else
                TSLCDSetFontColor(TS_COL_WHITE);
            if(i< ucFileCount)
              TSLCDFixedPrint(20,190-((i-(tmp))*18),240,pcFiles[i],TS_MODE_FULL);
            else
              TSLCDFixedPrint(20,190-((i-(tmp))*18),240," ",TS_MODE_FULL);
        }
    }
    if(iii&TIME_UPDATE)
    {
        TSLCDSetBackColor(TS_COL_BLACK);
        TSLCDSetFontColor(TS_COL_WHITE);
        //Play Time
        NumToString(MinsCurrent,&PlayTime[0],2);
        NumToString(SecondCurrent,&PlayTime[3],2);
        NumToString(sSongInfoHeader.ulMins,&PlayTime[6],2);
        NumToString(sSongInfoHeader.ulSecond,&PlayTime[9],2);
        TSLCDPrint((320-TSLCDGetStringWidth(PlayTime)>>1),20,PlayTime,TS_MODE_FULL);
        //Time line
        PlayedPercent = ((MinsCurrent*60+SecondCurrent)*100)/(sSongInfoHeader.ulMins*60+sSongInfoHeader.ulSecond);
        TSLCDFillRect(228,232,20,319,TS_COL_WHITE,TS_MODE_NORMAL);
        TSLCDFillRect(228,232,20,20+PlayedPercent*3,TS_COL_RED,TS_MODE_NORMAL);
    }
    if(iii&STATE_UPDATE)
    {
        TSLCDSetBackColor(TS_COL_BLACK);
        TSLCDSetFontColor(TS_COL_WHITE);
        //Display State
        if(PlayState == PLAY_STATE)
            PlayStateStr=(char*)PlayStr;
        else if(PlayState == PAUSE_STATE)
            PlayStateStr=(char*)PauseStr;
        TSLCDPrint(5,2,PlayStateStr,TS_MODE_FULL);
    }
}
#define     DISPLAY_ITEM_MAX        8

//static char    *Items[32];

void Browse(unsigned char iii)
{
    unsigned char i,tmp;
    if(iii&BROWSE_INITIAL_BACK)
    {
        //Fill background
        TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
        //Fill title
        TSLCDFillRect(0,30,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
        TSLCDSetFontColor(TS_COL_WHITE);
        TSLCDSetBackColor(TS_COL_BLUE);
        TSLCDPrint((320-TSLCDGetStringWidth("BROWSE"))>>1,215,"BROWSE",TS_MODE_FULL);
    }
    if(iii&BROWSE_UPDATE)
    {
        if(ucItemCount ==0)
            return;
        TSLCDSetBackColor(TS_COL_BLACK);
        TSLCDSetFontColor(TS_COL_WHITE);
        if(ucItemIndex >= DISPLAY_ITEM_MAX)
            tmp =(ucItemIndex - (DISPLAY_ITEM_MAX-1));
        else
            tmp =0;
        for(i=tmp; i< ucItemCount && i-tmp < DISPLAY_ITEM_MAX; i++)
        {
            if(i == ucItemIndex)
                TSLCDSetFontColor(TS_COL_RED);
            else
                TSLCDSetFontColor(TS_COL_WHITE);
            if(i< ucItemCount)
              TSLCDFixedPrint(20,190-((i-(tmp))*18),240,sItemList[i].fname,TS_MODE_FULL);
            else
              TSLCDFixedPrint(20,190-((i-(tmp))*18),240," ",TS_MODE_FULL);
            
        }
    }
}
const char SettingItems[2][13]={"Play mode","Button sound"};
const char ModesStr[4][8]={"Normal","Single ","Repeat","Loop  "};
const char ButtonSoundChooses[2][4]={"Off","On"};
unsigned char PlayMode;
unsigned char SoundButton;
#define     PLAY_MODE       0
#define     BUTTON_SOUND    1
unsigned char Selected;
void Settings(unsigned char iii)
{
    unsigned char i;
    if(iii&SETTING_INIT_BACK)
    {
        //Fill background
        TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
        //Fill title
        TSLCDFillRect(0,30,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
        TSLCDSetFontColor(TS_COL_WHITE);
        TSLCDSetBackColor(TS_COL_BLUE);
        TSLCDPrint((320-TSLCDGetStringWidth("SETTINGS"))>>1,215,"SETTINGS",TS_MODE_FULL);
        Selected = PLAY_MODE;
    }
    if(iii&SETTING_MODE)
    {
        TSLCDSetBackColor(TS_COL_BLACK);
        for(i=0;i<2;i++)
        {
            if(i==Selected)
                TSLCDSetFontColor(TS_COL_RED);
            else
                TSLCDSetFontColor(TS_COL_WHITE);
            //
            TSLCDPrint(50,160-i*30,SettingItems[i],TS_MODE_FULL);
            if(i == PLAY_MODE)
            {
                TSLCDPrint(220,160-i*30,ModesStr[PlayMode],TS_MODE_FULL);
            }
            else if(i == BUTTON_SOUND)
            {
                TSLCDPrint(220,160-i*30,ButtonSoundChooses[SoundButton],TS_MODE_FULL);
            }
        }
    }
    
}
void Info(unsigned char iii)
{
    //Fill background
    TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
    //Fill title
    TSLCDFillRect(0,30,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
    TSLCDSetFontColor(TS_COL_WHITE);
    TSLCDSetBackColor(TS_COL_BLUE);
    TSLCDPrint((320-TSLCDGetStringWidth("INFO"))>>1,215,"INFO",TS_MODE_FULL);
    //;
    TSLCDSetFontColor(TS_COL_RED);
    TSLCDSetBackColor(TS_COL_BLACK);
    TSLCDPrint((320-TSLCDGetStringWidth("PANDORA Player"))>>1,130,"PANDORA Player",TS_MODE_FULL);
    TSLCDPrint((320-TSLCDGetStringWidth("Version 1.0"))>>1,110,"Version 1.0",TS_MODE_FULL);
    TSLCDPrint((320-TSLCDGetStringWidth("Copyright@2011"))>>1,90,"Copyright@2011",TS_MODE_FULL);
    
}
void USBTransfer(void)
{
  //Fill background
  TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
  //Fill title
  TSLCDFillRect(100,140,0,319,TS_COL_BLUE,TS_MODE_NORMAL);
  TSLCDSetFontColor(TS_COL_WHITE);
  TSLCDSetBackColor(TS_COL_BLUE);
  TSLCDPrint((320-TSLCDGetStringWidth("USB MASS STORAGE"))>>1,110,"USB MASS STORAGE",TS_MODE_FULL);
}
void initLCD(void)
{
//
    // Initialize the device pinout appropriately for this board.
    //
    TS_LCD_DATA_PORT_ENABLE() ;   
    TS_LCD_CONTROL_PORT_ENABLE() ;
    TS_LCD_CONTROL_PORT_OUT();
    TS_LCD_BL_SET();
    TS_LCD_BL_CLR();
    TSLCDInit();
    delay_1ms(1000);
    //
    //TSLCDFillRect(0,239,0,319,TS_COL_BLACK,TS_MODE_NORMAL);
    TSLCDSetFontColor(TS_COL_WHITE);
    TSLCDSetBackColor(TS_COL_BLACK);
    TSLCDCfgFont(font_vrinda18x16,18,2,1);
    //Menu
    ChooseItem=NOWPLAY;
    //NowPlay
    ucFileCount=0;
    ucFileIndex=0;
    sSongInfoHeader.ulMins=0;
    sSongInfoHeader.ulSecond=0;
    MinsCurrent=0;
    SecondCurrent=0;
    PlayState=PAUSE_STATE;
    VolumeValue=60;
    //Browse
    ucItemIndex=0;
    ucItemCount=0;
    //Settingd
    PlayMode=0;
    SoundButton=0;
    Selected=PLAY_MODE;
}