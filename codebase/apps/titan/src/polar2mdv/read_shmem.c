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
 * reads a mile-high beam from the shared-memory buffer
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 * Hacked from 'ray' code of Gary Wiener / Bill Myers
 *
 ****************************************************************************/

#include "polar2mdv.h"

#define QUIT_CHECK_INTERVAL 50

static si32 Last_beam_used = 0;

void read_shmem(vol_file_handle_t *v_handle,
		rc_table_file_handle_t *rc_handle,
		clutter_table_file_handle_t *clutter_handle)

{

  char *shmem_buffer = get_shmem_buffer();
  ui08 *field_data;

  int forever = TRUE;
  int sem_id = get_sem_id();
  int end_of_vol_found = FALSE;

  si32 this_beam_num;
  si32 nbeams_buffer;

  register int loop_count = 0;

  rdata_shmem_header_t *shdr = get_shmem_header();
  rdata_shmem_beam_header_t *beam_headers = get_beam_headers();
  rdata_shmem_beam_header_t *bhdr;

  nbeams_buffer = shdr->nbeams_buffer;
  
  while (forever) {

    /*
     * Once in a while, check the polar_ingest quit semaphore,
     * and quit if it is set. Otherwise, just register the
     * process and server
     */

    loop_count++;

    if (loop_count > QUIT_CHECK_INTERVAL) {

      if (usem_check(sem_id, POLAR_INGEST_QUIT_SEM))
	tidy_and_exit(0);

      if (Glob->debug)
	fprintf(stderr, "Last written, last_used = %ld, %ld\n",
		(long) shdr->last_beam_written,
		(long) shdr->last_beam_used);
	   
      loop_count = 0;

      /*
       * register process and server
       */
      
      register_server();

    }
    
    /*
     * check to see if late end of vol has been set
     */
    
    if (shdr->late_end_of_vol) {
      
      bhdr = beam_headers + Last_beam_used;
      
      if (bhdr->end_of_volume) {
	
	/*
	 * end_of_vol flag was found, so write out volume
	 */
	
	handle_end_of_volume();
	shdr->late_end_of_vol = FALSE;
	
      } /* if (bhdr->end_of_volume) */
      
    } /* if (shdr->late_end_of_vol) */

    /*
     * check for incoming beam
     */
    
    if(shdr->last_beam_written != Last_beam_used) {

      /*
       * Get next beam available
       */

      this_beam_num = (Last_beam_used + 1) %
	nbeams_buffer;

      bhdr = beam_headers + this_beam_num;

      field_data = (ui08 *) shmem_buffer + bhdr->field_data_offset;
      
      /*
       * process the beam
       */
      
      process_beam(bhdr, field_data, v_handle, rc_handle, clutter_handle);
      
      /*
       * Update last used
       */
    
      Last_beam_used = this_beam_num;
      shdr->last_beam_used = Last_beam_used;

      end_of_vol_found = FALSE;
      
    } else {

      uusleep((ui32) Glob->shmem_read_wait);

    } /* if(shdr->last_beam_written.... */

  } /* while */
  
}

