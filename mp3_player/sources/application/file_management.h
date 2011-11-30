#ifndef _FILE_MANAGEMENT_H_
#define _FILE_MANAGEMENT_H_
#include "third_party/fatfs/src/ff.h"
//Store Filenames for Play
#define MAX_FILENAME_STRING_LEN (8 + 1 + 3 + 1)
#define NUM_LIST_STRINGS 50
typedef struct ITEM{
  unsigned char fattrib;
  char fname[MAX_FILENAME_STRING_LEN];
}Item;
/**
*The wav file header information.
*
*/
typedef struct
{
    //
    // Sample rate in bytes per second.
    //
    unsigned long ulSampleRate;

    //
    // The average byte rate for the sound file.
    //
    unsigned long ulAvgByteRate;

    //
    // The size of the data in the file.
    //
    unsigned long ulDataSize;

    //
    // The number of bits per sample.
    //
    unsigned short usBitsPerSample;

    //
    // The sound file format.
    //
    unsigned short usFormat;

    //
    // The number of audio channels.
    //
    unsigned short usNumChannels;
    unsigned long  ulMins;
    unsigned long  ulSecond;
} SongInfoHeader;
//
extern FIL sSongFileObject;
extern SongInfoHeader sSongInfoHeader;
extern char *sFilePath;
extern unsigned long  MinsCurrent;
extern unsigned long  SecondCurrent;
//**********Files List***********
extern char pcFiles[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];;
//the number of files
extern unsigned char ucFileCount;
//index of the file
extern unsigned char ucFileIndex;
//*****Store for browsing********
extern Item sItemList[NUM_LIST_STRINGS];
extern unsigned char ucItemCount;
extern unsigned char ucItemIndex;
//*******************************
//Define file format
#define WAV_FILE    		0
#define MP3_FILE    		1
#define NOT_SUPPORT_FORMAT 	2
char CheckExtension(char *fileName);
char * preFile(void);
char * nextFile(void);
int ListDirItems(void);
int ListFiles(void);
char * getCurrentFile(void);
int BackParentDir(void);
void OpenChooseDir(void);
void NextItem(void);
void PreItem(void);
char InitFileManagement(void);
#endif //_FILE_MANAGEMENT_H_
