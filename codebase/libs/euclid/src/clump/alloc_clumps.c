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
 * alloc_clumps()
 *
 * allocate memory for clump_order and interval_order
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
 *    int n_intervals - number of intervals to be catered for
 *
 *    int *n_intervals_alloc_p - pointer to N_intervals_alloc in
 *      calling routine, which should be static.
 *      Must be initialized to 0 before first call to alloc_clumps().
 *      Typical declaration in calling routine is:
 *        static int N_intervals_alloc = 0;
 *
 *    Clump_order **clumps_p - pointer to Clumps in calling routine which
 *      should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Clump_order *Clumps = NULL;
 *    
 *    Interval ***interval_order_p - pointer to Interval_order
 *      in calling routine which should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Interval **Interval_order = NULL;
 *    
 * OUTPUTS:
 *
 *    *n_intervals_alloc_p, *clumps_p and *interval_order_p are set to
 *    new values as necessary to meet the needs of reallocation.
 *
 * RETURNS:
 *
 *   void
 *
 */

void EG_alloc_clumps(int n_intervals,
		     int *n_intervals_alloc_p,
		     Clump_order **clumps_p,
		     Interval ***interval_order_p)

{

  int n_intervals_alloc = *n_intervals_alloc_p;
  Clump_order *clumps = *clumps_p;
  Interval **interval_order = *interval_order_p;
  
  if (n_intervals > n_intervals_alloc) {
    
    /*
     * allocate space for clumps
     */
    
    if (clumps == NULL) {
      
      clumps = (Clump_order *)
	EG_malloc((unsigned int) ((n_intervals+2)*sizeof(Clump_order)));
      
    } else {
      
      clumps = (Clump_order *)
	EG_realloc((char *) clumps,
		   (unsigned int) ((n_intervals+2)*sizeof(Clump_order)));
      
    } /* if (clumps == NULL) */
    
    /*
     * allocate space for interval_order
     */
    
    if (interval_order == NULL) {
      
      interval_order = (Interval **)
	EG_malloc((unsigned int) ((n_intervals+1)*sizeof(Interval *)));
      
    } else {
      
      interval_order = (Interval **)
	EG_realloc((char *) interval_order,
		   (unsigned int) ((n_intervals+1)*sizeof(Interval *)));
      
    }

    n_intervals_alloc = n_intervals;
    
  } /* if (n_intervals > n_intervals_alloc) */

  *n_intervals_alloc_p = n_intervals_alloc;
  *clumps_p = clumps;
  *interval_order_p = interval_order;

}

void EG_free_clumps(int *n_intervals_alloc_p,
		    Clump_order **clumps_p,
		    Interval ***interval_order_p)

{

  int n_intervals_alloc = *n_intervals_alloc_p;
  Clump_order *clumps = *clumps_p;
  Interval **interval_order = *interval_order_p;
  
  if (clumps) {
    EG_free((void *) clumps);
  }
  if (interval_order) {
    EG_free((void *) interval_order);
  }
  clumps = NULL;
  interval_order = NULL;
  n_intervals_alloc = 0;

  *n_intervals_alloc_p = n_intervals_alloc;
  *clumps_p = clumps;
  *interval_order_p = interval_order;

}
