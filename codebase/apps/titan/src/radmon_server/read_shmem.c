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
 * read_shmem.c
 *
 * reads a beam from the shared-memory buffer
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * August 1996
 *
 ****************************************************************************/

#include "radmon_server.h"

static si32 Last_beam_used = 0;

void read_shmem(void)

{

  int forever = TRUE;

  si32 this_beam_num;
  si32 nbeams_buffer;
  si32 last_print_time = 0;

  rdata_shmem_header_t *shdr = get_shmem_header();
  rdata_shmem_beam_header_t *beam_headers = get_beam_headers();
  rdata_shmem_beam_header_t *bhdr;

  nbeams_buffer = shdr->nbeams_buffer;
  
  while (forever) {

    /*
     * register the process
     */
  
    PMU_auto_register("In read_shmem()");

    /*
     * check for incoming beam
     */
    
    if(shdr->last_beam_written != Last_beam_used) {

      /*
       * Get next beam available
       */
      
      this_beam_num = (Last_beam_used + 1) % nbeams_buffer;

      bhdr = beam_headers + this_beam_num;

      if ((bhdr->beam_time - last_print_time) >=
	  Glob->params.update_interval) {
	fprintf(stdout, "  %s %5.1f %5.1f\r",
		utimstr(bhdr->beam_time),
		bhdr->azimuth, bhdr->elevation);
	last_print_time = bhdr->beam_time;
      }
      
      /*
       * Update last used
       */
    
      Last_beam_used = this_beam_num;
      
    } else {

      fflush(stdout);
      
      sleep(1);
      
    } /* if(shdr->last_beam_written.... */

  } /* while */
  
}

