
/*--------------------------------------------------------------------------*/
/* Author:                 Paolo Bernasconi                                 */
/* Project:                MP3                                              */
/* Filename:               midmad.h                                         */
/* Version:                1.00                                             */
/* First Version Date:     March 2006                                       */
/* Date:                                                                    */
/* Language:               GNU GCC "C" Language                             */
/* Functional description: queue management functions                       */

/*--------------------------------------------------------------------------*/
#ifndef  __MIDMAD_H
#define  __MIDMAD_H

//--# include <efs.h>
# include "third_party/fatfs/src/ff.h"
/* Play an MP3 file fom the memory card */
//--void mp3_play(EmbeddedFile *file);     
void mp3_player(FIL *FileObject);//++     
#endif
