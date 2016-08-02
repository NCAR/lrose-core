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
/******************************************************************
 * fmq_stat.c
 *
 * Status routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * May 1997
 *
 */

#include <toolsa/fmq_private.h>

/*****************************
 * fmq_fraction_used()
 *
 * Computes the fraction of the available
 * space used in terms of slots and buffer.
 *
 * returns 0 on success, -1 on failure.
 */

int fmq_fraction_used(FMQ_handle_t *handle,
		      double *slot_fraction_p,
		      double *buffer_fraction_p)

{

  int islot;
  int nslots_active;
  int nbytes_active;
  double slot_fraction;
  double buffer_fraction;
  fmq_slot_t *slot;

  /*
   * read in status struct 
   */
  
  if (fmq_read_stat(handle)) {
    return -1;
  }

  /*
   * read in slots array
   */
  
  if (fmq_read_slots(handle)) {
    return -1;
  }

  /*
   * count the active slots
   */

  nslots_active = 0;
  nbytes_active = 0;
  slot = handle->fslots;
  for (islot = 0; islot < handle->fstat.nslots; islot++, slot++) {
    if (slot->active) {
      nslots_active++;
      nbytes_active += slot->stored_len;
    }
  } 

  slot_fraction =
    (double) nslots_active / (double) handle->fstat.nslots;

  buffer_fraction =
    (double) nbytes_active / (double) handle->fstat.buf_size;

  *slot_fraction_p = slot_fraction;
  *buffer_fraction_p = buffer_fraction;

  return 0;

}

