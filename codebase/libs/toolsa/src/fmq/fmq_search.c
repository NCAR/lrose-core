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
 * fmq_search.c
 *
 * Search routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>

/*****************************
 * fmq_slot_in_active_region()
 *
 * Checks if the given slot is within the active
 * region or not.
 *
 * This is based upon the settings of oldest_slot and youngest_slot.
 * If oldest_slot is less than youngest_slot, then the active area
 * is between the two, inclusive of the two.
 * Otherwise, it is the inactive area which is between them.
 * See fmq.doc.
 *
 * Return value:
 *   TRUE if YES, FALSE if NO.
 */

int fmq_slot_in_active_region (FMQ_handle_t *handle, int slot_num)

{

  /*
   * case in which no slots have yet been used - both oldest
   * and youngest are -1
   */
  
  if (handle->fstat.youngest_slot < 0 ||
      handle->fstat.oldest_slot < 0) {
    return FALSE;
  }

  if (handle->fstat.youngest_slot >= handle->fstat.oldest_slot) {
    
    if (slot_num >= handle->fstat.oldest_slot &&
	slot_num <= handle->fstat.youngest_slot) {
      return TRUE;
    } else {
      return FALSE;
    }
    
  } else {
    
    if (slot_num >= handle->fstat.oldest_slot ||
	slot_num <= handle->fstat.youngest_slot) {
      return TRUE;
    } else {
      return FALSE;
    }
    
  }

}

/*****************
 * fmq_next_slot()
 *
 * Returns the slot number following the one given.
 *
 */

int fmq_next_slot (FMQ_handle_t *handle, int slot_num)

{

  if (slot_num >= handle->fstat.nslots - 1) {
    return 0;
  } else {
    return (slot_num + 1);
  }

}

/*****************
 * fmq_prev_slot()
 *
 * Returns the slot number before the one given.
 *
 */

int fmq_prev_slot (FMQ_handle_t *handle, int slot_num)

{

  if (slot_num == 0) {
    return (handle->fstat.nslots - 1);
  } else {
    return (slot_num - 1);
  }

}

/***************
 * fmq_next_id()
 *
 * Returns the id number following the one given.
 *
 */

int fmq_next_id (int id)

{

  if (id == FMQ_MAX_ID - 1) {
    return 0;
  } else {
    return (id + 1);
  }

}

/***************
 * fmq_prev_id()
 *
 * Returns the id number before the one given.
 *
 */

int fmq_prev_id (int id)

{
  if (id == 0) {
    return (FMQ_MAX_ID - 1);
  } else {
    return (id - 1);
  }
}

/***********************
 * fmq_find_slot_for_id()
 *
 * Loads the slot number for the given id.
 *
 * Returns 0 on success, -1 on error.
 *
 */

int fmq_find_slot_for_id (FMQ_handle_t *handle,
			  int search_id, int *slot_p)
     
{

  int islot;
  fmq_slot_t *slot;

  /*
   * special case - search_id is -1, no messages yet.
   * return slot num of -1.
   */

  if (search_id == -1) {
    *slot_p = -1;
    return 0;
  }

  if (fmq_read_slots(handle)) {
    return -1;
  }

  slot = handle->fslots;
  for (islot = 0; islot < handle->fstat.nslots; islot++, slot++) {
    
    if (search_id == slot->id) {
      *slot_p = islot;
      return 0;
    }

  } 

  return -1;

}

