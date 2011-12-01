/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

# include <stdio.h>

//# include <ls.h>
# include <string.h>

//# include <targets/lpc2148.h>
# include "mad.h"
//# include "debug.h"
# include "third_party/fatfs/src/ff.h"
//# include "lpc_io.h"
# include "midmad.h"
//#include "synth.h"
#include "SafeRTOS/SafeRTOS_API.h"
#include "priorities.h"
#include "file_management.h"
#include "audio_codec.h"
#include "sound_player_task.h"
#include "mp3_decoder_task.h"

static FIL * pFileObject;
static unsigned char  mp3_stream_buf[512*3];
/*			  
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */
static int decode(unsigned char const *, unsigned long);
static
enum mad_flow input(void *data, struct mad_stream *stream);
static
enum mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm);
static
enum mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame);
/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};
//==============================================================================

static unsigned long MP3DecoderTaskStack[2048];
static xTaskHandle pvCreatedTask ;

static unsigned long Flags = MP3_STOP;
static unsigned char CtrlFlag=0;
//
void StartMP3Play(void)
{
 // vTaskSuspendScheduler();
    CtrlFlag =0;
    Flags= MP3_START;
   // xTaskResumeScheduler();
    xTaskResume(pvCreatedTask);
    
}
void PauseMP3Play(void)
{
  //vTaskSuspendScheduler();
    CtrlFlag =1;
    //xTaskResumeScheduler();
}
void ContinueMP3Play(void)
{
  //vTaskSuspendScheduler();
    CtrlFlag =0;
    Flags = MP3_CONTINUE;
   // xTaskResumeScheduler();
    xTaskResume(pvCreatedTask);
    
}
void StopMP3Play(void)
{
  
    if(Flags == MP3_PAUSE)
    {
      xTaskResume(pvCreatedTask);
      Flags = MP3_STOP;
    }
    //vTaskSuspendScheduler();
    else if(Flags == MP3_CONTINUE)
      CtrlFlag =2;
    else
     Flags = MP3_STOP;
   // xTaskResumeScheduler();
      
}
unsigned long GetStatusMP3Play(void)
{
  return Flags;
}
//unsigned char set__=0;
static void
MP3DecoderTask(void *pvParameters)
{
    //unsigned short usCount;
    struct buffer Buffer;
    struct mad_decoder decoder;
    int result;
    FRESULT Result;
    
    
    CtrlFlag=0;
    Flags = MP3_STOP;
    pFileObject =&sSongFileObject;
    while(1)
    {
        if(Flags == MP3_STOP)
        {
            CtrlFlag =0;
            f_close(pFileObject);
            //Wait signal;
            xTaskSuspend(NULL);
        }
        else if(Flags == MP3_START)
        {
            f_close(pFileObject);
            /* release the decoder */
            mad_decoder_finish(&decoder);
            
            Result = f_open(pFileObject, sFilePath, FA_READ);
            if(Result != FR_OK)
            {
                Flags = MP3_STOP;
                
            }else
            {
              MinsCurrent=0;
              SecondCurrent=0;
              sSongInfoHeader.ulSecond =0;
              sSongInfoHeader.ulMins=0;
              Buffer.start  = mp3_stream_buf;
              Buffer.length = sizeof(mp3_stream_buf);
              initBuffer(2048);
              //set__=1;
              Flags = MP3_CONTINUE;
              /* configure input, output, and error functions */
  
              mad_decoder_init(&decoder, 
                            &Buffer,
                     input, 
                             0 /* header */, 
                             0 /* filter */,
                             output,
                     error, 
                             0 /* message */);
            }
        }
        else if(Flags == MP3_CONTINUE)
        {
            result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
            vTaskSuspendScheduler();
            if(CtrlFlag ==1)
              Flags = MP3_PAUSE;
            else//(CtrlFlag ==2)
              Flags = MP3_STOP;
            xTaskResumeScheduler();
        }else{
            //Wait signal
            xTaskSuspend(NULL);
        }
    }
}
/**
* Intialize, create all components for Sound Player Task
*/
char initMP3DecoderTask(void){
  //
  // Create the task.
  //
  if(xTaskCreate(MP3DecoderTask, (signed portCHAR *)"mp3",
                   (signed portCHAR *)MP3DecoderTaskStack,
                   sizeof(MP3DecoderTaskStack), NULL, PRIORITY_SOUND_PLAYER_TASK+1,
                   &pvCreatedTask) != pdPASS)
  {
    return(1);//Fail
  }
  
  return 0;//Success
}

//==============================================================================


void abort(void)
{
}

