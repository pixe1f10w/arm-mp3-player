/*
*
*/
#include "audio_play.h"
#include "mp3_file.h"
//#include "audio.h"
FRESULT OpenMp3File(FIL *g_sFileObject, char *g_pcFilenames, SoundInfoHeader *pSoundInfoHeader)
{
  //FRESULT Result;
  //TODO Read the MP3 header
  //TODO Setup SoundHeader
  /*
  pSoundInfoHeader->usFormat=;
  pSoundInfoHeader->usNumChannels =;
  pSoundInfoHeader->ulSampleRate = ;
  pSoundInfoHeader->ulAvgByteRate =;
  pSoundInfoHeader->usBitsPerSample =;
  //
  // Save the size of the data.
  //
  pSoundInfoHeader->ulDataSize = ;
  */

  //
  // Only mono and stereo supported.
  //
  if(pSoundInfoHeader->usNumChannels > 2)
  {
      f_close(g_sFileObject);
      return(FR_INVALID_NAME);
  }
  //
    // Adjust the average bit rate for 8 bit mono files.
    //
    if((pSoundInfoHeader->usNumChannels == 1) && (pSoundInfoHeader->usBitsPerSample == 8))
    {
        pSoundInfoHeader->ulAvgByteRate <<=1;
    }
  return(FR_OK);
}
unsigned short
DecodedMP3Read(FIL *psFileObject,SoundInfoHeader *pSoundInfoHeader, unsigned char *pucBuffer){
  unsigned short usCount =0;//the number of bytes read from MP3 file
  //TODO read MP3 file
  ;
  //TODO Decode MP3 file
  ;
  //TODO Write to pucBuffer with the number of bytes < g_ulMaxBufferSize
  ;
  //
  // Decrement the number of data bytes remaining to be read.
  //
  g_ulBytesRemaining -= usCount;
  //
  // Need to convert the audio from unsigned to signed if 8 bit
  // audio is used.
  //
  if(pSoundInfoHeader->usBitsPerSample == 8)
  {
     Convert8Bit(pucBuffer, usCount);
  }
  return usCount;
}