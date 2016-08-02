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

#include "precip_map.h"

/**************************************************************************
 * verify_generate.c
 *
 * generate the map
 *
 * Returns time of last file written, -1 on error
 *
 * July 1993
 *
 **************************************************************************/

time_t verify_generate(storm_file_handle_t *s_handle,
		       vol_file_handle_t *v_handle,
		       vol_file_handle_t *map_v_handle,
		       date_time_t *scan_times,
		       si32 scan_num,
		       double lead_time)

{

  static double *dprecip = NULL;
  static double *scan_contrib_time = NULL;
  static ui08 *uprecip = NULL;
  static ui08 *comp = NULL;

  ui08 *dbz;

  char rdata_file_path[MAX_PATH_LEN];
  char map_file_dir[MAX_PATH_LEN];
  char map_file_path[MAX_PATH_LEN];

  si32 i;
  si32 jscan;
  si32 end_scan, n_scans;
  si32 n_verify_scans;
  si32 npoints;
  si32 min_search_time, max_search_time;

  double *dp;
  double precip_lookup[256];
  double contrib_time;
  double contribs_so_far;
  double coeff, expon;

  time_t time_this_scan, time_latest_scan;
  date_time_t *stime, *this_time, *prev_time, *next_time;
  cart_params_t cart;
  field_params_t *dbz_fparams, *precip_fparams;
  storm_file_params_t *sparams;

  sparams = &s_handle->header->params;
  time_latest_scan = scan_times[s_handle->header->n_scans - 1].unix_time;
  n_scans = s_handle->header->n_scans;

  stime = scan_times + scan_num;
  
  PMU_auto_register("In verify_generate");
  
  time_this_scan = stime->unix_time;
  min_search_time = time_this_scan + lead_time;
  max_search_time = min_search_time + Glob->params.scan_interval / 2.0;

  /*
   * return now if there is not enough data for a full
   * verification
   */
    
  if (min_search_time > time_latest_scan) {
    return (-1);
  }
  
  /*
   * get path name for map file
   */
  
  sprintf(map_file_dir,
	  "%s%s%.4ld%.2ld%.2ld",
	  Glob->params.map_dir, PATH_DELIM,
	  (long) stime->year, (long) stime->month, (long) stime->day);
  
  sprintf(map_file_path,
	  "%s%s%.2ld%.2ld%.2ld.%s",
	  map_file_dir, PATH_DELIM,
	  (long) stime->hour, (long) stime->min, (long) stime->sec,
	  Glob->params.output_file_ext);
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "\nScan %ld, time %s\n", (long) scan_num,
	    utimestr(stime));
    fprintf(stderr, "Map path: %s\n", map_file_path);
    fprintf(stderr, "Lead_time: %g\n", lead_time);
  } /* if (Glob->params.debug >= DEBUG_NORM) */
  
  /*
   * determine the number of scans ahead for the verification
   */
  
  this_time = stime + 1;
  
  for (jscan = scan_num + 1; jscan < n_scans; jscan++) {
      
    if (this_time->unix_time > max_search_time) {
      end_scan = jscan - 1;
      break;
    }
    
    this_time++;
    
  } /* jscan */
    
  n_verify_scans = end_scan - scan_num;

  if (scan_contrib_time == NULL) {
    scan_contrib_time = (double *) umalloc
      (n_verify_scans * sizeof(double));
  } else {
    scan_contrib_time = (double *) urealloc
      (scan_contrib_time, 
       n_verify_scans * sizeof(double));
  }
  
  /*
   * load up the scan_contrib_time array - this holds the time,
   * in hours, which is contributed by the precip from each scan
   */
  
  prev_time = stime;
  this_time = stime + 1;
  next_time = stime + 2;
  
  contribs_so_far = 0.0;
  
  for (jscan = 0; jscan < n_verify_scans - 1; jscan++) {
    
    scan_contrib_time[jscan] =
      (double) (next_time->unix_time -
		prev_time->unix_time) / 7200.0;
    
    contribs_so_far += scan_contrib_time[jscan];
    
    prev_time++;
    this_time++;
    next_time++;
    
  } /* jscan */
  
  scan_contrib_time[n_verify_scans - 1] =
    lead_time / 3600.0 - contribs_so_far;
  
  if (Glob->params.debug >= DEBUG_EXTRA) {
    
    fprintf(stderr, "n_verify_scans : %ld\n", (long) n_verify_scans);
    
    for (jscan = 0; jscan < n_verify_scans; jscan++)
      fprintf(stderr, "Scan #, contrib_time : %ld, %g\n",
	      (long) jscan,
	      scan_contrib_time[jscan]);
    
  } /* if (Glob->params.debug >= DEBUG_EXTRA) */
  
  /*
   * loop through the scans for this verification
   */
  
  this_time = stime + 1;
  
  for (jscan = 0; jscan < n_verify_scans; jscan++) {
    
    /*
     * compute the radar data file path
     */
    
    sprintf(rdata_file_path,
	    "%s%s%.4ld%.2ld%.2ld%s%.2ld%.2ld%.2ld.%s",
	    Glob->params.rdata_dir, PATH_DELIM,
	    (long) this_time->year, (long) this_time->month,
	    (long) this_time->day,
	    PATH_DELIM,
	    (long) this_time->hour, (long) this_time->min, 
	    (long) this_time->sec,
	    Glob->params.output_file_ext);
    
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "\n  Scan %ld, time %s\n", (long) (jscan + scan_num),
	      utimestr(this_time));
      fprintf(stderr, "  Rdata path: %s\n", rdata_file_path);
    } /* if (Glob->params.debug >= DEBUG_NORM) */
    
    /*
     * read in radar volume
     */
    
    v_handle->vol_file_path = rdata_file_path;
    
    if (RfReadVolume(v_handle, "verify_generate")) {
      fprintf(stderr, "ERROR - %s:verify_generate\n", Glob->prog_name);
      fprintf(stderr, "Could not read in Mdv radar volume\n");
      perror(rdata_file_path);
      return (-1);
    }

    if (jscan == 0) {

      /*
       * initialize cart params
       */
      
      cart = v_handle->vol_params->cart;
      npoints = cart.nx * cart.ny;
      
      /*
       * allocate precip grids
       */
      
      if (dprecip == NULL) {
	dprecip = (double *) umalloc
	  (npoints * sizeof(double));
      } else {
	dprecip = (double *) urealloc
	  (dprecip, npoints * sizeof(double));
      }
      memset(dprecip, 0, npoints * sizeof(double));
      
      if (uprecip == NULL) {
	uprecip = (ui08 *) umalloc (npoints * sizeof(ui08));
      } else {
	uprecip = (ui08 *) urealloc
	  (uprecip, npoints * sizeof(ui08));
      }
      memset(uprecip, 0, npoints * sizeof(ui08));
      
      if (comp == NULL) {
	comp = (ui08 *) umalloc (npoints * sizeof(ui08));
      } else {
	comp = (ui08 *) urealloc
	  (comp, npoints * sizeof(ui08));
      }
      memset(comp, 0, npoints * sizeof(ui08));

    } /* if (jscan == 0) */
    
    /*
     * get ZR params
     */
    
    if (Glob->params.get_zr_from_file) {
      if (RfGetZrClosest(Glob->params.zr_dir,
			 stime->unix_time, 7200,
			 &coeff, &expon)) {
	coeff = Glob->params.ZR.coeff;
	expon = Glob->params.ZR.expon;
      }
    } else {
      coeff = Glob->params.ZR.coeff;
      expon = Glob->params.ZR.expon;
    }
    
    dbz_fparams = v_handle->field_params[Glob->params.dbz_field];
    compute_precip_lookup(precip_lookup, dbz_fparams,
			  coeff, expon);
    
    /*
     * create composite reflectivity array
     */
    
    create_composite(npoints, cart_comp_base(&cart), cart_comp_top(&cart),
		     v_handle->field_plane[Glob->params.dbz_field],
		     comp);
  
    /*
     * increment precip array
     */
      
    dp = dprecip;
    dbz = comp;
    contrib_time = scan_contrib_time[jscan];
    
    for (i = 0; i < npoints; i++) {
      
      *dp += precip_lookup[*dbz] * contrib_time;
      dp++;
      dbz++;
      
    } /* i */
    
    this_time++;
    
  } /* jscan */
  
  /*
   * initialize map index
   */
  
  init_map_index(map_v_handle, v_handle, stime);
      
  /*
   * scale the precip array and copy into the first field
   * of the volume index
   */
  
  precip_fparams = map_v_handle->field_params[0];
  
  scale_data(dprecip,
	     uprecip,
	     npoints,
	     precip_fparams->factor,
	     &precip_fparams->scale,
	     &precip_fparams->bias);
  
  /*
   * write the map file
   */
  
  sprintf(map_v_handle->vol_params->note,
	  "%s\n%s%g\n%s%g\n%s%g\n",
	  "Precipitation verification",
	  "Refl threshold: ", s_handle->header->params.low_dbz_threshold,
	  "Z-R coeff : ", coeff,
	  "Z-R expon : ", expon);
  
  sprintf(precip_fparams->name,
	  "%g hr forecast ahead of time", lead_time / 3600.0);
  
  map_v_handle->field_plane[0][0] = uprecip;
  
  RfWriteDobson(map_v_handle, FALSE, Glob->params.debug,
		Glob->params.map_dir,
		Glob->params.output_file_ext,
		Glob->prog_name,
		"verify_generate");
  
  return (stime->unix_time);

}