void mp3_player(FIL *FileObject)
{ 
    pFileObject = FileObject;
    FRESULT Result;
    unsigned short usCount;
 /*   static unsigned char g_pucBuffer[4096];
    Result = f_read(FileObject, g_pucBuffer, 4096, &usCount);
    if(Result != FR_OK)
    {
        f_close(FileObject);
        return;
    }*/
    decode((void *)mp3_stream_buf, sizeof(mp3_stream_buf));
    f_close(FileObject);
}


/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow input(void *data, struct mad_stream *stream)
{
  struct buffer *buffer = data;
  unsigned int  rb = 0;
  unsigned short lb;
  FRESULT fResult;
  //-----------
  if(CtrlFlag)
    return MAD_FLOW_STOP;
  //-------------
  if (!buffer->length)
    return MAD_FLOW_STOP;

  if (stream->this_frame && stream->next_frame)
  {
    rb = (unsigned int)(buffer->length) - 
         (unsigned int)(stream->next_frame - stream->buffer);

    memmove((void *)stream->buffer, (void *)stream->next_frame, rb);
    fResult = f_read(pFileObject,(void *)(stream->buffer + rb),(WORD) (buffer->length - rb),(WORD*)&lb );
    if(fResult != FR_OK)
      return MAD_FLOW_STOP;
  }
  else  
    if(f_read(pFileObject,(void *)buffer->start,(WORD) (buffer->length), (WORD*)&lb) != FR_OK)
      return MAD_FLOW_STOP;

  if (lb == 0)
  {
    
    //wait_end_of_excerpt();
    buffer->length = 0;
    return MAD_FLOW_STOP;
  }
  else 
    buffer->length = lb + rb;

  mad_stream_buffer(stream, buffer->start, buffer->length);
  //  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static
enum mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  unsigned int samplerate;
  //  unsigned int i;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  samplerate = pcm->samplerate;

  //TOGGLE_LIVE_LED0();

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static
enum mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  // struct buffer *buffer = data;

  // printf("decoding error 0x%04x (%s) at byte offset %u\n",
  //	  stream->error, mad_stream_errorstr(stream),
  // 	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static
int decode(unsigned char const *start, unsigned long length)
{
  struct buffer buffer;
  struct mad_decoder decoder;
  int result;

  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, 
                  &buffer,
		   input, 
                   0 /* header */, 
                   0 /* filter */,
                   output,
		   error, 
                   0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
/* Get ID3v2 tag string from buffer
 *
 *
 *
 *
 */

/*
char* GetID3v2TagStringFromBuffer( char* frame_type, char* buffer, int buffer_length )
{
  char* scan_ptr = buffer;
  int frame_type_length;
  int match;
  int found_frame = 0;
  int frame_length;
  char* return_string;
  int return_string_length;
  
  frame_type_length = strlen( frame_type );
  
  do
  {
    match = strncmp( frame_type, scan_ptr, frame_type_length );
    if( match == 0 )
    {
      found_frame = 1;
      break;
    }
    scan_ptr++;
    // Scan to end of buffer (minus length of frame ID
  } while( scan_ptr < ( buffer + ( buffer_length - frame_type_length )));
  
  if( found_frame == 1 )
  {
    frame_length = GetUnalignedInt( scan_ptr + 4 );
    if( *( scan_ptr + 10 ) ==  0x0 ) // ISO-8859-1
    {
      return_string = malloc( frame_length );
      if(return_string == NULL)
      {
    //  printf("ERROR: Allocating MEmory\n");
        return NULL;
      }
      strncpy( return_string, scan_ptr + 11, frame_length-1 );
      return_string[frame_length - 1] = 0;  // Just to make sure it's terminated
    }
    else if( *( scan_ptr + 10 ) ==  0x1 ) // Unicode
    {
      return_string_length = (( frame_length - 3 ) / 2 ) + 1;
      return_string = malloc( return_string_length );
      CopyUnicodeString( return_string, scan_ptr + 11, frame_length-1 );
      return_string[return_string_length - 1] = 0;  // Just to make sure it's terminated
    }
  }
  else
  {
    return_string = NULL;
  }
  
  return( return_string );  
}


*/
/* Get length of mp3 file from buffer
 *
 *
 *
 *
 */
/*
void Mp3_length(char *MM,char* SS, FIL *FileObject)
{
  int length=0;
  FRESULT Result;
  unsigned short usCount;
  char Buffer[256];
  Result = f_read(FileObject, Buffer, 256, &usCount);
  if(Result != FR_OK)
  {
      f_close(FileObject);
      return;
  }
  length = (int) GetID3v2TagStringFromBuffer( "TLEN", Buffer, 256);
  MM = (char*) (length / 60000);
  SS = (char*) ((length % 60000)/1000);
}
*/