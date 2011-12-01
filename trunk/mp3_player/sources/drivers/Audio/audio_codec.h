#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_
#include "third_party/fatfs/src/ff.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "file_management.h"

extern unsigned char VolumeValue;
void VolumeUp(void);
void VolumeDown(void);
//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004

extern volatile unsigned long ulFlags;
extern unsigned long   ulMaxBufferSize;
char WaitBufferSignal(portTickType timeout);
char StartBufferSignal(portTickType timeout);
int initAudioCodec(unsigned long volume);
void initBuffer(unsigned long   ulBufferSize);
void setupAudioCodecForSong(SongInfoHeader *pSongInfoHeader);
unsigned short WriteWAVToBuffer(FIL* pSongFileObject,unsigned char *pData, unsigned short  nBytes, \
                                unsigned char usForcePlay, unsigned char Convert8Bit);
unsigned short WriteMP3ToBuffer(short int *pData, 
                                unsigned short  nsCount);
#endif //_AUDIO_CODEC_H_