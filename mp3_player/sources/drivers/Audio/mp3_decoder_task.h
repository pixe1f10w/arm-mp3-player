#ifndef _MP3_DECODER_TASK_H_
#define _MP3_DECODER_TASK_H_
//
void mp3_player(FIL *FileObject);//++  
//
char initMP3DecoderTask(void);
void StartMP3Play(void);
void PauseMP3Play(void);
void ContinueMP3Play(void);
void StopMP3Play(void);
unsigned long GetStatusMP3Play(void);
#endif //_MP3_DECODER_TASK_H_