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
 * persistence_generate.c
 *
 * generate the map based on a persistence forecast
 *
 * Returns time of file written, -1 on error
 *
 * July 1993
 *
 **************************************************************************/

time_t persistence_generate(storm_file_handle_t *s_handle,
			    vol_file_handle_t *v_handle,
			    vol_file_handle_t *map_v_handle,
			    date_time_t *scan_times,
			    si32 scan_num,
			    double lead_time)

{

  static double *dprecip = NULL;
  static ui08 *uprecip = NULL;
  static ui08 *comp = NULL;

  ui08 *dbz;

  char rdata_file_path[MAX_PATH_LEN];
  char map_file_dir[MAX_PATH_LEN];
  char map_file_path[MAX_PATH_LEN];

  si32 i;
  si32 n_scans;
  si32 npoints;

  double *dp;
  double precip_lookup[256];
  double contrib_time;
  double coeff, expon;

  cart_params_t cart;
  field_params_t *dbz_fparams, *precip_fparams;
  storm_file_params_t *sparams;
  date_time_t *stime, ftime;

  sparams = &s_handle->header->params;
  n_scans = s_handle->header->n_scans;
  
  stime = scan_times + scan_num;

  PMU_auto_register("In persistence_generate");

    /*
     * get path name for map file
     */

  ftime.unix_time = stime->unix_time;
    
  uconvert_from_utime(&ftime);
    
  sprintf(map_file_dir,
	  "%s%s%.4ld%.2ld%.2ld",
	  Glob->params.map_dir, PATH_DELIM,
	  (long) ftime.year, (long) ftime.month, (long) ftime.day);

  sprintf(map_file_path,
	  "%s%s%.2ld%.2ld%.2ld.%s",
	  map_file_dir, PATH_DELIM,
	  (long) ftime.hour, (long) ftime.min, (long) ftime.sec,
	  Glob->params.output_file_ext);

  /*
   * compute the radar data file path
   */
    
  sprintf(rdata_file_path,
	  "%s%s%.4ld%.2ld%.2ld%s%.2ld%.2ld%.2ld.%s",
	  Glob->params.rdata_dir, PATH_DELIM,
	  (long) stime->year, (long) stime->month,
	  (long) stime->day,
	  PATH_DELIM,
	  (long) stime->hour, (long) stime->min,
	  (long) stime->sec,
	  Glob->params.output_file_ext);
      
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "\nScan %ld, time %s\n", (long) scan_num,
	    utimestr(stime));
    fprintf(stderr, "Rdata path: %s\n", rdata_file_path);
    fprintf(stderr, "Map path: %s\n", map_file_path);
    fprintf(stderr, "Lead_time: %g\n", lead_time);
  } /* if (Glob->params.debug >= DEBUG_NORM) */

  /*
   * read in radar volume
   */

  v_handle->vol_file_path = rdata_file_path;

  if (RfReadVolume(v_handle, "persistence_generate")) {
    fprintf(stderr, "ERROR - %s:persistence_generate\n", Glob->prog_name);
    fprintf(stderr, "Could not read in Mdv radar volume\n");
    perror(rdata_file_path);
    return (-1);
  }
      
  cart = v_handle->vol_params->cart;
  npoints = cart.nx * cart.ny;
	
  /*
   * allocate grids
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
    
  init_map_index(map_v_handle, v_handle, &ftime);

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
   * load precip array
   */
  
  dp = dprecip;
  dbz = comp;
  contrib_time = lead_time / 3600.0;

  for (i = 0; i < npoints; i++) {
      
    *dp += precip_lookup[*dbz] * contrib_time;
    dp++;
    dbz++;
      
  } /* i */
      
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
	  "Precipitation forecast - persistence",
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
		"forecast_generate");
    
  return (ftime.unix_time);

}


