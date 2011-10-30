#ifndef _MP3_FILE_H_
#define _MP3_FILE_H_
#include "third_party/fatfs/src/ff.h"
#include "audio_play.h"
FRESULT OpenMp3File(FIL *g_sFileObject, char *g_pcFilenames, SoundInfoHeader *SoundHeader);
unsigned short
DecodedMP3Read(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader, unsigned char *pucBuffer);
#endif //_MP3_FILE_H_