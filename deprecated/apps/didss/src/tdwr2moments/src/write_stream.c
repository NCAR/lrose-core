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
/***************************************************************************
 * write_stream.c
 *
 * Write the output stream.
 *
 * Buffer copy is made to avoid side effects from byte_swapping
 * in the output modules.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include <toolsa/umisc.h>
# include   "tdwr2moments.h"

extern	Global	Glob;

static ui08 *alloc_buffer_copy(int len);

int write_stream(ui08 *write_buffer, int nwrite)

{
  
  static int count = 0;
  int iret;
  ui08 *buffer_copy;
  char *wheel = "|/-\\";
  static int j = 0;

  (void) printf ("    %c\r", wheel[j%4]);
  fflush (stdout);
  j++;

  if (Glob.write_fmq_output) {

    buffer_copy = alloc_buffer_copy(nwrite);
    memcpy(buffer_copy, write_buffer, nwrite);
  
	Beam_hdr_to_BE((Beam_hdr *) buffer_copy);
    if (write_output_fmq(buffer_copy, nwrite)) {
      return (-1);
    }

  }

  if (Glob.write_shm_output) 
  {
    buffer_copy = alloc_buffer_copy(nwrite);
    memcpy(buffer_copy, write_buffer, nwrite);

	Beam_hdr_to_BE((Beam_hdr *) buffer_copy);
    iret = send_shm(2, nwrite, buffer_copy);
    count++;
    if (count == 360) {
      count = 0;
    }
    if (iret < 4) {
      if (count == 0) 
      {
	if (iret == 3)
        {
	  fprintf(stderr, "\nwrite_stream - output buffer is full\n");
	} else 
        {
	  fprintf(stderr, "\nwrite_stream - client delay\n");
	}
      }
      return (-1);
    } /* if (iret < 4) */

  }

  return (0);
    
}

static ui08 *alloc_buffer_copy(int nwrite)

{

  static ui08 *buffer_copy = NULL;
  static int nalloc = 0;

  if (nwrite > nalloc) {
    if (buffer_copy == NULL) {
      buffer_copy = umalloc(nwrite);
    } else {
      buffer_copy = urealloc(buffer_copy, nwrite);
    }
    nalloc = nwrite;
  }

  return (buffer_copy);

}

