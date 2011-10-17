#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_
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
#endif //_WAV_FILE_H_