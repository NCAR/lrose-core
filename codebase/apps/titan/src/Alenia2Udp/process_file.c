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
 * process_file.c
 *
 * Process one file
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * July 1997
 *
 **************************************************************************/

#include "Alenia2Udp.h"

int process_file(char *file_path)

{

  ui08 *beam_buffer;
  int len;
  int sleep_usecs;

  /*
   * open input file and start udp transmission
   */
  
  if (open_input_file(file_path)) {
    return (-1);
  }
  start_udp_transmission();

  /*
   * read through data
   */
  
  sleep_usecs = Glob->params.beam_wait_msecs * 1000;
  while(read_input_beam(&beam_buffer, &len) == 0) {
    if(write_output_udp(beam_buffer, len)) {
      close_input_file();
      return (-1);
    }
    uusleep(sleep_usecs);
  }
  
  /*
   * close input file and stop transmission
   */

  close_input_file();
  if (stop_udp_transmission()) {
    return (-1);
  }

  return (0);
  
}


