#ifndef _WAV_DECODER_TASK_H_
#define _WAV_DECODER_TASK_H_


char initWAVDecoderTask(void);
void StartWAVPlay(void);
void PauseWAVPlay(void);
void ContinueWAVPlay(void);
void StopWAVPlay(void);
unsigned long GetStatusWAVPlay(void);
#endif //_WAV_DECODER_TASK_H_