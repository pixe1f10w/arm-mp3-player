#ifndef _CONF_H_
#define _CONF_H_
/**
* Define ButtonEvent
*/
#define	S_CENTER	0
#define L_CENTER	1
#define	S_UP    	2
#define	L_UP	        3
#define S_DOWN  	4
#define	L_DOWN  	5
#define	S_RIGHT 	6
#define	L_RIGHT 	7
#define S_LEFT  	8
#define	L_LEFT  	9
/**
* Define PlayEvent
*/
#define	PLAY_DONE  	10
#define OPEN_FAIL       23
#define FORMAT_ERR      22
/**
* Define SoundControlEvent
*/
#define START           11
#define	PLAY    	12
#define	PAUSE           13
#define PAUSE_PLAY      14
#define	STOP            15
#define	VLM_UP          16
#define	VLM_DOWN        17
#define	PLAY_BTN_SND    18
/**
* Define USBEvent
*/
#define	TRANS_DATA      19
#define STORAGE         20
#define DISCONNECT      21
#endif //_CONF_H_