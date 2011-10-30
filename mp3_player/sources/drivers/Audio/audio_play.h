#ifndef _AUDIO_PLAY_H_
#define _AUDIO_PLAY_H_
#include "third_party/fatfs/src/ff.h"
#include "SafeRTOS/SafeRTOS_API.h"
//Define for storage device
#define SD_CARD     0
#define USB_STORAGE 1
//Define file format
#define WAV_FILE    		0
#define MP3_FILE    		1
#define NOT_SUPPORT_FORMAT 	2
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
} SoundInfoHeader;
//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004
//
extern unsigned long g_ulMaxBufferSize;
extern unsigned long g_ulBytesRemaining;
extern unsigned short g_usMinutes;
extern unsigned short g_usSeconds;
extern volatile unsigned long g_ulFlags;
void
Convert8Bit(unsigned char *pucBuffer, unsigned long ulSize);
char CheckExtension(char *fileName);
unsigned long
UpdateBufferForPlay(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader,char format);
char WaitBufferSignal(portTickType timeout);
char StartBufferSignal(portTickType timeout);
int
initAudioCodec(unsigned long volume);
void setupAudioCodecForSong(SoundInfoHeader pSoundInfoHeader);
void playButtonSound(SoundInfoHeader SongInfoHeader);
#endif //_AUDIO_PLAY_H_