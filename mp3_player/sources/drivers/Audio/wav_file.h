#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_
#include "third_party/fatfs/src/ff.h"
//
// Flags used in the g_ulFlags global variable.
//
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004
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
    // The average byte rate for the wav file.
    //
    unsigned long ulAvgByteRate;

    //
    // The size of the wav data in the file.
    //
    unsigned long ulDataSize;

    //
    // The number of bits per sample.
    //
    unsigned short usBitsPerSample;

    //
    // The wav file format.
    //
    unsigned short usFormat;

    //
    // The number of audio channels.
    //
    unsigned short usNumChannels;
}
tWaveHeader;
FRESULT
WaveOpen(FIL *psFileObject, const char *pcFileName, tWaveHeader *pWaveHeader);
unsigned long
WavePlay(FIL *psFileObject, tWaveHeader *pWaveHeader);
int
init_wav_play(void);
unsigned long
UpdateBufferForPlay(FIL *psFileObject, tWaveHeader *pWaveHeader);
char wait_buffer_signal(portTickType timeout);
void set_play_flags(unsigned long a);
unsigned long get_play_flags(void);
#endif //_WAV_FILE_H_