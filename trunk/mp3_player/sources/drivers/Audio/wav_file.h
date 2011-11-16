#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_
#include "third_party/fatfs/src/ff.h"
#include "audio_play.h"

unsigned short
WaveRead(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader, unsigned char *pucBuffer);
FRESULT OpenWavFile(FIL *g_sFileObject, char *g_pcFilenames, SoundInfoHeader *pSoundInfoHeader,char flag);

#endif //_WAV_FILE_H_