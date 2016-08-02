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
/*******************************************************************************
 * load_storm_props.c
 *
 * loads the storms props struct and arrays
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * December 1995
 *
 *******************************************************************************/

#include "storm_track.h"

static void alloc_proj_runs(storm_status_t *storm);
     
void load_storm_props(storm_file_handle_t *s_handle,
		      si32 nstorms,
		      storm_status_t *storms,
		      date_time_t *scan_time)
     
{
 
  int istorm, j;
  storm_track_props_t *current;
  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  storm_status_t *storm;
  
  /*
   * set local variables
   */
  
  sparams = &s_handle->header->params;

  for (istorm = 0; istorm < nstorms; istorm++) {

    storm = storms + istorm;
    gprops = s_handle->gprops + istorm;
    
    /*
     * load props struct
     */
    
    current = &storm->current;

    current->time = *scan_time;

    current->proj_area_centroid_x = gprops->proj_area_centroid_x;
    current->proj_area_centroid_y = gprops->proj_area_centroid_y;
    current->vol_centroid_z = gprops->vol_centroid_z;
    current->refl_centroid_z = gprops->refl_centroid_z;
    current->top = gprops->top;
    
    current->dbz_max = gprops->dbz_max;
    current->volume = gprops->volume;
    current->precip_flux = gprops->precip_flux;
    current->mass = gprops->mass;
    current->proj_area = gprops->proj_area;

    for (j = 0; j < N_POLY_SIDES; j++) {
      storm->current.proj_area_rays[j] = gprops->proj_area_polygon[j];
    }
    
    current->bound.min_ix = gprops->bounding_min_ix;
    current->bound.min_iy = gprops->bounding_min_iy;
    current->bound.max_ix = gprops->bounding_max_ix;
    current->bound.max_iy = gprops->bounding_max_iy;

    storm->n_proj_runs = gprops->n_proj_runs;

    if (storm->n_proj_runs > 0) {
      
      /*
       * read proj runs
       */
      
      if (RfReadStormProjRuns(s_handle, istorm, "load_storm_props")
	  != R_SUCCESS) {
	storm->n_proj_runs = 0;
	return;
      }
      
      /*
       * copy over to storm array
       */
      
      alloc_proj_runs(storm);

      memcpy((void *) storm->proj_runs,
	     (void *) s_handle->proj_runs,
	     storm->n_proj_runs * sizeof(storm_file_run_t));

    } else {

      Glob->params.use_runs_for_overlaps = FALSE;

    } /* if (storm->n_proj_runs > 0) */
      
  } /* i */

}

static void alloc_proj_runs(storm_status_t *storm)
     
{

  if (storm->n_proj_runs > storm->n_proj_runs_alloc) {
    if (storm->proj_runs == NULL) {
      storm->proj_runs = (storm_file_run_t *)
	umalloc(storm->n_proj_runs * sizeof(storm_file_run_t));
    } else {
      storm->proj_runs = (storm_file_run_t *)
	urealloc((char *) storm->proj_runs,
		 storm->n_proj_runs * sizeof(storm_file_run_t));
    }
    storm->n_proj_runs_alloc = storm->n_proj_runs;
  }

  return;

}



