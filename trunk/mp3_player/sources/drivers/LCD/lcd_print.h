#ifndef	_TSLCD_PRINT_H_
#define _TSLCD_PRINT_H_

#define NOWPLAY     0
#define BROWSE      1
#define SETTINGS    2
#define INFO        3

#define     MENU_INIT_BACK      0x01
#define     MENU_SELECT_UPDATE  0x02
void DisplayMenu(unsigned char iii);

#define     INITIAL_BACK    0x01
#define     VOLUME_UPDATE   0x02
#define     SELECTED_UPDATE 0x04
#define     TIME_UPDATE     0x08
#define     STATE_UPDATE    0x10
extern unsigned char VolumeValue;
#define     PLAY_STATE      0
#define     PAUSE_STATE     1
extern unsigned char PlayState;
extern unsigned long  MinsPlay;
extern unsigned long  SecondPlay;
extern unsigned long  MinsCurrent;
extern unsigned long  SecondCurrent;
void NowPlay(unsigned char iii);
//**********************
#define     BROWSE_INITIAL_BACK    0x01
#define     BROWSE_UPDATE           0x02
void Browse(unsigned char iii);
#define     SETTING_INIT_BACK   0x01
#define     SETTING_MODE        0x02
#define     PLAY_MODE       0
#define     BUTTON_SOUND    1
extern unsigned char PlayMode;
extern unsigned char SoundButton;
extern unsigned char Selected;
void Settings(unsigned char iii);
//************************
void Info(unsigned char iii);
void initLCD(void);
#endif //_LCD_PRINT_H_