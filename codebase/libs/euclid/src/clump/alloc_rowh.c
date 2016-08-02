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
/**************************************************
 * alloc_rowh()
 *
 * allocate memory for the row headers for clumping
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307
 *
 * May 1995
 */

#include <euclid/clump.h>
#include <euclid/alloc.h>

/*
 *  DESCRIPTION:    
 *
 *    Allocates or reallocates the row header array.
 *
 *  INPUTS:
 *
 *    int ny - number of rows needed
 *
 *    int *nrows_alloc_p - pointer to nrows_alloc in calling routine,
 *      which should be static. Must be initialized to 0 before first
 *      call to alloc_rowh().
 *      Typical declaration in calling routine is:
 *        static int Nrows_alloc = 0;
 *
 *    Row_hdr **rowh_p - pointer to rowh in calling routine which
 *      should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Row_hdr *Rowh = NULL;
 *    
 * OUTPUTS:
 *
 *    *nrows_alloc_p and *rowh_p are set to new values as necessary
 *    to meet the needs of reallocation.
 *
 * RETURNS:
 *
 *   void
 *
 */


void EG_alloc_rowh(int ny, int *nrows_alloc_p, Row_hdr **rowh_p)

{
  
  int nrows_alloc = *nrows_alloc_p;
  Row_hdr *rowh = *rowh_p;

  if (ny > nrows_alloc) {

    if (rowh == NULL) {
      rowh = (Row_hdr *) EG_malloc ((unsigned int) (ny * sizeof(Row_hdr)));
    } else {
      rowh = (Row_hdr *)
	EG_realloc ((char *) rowh, (unsigned int) (ny * sizeof(Row_hdr)));
    }

    nrows_alloc = ny;

  }

  *nrows_alloc_p = nrows_alloc;
  *rowh_p = rowh;

  return;

}

void EG_free_rowh(int *nrows_alloc_p, Row_hdr **rowh_p)

{
  
  int nrows_alloc = *nrows_alloc_p;
  Row_hdr *rowh = *rowh_p;

  if (rowh) {
    EG_free(rowh);
  }
  rowh = (Row_hdr *) NULL;
  nrows_alloc = 0;

  *nrows_alloc_p = nrows_alloc;
  *rowh_p = rowh;

  return;

}

