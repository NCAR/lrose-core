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
/*****************************************************************************
 * setup_shmem.c
 *
 * set up the shared memory header and buffer
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 ****************************************************************************/

#include "polar2mdv.h"

static char *Shmem_buffer;
static rdata_shmem_header_t *Shmem_header;
static int Sem_id;
static field_params_t *Field_params;
static rdata_shmem_beam_header_t *Beam_headers;

/*
 * main module routine
 */

void setup_shmem(scan_table_t *scan_table)

{

  rdata_shmem_header_t *shdr;

  /*
   * get the semaphore set
   */
  
  if ((Sem_id = usem_get(Glob->shmem_key,
			 N_POLAR_INGEST_SEMS)) < 0) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot get semaphore set, key = %x\n",
	    Glob->shmem_key);
    tidy_and_exit(-1);
  }

  /*
   * test the not_ready sem, which is set
   * once polar_ingest has loaded up the header data
   * This is first done on a non-blocking loop so that
   * PMU_register may be called
   */

  if (Glob->debug) {
    fprintf(stderr,
	    "%s:setup_shmem - waiting for not_ready sem to clear\n",
	    Glob->prog_name);
  }

  while (usem_check(Sem_id, POLAR_INGEST_NOT_READY_SEM)) {
    PMU_auto_register("Waiting for sems");
    sleep(1);
  }
  
  if (usem_test(Sem_id, POLAR_INGEST_NOT_READY_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Testing not-ready semaphore\n");
    tidy_and_exit(-1);
  }
  
  if (Glob->debug) {
    fprintf(stderr,
	    "%s:setup_shmem - not_ready sem cleared\n",
	    Glob->prog_name);
  }

  PMU_force_register("Got sems");

  /*
   * set the active sem
   */

  if (usem_set(Sem_id, POLAR2MDV_ACTIVE_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Setting activesemaphore\n");
    tidy_and_exit(-1);
  }

  /*
   * get shared memory for the header - blocks till available
   */

  if ((Shmem_header = (rdata_shmem_header_t *)
       ushm_get(Glob->shmem_key + 1, sizeof(rdata_shmem_header_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Cannot get shared memory for header, key = %x\n",
	    Glob->shmem_key + 1);
  }
  
  /*
   * get shared memory for buffer - blocks till available
   */

  if ((Shmem_buffer =
       ushm_get(Glob->shmem_key + 2,
		(int) Shmem_header->buffer_size)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Cannot get shared memory for buffer, key = %x\n",
	    Glob->shmem_key + 2);
  }
  
  /*
   * set pointers into shared mem
   */
  
  Field_params = (field_params_t *)
    (Shmem_buffer + Shmem_header->field_params_offset);
  
  Beam_headers = (rdata_shmem_beam_header_t *)
    (Shmem_buffer + Shmem_header->beam_headers_offset);

  /*
   * fill in relevant radar parameters
   */

  shdr = Shmem_header;

  shdr->radar.ngates = shdr->ngates;
  shdr->radar.nelevations = scan_table->nelevations;
  shdr->radar.nazimuths = scan_table->nazimuths;

  shdr->radar.delta_azimuth =
    (si32) (scan_table->delta_azimuth * DEG_FACTOR + 0.5);
  shdr->radar.start_azimuth =
    (si32) (scan_table->start_azimuth * DEG_FACTOR + 0.5);

}

/*
 * access routines
 */

char *get_shmem_buffer(void)
{
  return (Shmem_buffer);
}

rdata_shmem_header_t *get_shmem_header(void)
{
  return (Shmem_header);
}

int get_sem_id(void)
{
  return (Sem_id);
}

field_params_t *get_field_params(void)
{
  return (Field_params);
}

rdata_shmem_beam_header_t *get_beam_headers(void)
{
  return (Beam_headers);
}

