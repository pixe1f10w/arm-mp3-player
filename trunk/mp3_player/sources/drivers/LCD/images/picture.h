#include "pic_volume1.h"
#include "pic_volume2.h"
#include "pic_track1.h"
#include "pic_track2.h"
#include "pic_button1.h"
#include "pic_button2.h"
#include "pic_back.h"
#include "pic_icon.h"
#include "pic_up.h"
#include "pic_down.h"
#include "pic_bin.h"
#include "pic_mute.h"
#include "pic_playall.h"
#include "pic_next.h"
#include "pic_bg_ques.h"
#include "pic_ok.h"
#include "pic_cancle.h"
///////////////////////////////////////////////////////////////
#define SO_VOLUME		0
#define SO_TRACK		1
#define SO_BT1			2 //play and pause
#define SO_BT2			3 //stop
#define SO_DIR_LIST		5 //directory list
#define SO_DIR_BACK		6 //go back to upper directory
#define SO_DIR_UP		7 
#define SO_DIR_DOWN		8
#define SO_IMAGE		9
#define SO_bin			10
#define SO_mute		 	4
#define SO_playall		11
#define SO_next		    12
#define SO_bg_ques      13
#define SO_ok    		14
#define SO_cancle    	15
////////////////////////////////////////////////////////////////////
#define DIR_FILE_NUM	5 //number of file shown
#define COL_LITE_BLUE	0x0000
#define COL_FONT		TS_COL_WHITE
#define COL_FONT_SPL	    0x07BF        //TS_COL_BLACK
#define COL_BACK		COL_LITE_BLUE
#define FILE_TYPE_UNKNOWN		0
#define FILE_TYPE_FOLDER		1
#define FILE_TYPE_MP3			2
#define FILE_TYPE_TXT			3
#define FILE_TYPE_BIN			4
