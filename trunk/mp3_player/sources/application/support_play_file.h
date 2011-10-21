#ifndef _SUPPORT_PLAY_FILE_H_
#define _SUPPORT_PLAY_FILE_H_
#include "third_party/fatfs/src/ff.h"
#include "SafeRTOS/SafeRTOS_API.h"
//Define for storage device
#define SD_CARD     0
#define USB_STORAGE 1
//Define file format
#define WAV_FILE    0
#define MP3_FILE    1
//******************************************************************************
//
// The wav file header information.
//
//******************************************************************************
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
} SoundInfoHeader;

//******************************************************************************
//
// Buffer management and flags.
//
//******************************************************************************
#define AUDIO_BUFFER_SIZE       4096
extern unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
extern unsigned long g_ulMaxBufferSize;
//******************************************************************************
//
// State information for keep track of time.
//
//******************************************************************************
extern unsigned long g_ulBytesPlayed;
//
// Globals used to track playback position.
//
extern unsigned long g_ulBytesRemaining;
extern unsigned short g_usMinutes;
extern unsigned short g_usSeconds;
//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004
extern volatile unsigned long g_ulFlags;
#define NUM_LIST_STRINGS 48
#define MAX_FILENAME_STRING_LEN (8 + 1 + 3 + 1)
extern char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
extern char  maxItemCount;
void
BufferCallback(void *pvBuffer, unsigned long ulEvent);
int
init_play_sound(void);
char CheckExtension(char *fileName);

char WaitBufferSignal(portTickType timeout);
void 
CloseFile(FIL *psFileObject);
void set_play_flags(unsigned long a);
unsigned long
UpdateBufferForPlay(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader,char format);
int
PopulateFileListBox(void);
void
Convert8Bit(unsigned char *pucBuffer, unsigned long ulSize);
#endif //_SUPPORT_PLAY_FILE_H_