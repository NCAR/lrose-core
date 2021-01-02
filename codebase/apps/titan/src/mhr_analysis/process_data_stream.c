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
 * process_data_stream.c
 *
 * reads in the beam data and processes it
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1993
 *
 **************************************************************************/

#include "mhr_analysis.h"

void process_data_stream(void)

{

  char *beam_buffer;

  int forever = TRUE;
  int read_size;
  int header_count = 0;
  int summary_count = 0;
  
  /*
   * enter loop
   */

  while (forever) {

    /*
     * read in a beam from the tape
     */
    
    read_size = read_tape_beam(&beam_buffer);

    /*
     * if logical record size is zero, the end of the
     * tape has been reached, so return.
     */
    
    if (read_size == 0)
      return;
    
    /*
     * print out as required
     */

    /*
     * print header if requested
     */
    
    if (Glob->header_print) {
      
      if (header_count == 0)
	print_header(beam_buffer);
	
      header_count++;
	
      if (header_count == Glob->header_interval)
	header_count = 0;
      
    } /* if (Glob->header_print) */
      
    /*
     * print summary if requested
     */
      
    if (Glob->summary_print) {
      
      if (summary_count == 0)
	  
	print_summary(beam_buffer);
	
      summary_count++;
	
      if (summary_count == Glob->summary_interval)
	summary_count = 0;
      
    } /* if (Glob->summary_print) */
    
    /*
     * print summary if requested
     */
      
    if (Glob->analyze) {
      
      perform_analysis(beam_buffer);

    } /* if (Glob->analyze) */
    
  } /* while */

}

