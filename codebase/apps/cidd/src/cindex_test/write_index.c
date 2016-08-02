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
 * write_index.c
 *
 * Writes the index file
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * October 1996
 *
 ****************************************************************************/

#include "cindex_test.h"

void write_index(void)

{

  static int first_call = TRUE;
  static si32 delta_time;

  date_time_t stime;
  time_t now;
  cdata_current_index_t index;

  if (first_call) {

    if (Glob->params.mode == ARCHIVE) {

      stime.year = Glob->params.start_time.year;
      stime.month = Glob->params.start_time.month;
      stime.day = Glob->params.start_time.day;
      stime.hour = Glob->params.start_time.hour;
      stime.min = Glob->params.start_time.min;
      stime.sec = Glob->params.start_time.sec;

      uconvert_to_utime(&stime);
      delta_time = stime.unix_time - time(NULL);

    } else {

      delta_time = 0;
      
    }

    first_call = FALSE;
    
  } /* if (first_call) */

  now = time(NULL) + delta_time;

  fill_current_index(&index, now, Glob->params.file_extension,
		     NULL, NULL);

  if (Glob->params.debug) {
    fprintf(stderr, "Writing index for time %s\n",
	    utimstr(now));
  }

  if (cdata_write_index_simple(Glob->params.data_dir,
			       &index, Glob->prog_name,
			       "write_index") == NULL) {
    
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot write index file in dir %s\n",
	    Glob->params.data_dir);

  }

  return;
  
}

