/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <rapformats/kav_grid.h>
#include <rapformats/km.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>

static ui08 *Input_buffer;
static ui08 *Beyond;
static ui08 *In_ptr;
static int Input_buffer_len;
static int Input_line_no;
static ui08 *Dbz_byte = NULL;

/* kavouras compression specific stuff  */
#define SETCOLOR 64
#define EXTENDCOLOR 32
#define READBYTE 36
#define FILLTOEOL 35
#define SEPARATOR 33
#define END_OF_GRID 34


/************************************************************************
 * KM_vip2dbz_init(): 
 */

void KM_vip2dbz_init(double scale, double bias,
		     double *vip2dbz, int vip2dbz_len,
		     int output_type, double dbz_threshold)
{
  si32 byte_val;
  int i;

  /*
   * check on number of vals in array
   */

  if (vip2dbz_len != N_KAV_COLORS)
  {
    fprintf(stderr, "ERROR - KM_vip2dbz_init\n");
    fprintf(stderr, "Must have %d elements in vip2dbz array\n",
	    N_KAV_COLORS);
/*    tidy_and_exit(-1); */
    exit(-1);
  }

  /*
   * allocate the dbz_byte array
   */

  if (Dbz_byte != NULL)
    ufree((char *) Dbz_byte);

  Dbz_byte = (ui08 *) umalloc
    ((ui32) (vip2dbz_len * sizeof(ui08)));

  if (output_type == KM_OUTPUT_PLAIN)
  {
    for (i = 0; i < vip2dbz_len; i++)
      Dbz_byte[i] = i;
  }
  else
  {
    for (i = 0; i < vip2dbz_len; i++)
    {
      if (vip2dbz[i] < dbz_threshold)
      {
	Dbz_byte[i] = 0;
      }
      else
      {
	byte_val = (si32)
	  floor(((vip2dbz[i] - bias) / scale) + 0.5);
	if (byte_val > 255)
	{
	  byte_val = 255;
	}
	if (byte_val < 1)
	{
	  byte_val = 1;
	}
	Dbz_byte[i] = byte_val;
      }
      
    } /* i */

  }

}


/************************************************************************
 * KM_unpack_init(): 
 */

void KM_unpack_init(char *file_buf, int file_len)
{
  Input_buffer = (ui08 *) (file_buf + KAV_DATA_OFFSET);
  In_ptr = Input_buffer;
  Input_buffer_len = file_len - KAV_DATA_OFFSET;
  Input_line_no = 0;
  Beyond = Input_buffer + Input_buffer_len;
}

/************************************************************************
 * KM_raw_line(): Repack the buffin; return the output buffer and its
 *                length.
 */

ui08 *KM_unpack_raw_line()
{
  static ui08 lineout[KAV_NLON];

  ui08 command;
  int color = 0, count;
  ui08 *out_ptr = lineout;
  int out_pos = 0;
  int done = FALSE;
  
  /*
   * check line no
   */

  if (Input_line_no >= KAV_NLAT)
  {
    fprintf(stderr, "ERROR - KM_unpack_raw_line\n");
    fprintf(stderr,
	    "Requested line number exceeds max of %d\n", KAV_NLAT);
    return ((ui08 *) NULL);
  }

  /*
   * initialize
   */

  memset((void *) lineout, 0, KAV_NLON);
  
  while (In_ptr < Beyond && !done)
  {
    /* the next input byte */
    command = *In_ptr;
    In_ptr++;
    
    if (command >= SETCOLOR)
    {
      /* short count :
       * If command is >= 64 then the a new color is
       * being started, High 2 bits contain the short count
       * (1-3), while the Lower 6 bits contain the new color.
       */
      
      color = (int) (63 & command);
      count = command >> 6;

    }
    else if (command < EXTENDCOLOR)
    {
      /* extended count: command byte is the count */
      count = (int) command;
      
    }
    else if (command == READBYTE)
    {
      /* Long extended count:
       * if command is == 36 then the next byte is
       * a long extended count between 32-255 of current color.
       */
      
      count = (int) (*In_ptr);
      In_ptr++;

    }
    else if (command == FILLTOEOL)
    {
      /* End of line:
       * if command is == 35 then fill to end of current line
       * with previous color.
       */
      
      count = KAV_NLON - out_pos;
      Input_line_no++;
      done = TRUE;

    }
    else if ((command == SEPARATOR) || (command == END_OF_GRID))
    {
      count = 0;
      done = TRUE;
      
    }
    else
    {
      fprintf(stderr, "ERROR - KM_unpack_raw_line\n");
      fprintf(stderr, "Illegal command val = %d = %x\n",
	      command, command);
      count = 0;
      
    }

    if (out_pos + count > KAV_NLON)
    {
      fprintf(stderr, "ERROR - KM_unpack_raw_line\n");
      fprintf(stderr, "Line length exceeded\n");
      count = KAV_NLON - out_pos;
      done = TRUE;
    }
    
    if (count > 0)
    {
      memset((void *) out_ptr, Dbz_byte[color], count);
      out_ptr += count;
      out_pos += count;
    }
    
  } /* while */
  
  return (lineout);

}
