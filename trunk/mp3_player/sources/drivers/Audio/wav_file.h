#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_
#include "third_party/fatfs/src/ff.h"
#include "support_play_file.h"

unsigned short
WaveRead(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader, unsigned char *pucBuffer);
FRESULT OpenWavFile(FIL *g_sFileObject, char *g_pcFilenames, SoundInfoHeader *pSoundInfoHeader);
#endif //_WAV_FILE_H_