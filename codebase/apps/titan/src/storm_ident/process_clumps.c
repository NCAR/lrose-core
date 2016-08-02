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
/*********************************************************************
 * process_clumps.c
 *
 * Checks storm props.
 * If necessary splits storms.
 * Passes clumps to props_compute for copmutations
 * of storm properties.
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#include <toolsa/file_io.h>

void process_clumps(vol_file_handle_t *v_handle,
		    storm_file_handle_t *s_handle,
		    si32 scan_num,
		    si32 nplanes,
		    si32 nclumps,
		    Clump_order *clumps)
     
{

  si32 iclump;
  si32 nstorms;
  si32 n_added;

  double clump_size; 
  double dvol_at_centroid;
  double darea_at_centroid;
  double darea_ellipse;
  double min_valid_z;

  storm_file_scan_header_t *scan_hdr;
  Clump_order *clump;

  /*
   * read in storm file header
   */
  
  if (RfReadStormHeader(s_handle, "process_clumps")) {
    tidy_and_exit(-1);
  }
  
  /*
   * Make sure there's memory allocated for the scan and storms,
   * and initialize
   */
  
  RfAllocStormScan(s_handle, nclumps, "process_clumps");
  memset ((void *) s_handle->gprops,
          (int) 0,
	  (size_t) (nclumps * sizeof(storm_file_global_props_t)));
  
  /*
   * set write lock on header file
   */
  
  if (ta_lock_file_procmap(s_handle->header_file_path,
			   s_handle->header_file, "w")) {
    fprintf(stderr, "ERROR - %s:process_clumps\n", Glob->prog_name);
    tidy_and_exit(-1);
  }

  /*
   * initialize the computation module for storm props
   */

  init_props_compute(v_handle, s_handle,
		     &min_valid_z);

  /*
   * loop through the clumps - index starts at 1
   */
  
  clump = clumps + 1;
  nstorms = 0;
  
  for (iclump = 0; iclump < nclumps; iclump++, clump++) {

    /*
     * check if clump volume exceeds min, otherwise
     * go to end of loop
     */

    vol_and_area_comps(clump, &clump_size, &dvol_at_centroid,
		       &darea_at_centroid, &darea_ellipse);
    
    if (clump_size >= Glob->params.min_storm_size &&
	clump_size <= Glob->params.max_storm_size) {
      
      n_added = process_this_clump(nstorms, clump,
				   v_handle, s_handle, nplanes,
				   clump_size, dvol_at_centroid,
				   darea_at_centroid, darea_ellipse);
      
      nstorms += n_added;
      
    }
    
  } /* iclump */
  
  /*
   * load up scan structure
   */
  
  scan_hdr = s_handle->scan;
  
  scan_hdr->nbytes_char = v_handle->vol_params->cart.nbytes_char;
  scan_hdr->scan_num = scan_num;
  scan_hdr->nstorms = nstorms;

  scan_hdr->time = Rfrtime2utime(&v_handle->vol_params->mid_time);
  
  scan_hdr->min_z = min_valid_z;
  scan_hdr->delta_z = Glob->delta_z;

  RfCartParams2TITANGrid(&v_handle->vol_params->cart,
			 &scan_hdr->grid, Glob->projection);

  /*
   * read in storm file header
   */

  if (RfReadStormHeader(s_handle, "process_clumps"))
    tidy_and_exit(-1);
  
  /*
   * write the scan header and global props
   */
  
  if (RfWriteStormScan(s_handle, scan_num,
		       "process_clumps"))
    tidy_and_exit(-1);
  
  /*
   * update the file header
   */
  
  if (scan_num == 0) {
    s_handle->header->start_time =
      Rfrtime2utime(&v_handle->vol_params->mid_time);
  }
  
  s_handle->header->end_time =
    Rfrtime2utime(&v_handle->vol_params->mid_time);
  
  /*
   * fill in number of scans, tracks and current file size
   */
  
  s_handle->header->n_scans = scan_num + 1;

  /*
   * rewrite header
   */
  
  if (RfWriteStormHeader(s_handle, "process_clumps"))
    tidy_and_exit(-1);
  
  if (RfFlushStormFiles(s_handle, "process_clumps"))
    tidy_and_exit(-1);
  
  /*
   * clear lock on header file
   */
  
  if (ta_unlock_file(s_handle->header_file_path,
		     s_handle->header_file)) {
    fprintf(stderr, "ERROR - %s:process_clumps\n", Glob->prog_name);
    tidy_and_exit(-1);
  }

  /*
   * printout
   */
  
  fprintf(stdout, "scan number %d\n", scan_num);
  fprintf(stdout, "nstorms : %d\n", nstorms);
  fprintf(stdout, "Time: %s\n",
	  utimstr(s_handle->header->end_time));
  
  /*
   * clear the semaphores and perform tracking if active
   */

  if (Glob->params.tracking) {

    if (scan_num == 0) {
      perform_tracking(PREPARE_NEW_FILE);
    } else {
      perform_tracking(TRACK_LAST_SCAN);
    }

  }

  return;
  
}

