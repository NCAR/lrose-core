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

/****************************************************************************
 * alloc.c
 *
 * Perform memory allocation
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include "forecast_monitor.h"

void alloc_simple_array(si32 n_simple_tracks,
			fm_simple_track_t **simple_array_p)

{
  
  static fm_simple_track_t *simple_array = NULL;
  static si32 n_alloc = 0;
  si32 n_extra_needed;
  
  /*
   * allocate array for simple tracks
   */
  
  n_extra_needed = n_simple_tracks - n_alloc;

  if (n_extra_needed > 0) {

    if (simple_array == NULL) {

      simple_array = (fm_simple_track_t *) umalloc
	((ui32) (n_simple_tracks * sizeof(*simple_array)));

    } else {

      simple_array = (fm_simple_track_t *) urealloc
	((char *) simple_array,
	 (ui32) (n_simple_tracks * sizeof(*simple_array)));

    }
    
    /*
     * initialize new members
     */
    
    memset((void *) (simple_array + n_alloc), 0,
	   (ui32) (n_extra_needed * sizeof(*simple_array)));

    n_alloc = n_simple_tracks;

  }

  *simple_array_p = simple_array;

  return;

}

void alloc_runs(fm_storm_t *storm)

{

  storm->n_runs = storm->gprops.n_runs;

  if (storm->n_runs > storm->n_runs_alloc) {

    if (storm->runs == NULL) {
      storm->runs = (storm_file_run_t *)
	umalloc(storm->n_runs * sizeof(*storm->runs));
    } else {
      storm->runs = (storm_file_run_t *)
	urealloc((char *) storm->runs,
		 storm->n_runs * sizeof(*storm->runs));
    }

    storm->n_runs_alloc = storm->n_runs;
    
  }

  return;

}

#ifdef NOTNOW

void alloc_grids(ui08 ***forecast_grid_p,
		 ui08 ***truth_grid_p)

{

  static int first_call = TRUE;
  static ui08 **forecast_grid;
  static ui08 **truth_grid;
  si32 nbytes_grid;
  
  if (first_call) {
    
    nbytes_grid = Glob->params.verify_grid.nx * Glob->params.verify_grid.ny;
    
    forecast_grid = (ui08 **) ucalloc2
      ((ui32) Glob->params.verify_grid.ny,
       (ui32) Glob->params.verify_grid.nx,
       (ui32) sizeof(ui08));
    
    truth_grid = (ui08 **) ucalloc2
      ((ui32) Glob->params.verify_grid.ny,
       (ui32) Glob->params.verify_grid.nx,
       (ui32) sizeof(ui08));
    
    first_call = FALSE;
    
  } /* if (first_call) */

  *forecast_grid_p = forecast_grid;
  *truth_grid_p = truth_grid;

  return;
  
}

void alloc_entries(fm_simple_track_t *strack)

{

  si32 n_extra_needed;
  
  strack->n_entries = strack->params.duration_in_scans;

  n_extra_needed = strack->n_entries - strack->n_alloc;
  
  if (n_extra_needed > 0) {

    if (strack->entries == NULL) {
      strack->entries = (fm_storm_t *)
	umalloc(strack->n_entries * sizeof(*strack->entries));
    } else {
      strack->entries = (fm_storm_t *)
	urealloc((char *) strack->entries,
		 strack->n_entries * sizeof(*strack->entries));
    }

    /*
     * initialize new members
     */
    
    memset((void *) (strack->entries + strack->n_alloc), 0,
	   (ui32) (n_extra_needed * sizeof(*strack->entries)));
    
    strack->n_alloc = strack->n_entries;
    
  }

  return;

}

void alloc_lookup(fm_lookup_t *lookup)

{

  lookup->nx = lookup->scan.cart.nx;
  lookup->ny = lookup->scan.cart.ny;

  if (lookup->nx > lookup->nx_alloc) {

    if (lookup->x == NULL) {
      lookup->x = (si32 *) umalloc (lookup->nx * sizeof(si32));
    } else {
      lookup->x = (si32 *) urealloc ((char *) lookup->x,
				     lookup->nx * sizeof(si32));
    }

    lookup->nx_alloc = lookup->nx;

  }

  if (lookup->ny > lookup->ny_alloc) {

    if (lookup->y == NULL) {
      lookup->y = (si32 *) umalloc (lookup->ny * sizeof(si32));
    } else {
      lookup->y = (si32 *) urealloc ((char *) lookup->y,
				     lookup->ny * sizeof(si32));
    }

    lookup->ny_alloc = lookup->ny;

  }

  return;

}

#endif
