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
 * mhr_analysis.c
 *
 * Reads a radar data stream from an input device, and analyzes it.
 * Optionally prints out header and summary.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Dec 1993
 *
 ****************************************************************************/

#define MAIN
#include "mhr_analysis.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)

{
  
  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    malloc((unsigned) sizeof(global_t));

  /*
   * register function to trap termination and interrupts
   */

  signal(SIGTERM, (void (*)())tidy_and_exit);
  signal(SIGINT, (void (*)())tidy_and_exit);
  signal(SIGQUIT, (void (*)())tidy_and_exit);

  /*
   * set program name
   */
  
  Glob->prog_name = "mhr_analysis";
  
  /*
   * parse the command line arguments
   */
  
  parse_args(argc, argv);

  /*
   * if filelist arg set, list the files on the tape
   * and then quit
   */

  if (Glob->filelist) {

    list_tape_files();
    return(0);

  } 

  /*
   * process the data
   */

  process_data_stream();

  tidy_and_exit(0);

  return(0);

}

