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
 * fmq_alloc.c
 *
 * Allocation routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>
#include <toolsa/umisc.h>

/*******************
 * fmq_alloc_slots()
 *
 * Memory alloc for slots.
 *
 */

int fmq_alloc_slots(FMQ_handle_t *handle, int nslots)
     
{

  if (handle->nslots_alloc < nslots) {

    if (handle->fslots == NULL) {
      handle->fslots =
	(fmq_slot_t *) umalloc (nslots * sizeof(fmq_slot_t));
    } else {
      handle->fslots =
	(fmq_slot_t *) urealloc (handle->fslots,
				 nslots * sizeof(fmq_slot_t));
    }

    handle->nslots_alloc = nslots;
    
  } /* if (handle->nslots_alloc < nslots) */

  return 0;

}

/******************
 * fmq_free_slots()
 *
 * Memory free for slots.
 *
 */

void fmq_free_slots(FMQ_handle_t *handle)
     
{

  if (handle->fslots != NULL) {

    ufree(handle->fslots);
    handle->fslots = NULL;
    handle->nslots_alloc = 0;

  }

}

/*******************
 * fmq_alloc_entry
 *
 * Memory alloc for msg entry
 *
 */

void fmq_alloc_entry(FMQ_handle_t *handle,
		     int msg_len)
     
{

  int nbytes_padded;
  int nbytes_needed;

  /*
   * compute padded length, to keep the total length aligned to
   * 32-bit words
   */
  
  nbytes_padded = (((msg_len - 1) / sizeof(si32)) + 1) * sizeof(si32);
  nbytes_needed = nbytes_padded + FMQ_NBYTES_EXTRA;
  
  if (handle->n_entry_alloc == 0) {
    handle->entry = umalloc (nbytes_needed);
    handle->n_entry_alloc = nbytes_needed;
  } else if (nbytes_needed > handle->n_entry_alloc) {
    handle->entry =
      urealloc (handle->entry, nbytes_needed);
    handle->n_entry_alloc = nbytes_needed;
  }
  memset(handle->entry, 0, nbytes_needed);
  
}

/*******************
 * fmq_free_entry
 *
 * Memory free for msg entry.
 *
 */

void fmq_free_entry(FMQ_handle_t *handle)
     
{

  if (handle->entry != NULL) {
    ufree(handle->entry);
    handle->entry = NULL;
    handle->n_entry_alloc = 0;
  }
  
}

