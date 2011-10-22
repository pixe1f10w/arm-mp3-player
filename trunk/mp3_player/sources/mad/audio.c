/*
 * madplay - MPEG audio decoder and player
 * Copyright (C) 2000-2004 Robert Leslie
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
 * $Id: audio.c,v 1.36 2004/01/23 09:41:31 rob Exp $
 */

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "global.h"

# include <string.h>
# include "mad/mad.h"

# include "audio.h"

char const *audio_error;

static struct audio_dither left_dither, right_dither;

# if defined(_MSC_VER)
#  pragma warning(disable: 4550)  /* expression evaluates to a function which
				     is missing an argument list */
# endif


/*
 * NAME:	audio_control_init()
 * DESCRIPTION:	initialize an audio control structure
 */
void audio_control_init(union audio_control *control,
			enum audio_command command)
{
  switch (control->command = command) {
  case AUDIO_COMMAND_INIT:
    control->init.path = 0;
    break;

  case AUDIO_COMMAND_CONFIG:
    control->config.channels  = 0;
    control->config.speed     = 0;
    control->config.precision = 0;
    break;

  case AUDIO_COMMAND_PLAY:
    control->play.nsamples   = 0;
    control->play.samples[0] = 0;
    control->play.samples[1] = 0;
    control->play.mode       = AUDIO_MODE_DITHER;
    control->play.stats      = 0;
    break;

  case AUDIO_COMMAND_STOP:
    control->stop.flush = 0;
    break;

  case AUDIO_COMMAND_FINISH:
    break;
  }
}

/*
 * NAME:	clip()
 * DESCRIPTION:	gather signal statistics while clipping
 */
static inline
void clip(mad_fixed_t *sample, struct audio_stats *stats)
{
  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  if (*sample >= stats->peak_sample) {
    if (*sample > MAX) {
      ++stats->clipped_samples;
      if (*sample - MAX > stats->peak_clipping)
	stats->peak_clipping = *sample - MAX;

      *sample = MAX;
    }
    stats->peak_sample = *sample;
  }
  else if (*sample < -stats->peak_sample) {
    if (*sample < MIN) {
      ++stats->clipped_samples;
      if (MIN - *sample > stats->peak_clipping)
	stats->peak_clipping = MIN - *sample;

      *sample = MIN;
    }
    stats->peak_sample = -*sample;
  }
}

/*
 * NAME:	audio_linear_round()
 * DESCRIPTION:	generic linear sample quantize routine
 */
# if defined(_MSC_VER)
extern  /* needed to satisfy bizarre MSVC++ interaction with inline */
# endif
inline
signed long audio_linear_round(unsigned int bits, mad_fixed_t sample )
{
  signed long ret_value;
  mad_fixed_t orig_sample = sample;
  
  /* round */
  sample += (1L << (MAD_F_FRACBITS - bits));

  /* clip */
//  clip(&sample, stats);

  /* quantize and scale */
  ret_value = sample >> (MAD_F_FRACBITS + 1 - bits);
  return ret_value;
}

/*
 * NAME:	prng()
 * DESCRIPTION:	32-bit pseudo-random number generator
 */
static inline
unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/*
 * NAME:	audio_linear_dither()
 * DESCRIPTION:	generic linear sample quantize and dither routine
 */
# if defined(_MSC_VER)
extern  /* needed to satisfy bizarre MSVC++ interaction with inline */
# endif
inline
signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
        struct audio_dither *dither )
{
  unsigned int scalebits;
  mad_fixed_t output, mask, random;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random  = prng(dither->random);
  output += (random & mask) - (dither->random & mask);

  dither->random = random;


  /* clip */
/*
  if (output > MAX) {
      output = MAX;
  
      if (sample > MAX)
          sample = MAX;
  }
  else if (output < MIN) {
      output = MIN;
  
      if (sample < MIN)
          sample = MIN;
  }
*/

  /* quantize */
  output &= ~mask;

  /* error feedback */
  dither->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}

