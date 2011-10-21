/*
*
*/

#include "mp3_file.h"
FRESULT OpenMp3File(FIL *g_sFileObject, char *g_pcFilenames, SoundInfoHeader *pSoundInfoHeader)
{
  //FRESULT Result;
  unsigned long ulBytesPerSample;
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
  // Reset the byte count.
  //
  g_ulBytesPlayed = 0;
  //
  // Calculate the Maximum buffer size based on format.  There can only be
  // 1024 samples per ping pong buffer due to uDMA.
  //
  ulBytesPerSample = (pSoundInfoHeader->usBitsPerSample *
                        pSoundInfoHeader->usNumChannels) >> 3;

  if(((AUDIO_BUFFER_SIZE >> 1) / ulBytesPerSample) > 1024)
  {
      //
      // The maximum number of DMA transfers was more than 1024 so limit
      // it to 1024 transfers.
      //
      g_ulMaxBufferSize = 1024 * ulBytesPerSample;
  }
  else
  {
      //
      // The maximum number of DMA transfers was not more than 1024.
      //
      g_ulMaxBufferSize = AUDIO_BUFFER_SIZE >> 1;
  }

  //
  // Only mono and stereo supported.
  //
  if(pSoundInfoHeader->usNumChannels > 2)
  {
      f_close(g_sFileObject);
      return(FR_INVALID_NAME);
  }
    

  g_usSeconds = pSoundInfoHeader->ulDataSize/pSoundInfoHeader->ulAvgByteRate;
  g_usMinutes = g_usSeconds/60;
  g_usSeconds -= g_usMinutes*60;
  //
  // Set the number of data bytes in the file.
  //
  g_ulBytesRemaining = pSoundInfoHeader->ulDataSize;
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