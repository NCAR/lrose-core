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
/************************************************************************
 * load_plot_data.c
 *
 * loads the track data for plotting and compute stats
 *
 * Returns 1 if no tracks, 0 otherwise
 *
 * time_hist routine
 * 
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"
#include <titan/tdata_partial_track.h>
#include <physics/vil.h>

static int Partial_found;
static si32 Nscans, Start_scan;
static si32 Start_time, End_time;
static thist_track_data_t *Tdata;
static time_hist_shmem_t *Tshmem;
static complex_track_params_t *Ct_params;
static tdata_complete_index_t *Td_index;
static storm_file_params_t *Sparams;
static track_file_params_t *Tparams;
static tdata_partial_track_t Partial;

static int entry_valid(track_file_entry_t *entry);
static void load_scan_nums(void);
static void load_basic_data(void);
static void load_thist_data(void);
static void load_timeht_data(void);
static void load_rdist_data(void);

int load_plot_data(void)

{

  static int first_call = TRUE;

  if (Glob->debug) {
    fprintf(stderr, "** load_plot_data **\n");
  }

  /*
   * return error if no tracks found in data
   */

  if (Glob->track_shmem->complex_track_num < 0)
    return (-1);

  /*
   * return now if all plots are current
   */

  if (Glob->thist_status == CURRENT &&
      Glob->timeht_status == CURRENT &&
      Glob->rdist_status == CURRENT)
    return (0);

  /*
   * set pointers and file scope variables
   */
  
  Td_index = &Glob->tdata_index.complete;
  Tdata = &Glob->tdata;
  Tshmem = Glob->track_shmem;
  Sparams = &Td_index->header.sparams;
  Tparams = &Td_index->header.tparams;
  Ct_params = Td_index->complex_params + Glob->complex_index;

  /*
   * initialize partial track struct
   */
  
  if (first_call) {

    tdata_init_partial_track(&Partial,
			     Glob->prog_name, Glob->debug);
    first_call = FALSE;

  }
  
  /*
   * find the partial track if required
   */
  
  if (Tshmem->track_type == PARTIAL_TRACK) {

    if (tdata_find_partial_track(&Partial,
				 Tshmem->partial_track_ref_time,
				 Tshmem->partial_track_past_period,
				 Tshmem->partial_track_future_period,
				 Tshmem->complex_track_num,
				 Tshmem->simple_track_num,
				 &Glob->tdata_index.complete)) {
      Partial_found = FALSE;
    } else {
      Partial_found = TRUE;
    }

  }

  load_scan_nums();

  /*
   * if there is new track data, get times and other basic data
   */

  if (Glob->plot_data_status == NOT_CURRENT) {
    
    load_basic_data();
    Glob->plot_data_status = CURRENT;

  } /* if (Glob->plot_data_status == NOT_CURRENT) */

  /*
   * get time history graph data
   */

  if (Glob->thist_status == NEEDS_DATA) {

    load_thist_data();
    Glob->thist_status = NEEDS_EXPOSE;

  }

  /*
   * get time height profile data
   */

  if (Glob->timeht_status == NEEDS_DATA) {

    load_timeht_data();
    Glob->timeht_status = NEEDS_EXPOSE;

  }

  /*
   * get reflectivity distribution data
   */

  if (Glob->rdist_status == NEEDS_DATA) {

    load_rdist_data();
    Glob->rdist_status = NEEDS_EXPOSE;

  }

  return (0);

}

/*******************************************************************
 * load_scan_nums()
 */

static void load_scan_nums(void)

{
  
  int isimple, ientry;
  si32 end_scan;
  simple_track_params_t *st_params;
  track_file_entry_t *entry;
  tdata_complete_track_entry_t *track_entry;

  if (Tshmem->track_type == COMPLEX_TRACK) {

    Nscans = Ct_params->duration_in_scans;
    Start_scan = Ct_params->start_scan;
    Start_time = Ct_params->start_time;
    End_time = Ct_params->end_time;

  } else if (Tshmem->track_type == SIMPLE_TRACK) {

    st_params = (Td_index->simple_params[Glob->complex_index] +
		 Glob->simple_index);
    Start_scan = st_params->start_scan;
    Nscans = st_params->duration_in_scans;
    Start_time = st_params->start_time;
    End_time = st_params->end_time;

  } else if (Tshmem->track_type == PARTIAL_TRACK) {
    
    if (Partial_found) {

      Start_scan = 1000000;
      end_scan = -1;
      
      for (isimple = 0;
	   isimple < Ct_params->n_simple_tracks; isimple++) {
    
	st_params = Td_index->simple_params[Glob->complex_index] + isimple;
      
	for (ientry = 0;
	     ientry < st_params->duration_in_scans; ientry++) {
	  
	  track_entry =
	    Td_index->track_entry[Glob->complex_index][isimple] + ientry;

	  entry = &track_entry->entry;
	  
	  if (tdata_entry_in_partial(&Partial, entry)) {

	    if (entry->scan_num < Start_scan) {
	      Start_scan = entry->scan_num;
	      Start_time = entry->time;
	    }

	    if (entry->scan_num > end_scan) {
	      end_scan = entry->scan_num;
	      End_time = entry->time;
	    }
	    
	  }
	  
	} /* ientry */
	
      } /* isimple */

      Nscans = end_scan - Start_scan + 1;

    } else {

      Nscans = 0;
      Start_scan = 0;
      Start_time = Ct_params->start_time;
      End_time = Ct_params->end_time;

    } /* if (Partial_found) */

  }

}

/*******************************************************************
 * entry_valid()
 *
 * returns TRUE if this entry is valid for the track
 * type selected.
 */

static int entry_valid(track_file_entry_t *entry)

{

  /*
   * complex track?
   */

  if (Tshmem->track_type == COMPLEX_TRACK) {
    return (TRUE);
  }

  /*
   * simple track?
   */

  if (Tshmem->track_type == SIMPLE_TRACK) {
    if (entry->simple_track_num == Tshmem->simple_track_num) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  /*
   * partial track
   */

  if (tdata_entry_in_partial(&Partial, entry)) {
    return (TRUE);
  } else {
    return (FALSE);
  }

}
  
/*******************************************************************
 * load_basic_data()
 */

static void load_basic_data(void)

{
  
  static int first_call = TRUE;
  static si32 nscans_allocated;
  
  si32 iscan;
  si32 isimple, ientry;
  si32 time_range;
  
  double mean_dt;
  double volume;
  double top, base;
  double min_z, max_z, delta_z;
  double dbz_max, ht_of_dbz_max;
  
  simple_track_params_t *st_params;
  
  storm_file_scan_header_t *scan_hdr;
  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;
  tdata_complete_track_entry_t *track_entry;
  
  time_range = End_time - Start_time;
  if (Nscans > 1)
    mean_dt = (double) time_range / (double) (Nscans - 1);
  else
    mean_dt = 1.0;

  Tdata->plot_start_time = Start_time - (si32) (mean_dt / 2.0);
  Tdata->plot_end_time = End_time + (si32) (mean_dt / 2.0);
  Tdata->nscans = Nscans;
  Tdata->scan_duration = (si32) (mean_dt + 0.5);

  /*
   * if first call, allocate memory. Otherwise, reallocate.
   */
  
  if (first_call) {
    
    Tdata->scan = (thist_scan_data_t *)
      umalloc((ui32) (Nscans * sizeof(thist_scan_data_t)));
    
    nscans_allocated = Nscans;
    
    first_call = FALSE;
    
  } else { 
    
    for (iscan = 0; iscan < nscans_allocated; iscan++) {
      
      ufree((char *) Tdata->scan[iscan].timeht_data);
      ufree((char *) Tdata->scan[iscan].timeht_flag);
      ufree((char *) Tdata->scan[iscan].rdist_data);
      ufree((char *) Tdata->scan[iscan].rdist_flag);
      
    }
    
    Tdata->scan = (thist_scan_data_t *)
      urealloc((char *) Tdata->scan,
	       (ui32) (Nscans * sizeof(thist_scan_data_t)));

    nscans_allocated = Nscans;

  } /* if (first_call) */
  
  /*
   * initialize
   */
  
  Tdata->max_dbz_intervals = 0;
  Tdata->max_layers = 0;
  Tdata->max_top = 0;
  Tdata->min_base = LARGE_DOUBLE;
  
  memset ((void *)  Tdata->scan,
          (int) 0, (size_t) (Nscans * sizeof(thist_scan_data_t)));
  
  for (iscan = 0; iscan < Nscans; iscan++) {
    
    Tdata->scan[iscan].top = 0;
    Tdata->scan[iscan].dbz_max = -LARGE_DOUBLE;
    Tdata->scan[iscan].base = LARGE_DOUBLE;
    Tdata->scan[iscan].min_z = LARGE_DOUBLE;
    Tdata->scan[iscan].delta_z = LARGE_DOUBLE;
    
  } /* iscan */
  
  Tdata->dbz_hist_interval = Sparams->dbz_hist_interval;
  Tdata->dbz_threshold = Sparams->low_dbz_threshold;
  
  /*
   * loop through track entries
   */
  
  for (isimple = 0;
       isimple < Ct_params->n_simple_tracks; isimple++) {
    
    st_params = Td_index->simple_params[Glob->complex_index] + isimple;
      
    for (ientry = 0;
	 ientry < st_params->duration_in_scans; ientry++) {
	
      track_entry =
	Td_index->track_entry[Glob->complex_index][isimple] + ientry;
      entry = &track_entry->entry;

      if (entry_valid(entry)) {

	gprops = &track_entry->gprops;
	scan_hdr = &track_entry->scan;
	iscan = entry->scan_num - Start_scan;

	Tdata->scan[iscan].time = entry->time;
	
	volume = gprops->volume;
	Tdata->scan[iscan].volume += volume;
	
	delta_z = scan_hdr->delta_z;
	min_z = scan_hdr->min_z;
	max_z = min_z +
	  (gprops->base_layer + gprops->n_layers - 1.0) * delta_z;
	
	Tdata->scan[iscan].delta_z =
	  MIN(delta_z, Tdata->scan[iscan].delta_z);
	
	Tdata->scan[iscan].min_z =
	  MIN(Tdata->scan[iscan].min_z, min_z);
	
	Tdata->scan[iscan].max_z =
	  MAX(Tdata->scan[iscan].max_z, max_z);
	
	dbz_max = gprops->dbz_max;
	
	if (dbz_max > Tdata->scan[iscan].dbz_max) {

	  ht_of_dbz_max = gprops->ht_of_dbz_max;
	  Tdata->scan[iscan].ht_of_dbz_max = ht_of_dbz_max;
	  Tdata->scan[iscan].dbz_max = dbz_max;

	}

	Tdata->scan[iscan].vol_centroid_z +=
	  gprops->vol_centroid_z * volume;
	
	Tdata->scan[iscan].refl_centroid_z +=
	  gprops->refl_centroid_z * volume;
	
	top = gprops->top;
	Tdata->scan[iscan].top =
	  MAX(top, Tdata->scan[iscan].top);
	Tdata->max_top = MAX(top, Tdata->max_top);

	base = gprops->base;
	Tdata->scan[iscan].base =
	  MIN(base, Tdata->scan[iscan].base);
	Tdata->min_base = MIN(base, Tdata->min_base);
	
	Tdata->scan[iscan].n_dbz_intervals =
	  MAX(Tdata->scan[iscan].n_dbz_intervals,
	      gprops->n_dbz_intervals);

      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  for (iscan = 0; iscan < Nscans; iscan++) {
    
    /*
     * compute number of layers - this is done in this way
     * because the delta_z value may vary, and the delta_z value
     * used is the minimum one.
     */
    
    Tdata->scan[iscan].n_layers =
      (si32) ((Tdata->scan[iscan].max_z - Tdata->scan[iscan].min_z) /
	      Tdata->scan[iscan].delta_z + 1.5);
    
    Tdata->max_layers = MAX(Tdata->max_layers,
			    Tdata->scan[iscan].n_layers);
    
    Tdata->max_dbz_intervals =
      MAX(Tdata->max_dbz_intervals, 
	  Tdata->scan[iscan].n_dbz_intervals);
    
    /*
     * compute centroid values - they are volume weighted
     */
    
    Tdata->scan[iscan].vol_centroid_z /= Tdata->scan[iscan].volume;
    Tdata->scan[iscan].refl_centroid_z /= Tdata->scan[iscan].volume;
    
  } /* iscan */
  
}

/*******************************************************************
 * load_thist_data()
 */

static void load_thist_data(void)

{

  si32 iscan, isimple, ientry, ifield, ilayer;
  si32 forecast_end_time;
  double forecast_lead_time;
  double current_val, forecast_val;
  storm_file_scan_header_t *scan_hdr;
  storm_file_global_props_t *gprops;
  storm_file_layer_props_t *lprops;
  track_file_entry_t *entry;
  tdata_complete_track_entry_t *track_entry;
  simple_track_params_t *st_params;
  track_file_forecast_props_t *fprops;
  
  /*
   * initialize
   */
  
  for (iscan = 0; iscan < Nscans; iscan++) {
    
    memset(Tdata->scan[iscan].thist_data, 0,
	   THIST_N_FIELDS * sizeof(double));
    memset(Tdata->scan[iscan].thist_dval_dt, 0,
	   THIST_N_FIELDS * sizeof(double));
    
  } /* iscan */

  /*
   * set label and unit strings
   */
  
  Tdata->thist_label[THIST_VOL_FIELD] = "Vol";
  Tdata->thist_units[THIST_VOL_FIELD] = "km3";
  Tdata->thist_label[THIST_AREA_FIELD] = "Area";
  Tdata->thist_units[THIST_AREA_FIELD] = "km2";
  Tdata->thist_label[THIST_PFLUX_FIELD] = "Pflux";
  Tdata->thist_units[THIST_PFLUX_FIELD] = "m3/s";
  Tdata->thist_label[THIST_MASS_FIELD] = "Mass";
  Tdata->thist_units[THIST_MASS_FIELD] = "ktons";
  Tdata->thist_label[THIST_VIL_FIELD] = "Vil";
  Tdata->thist_units[THIST_VIL_FIELD] = "kg/m2";

  /*
   * loop through track entries
   */
  
  for (isimple = 0;
       isimple < Ct_params->n_simple_tracks; isimple++) {
    
    st_params = Td_index->simple_params[Glob->complex_index] + isimple;
      
    for (ientry = 0;
	 ientry < st_params->duration_in_scans; ientry++) {
	
      track_entry =
	Td_index->track_entry[Glob->complex_index][isimple] + ientry;
      entry = &track_entry->entry;

      if (entry_valid(entry)) {

	scan_hdr = &track_entry->scan;
	gprops = &track_entry->gprops;
	fprops = &entry->dval_dt;
	iscan = entry->scan_num - Start_scan;
	
	Tdata->scan[iscan].thist_data[THIST_VOL_FIELD] +=
	  gprops->volume;
	Tdata->scan[iscan].thist_dval_dt[THIST_VOL_FIELD] +=
	  fprops->volume / 3600.0;
	
	Tdata->scan[iscan].thist_data[THIST_AREA_FIELD] +=
	  gprops->proj_area;
	Tdata->scan[iscan].thist_dval_dt[THIST_AREA_FIELD] +=
	  fprops->proj_area / 3600.0;
	
	Tdata->scan[iscan].thist_data[THIST_PFLUX_FIELD] +=
	  gprops->precip_flux;
	Tdata->scan[iscan].thist_dval_dt[THIST_PFLUX_FIELD] +=
	  fprops->precip_flux / 3600.0;
	  
	Tdata->scan[iscan].thist_data[THIST_MASS_FIELD] +=
	  gprops->mass;
	Tdata->scan[iscan].thist_dval_dt[THIST_MASS_FIELD] +=
	  fprops->mass / 3600.0;

	/*
	 * vil
	 */
	
	lprops = track_entry->lprops;
	vil_init();
	for (ilayer = 0;
	     ilayer < gprops->n_layers; ilayer++, lprops++) {
	  vil_add(lprops->dbz_max, scan_hdr->delta_z);
	}
	Tdata->scan[iscan].thist_data[THIST_VIL_FIELD] +=
	  vil_compute();

      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute max and min values
   */
  
  forecast_lead_time = (Tshmem->n_forecast_steps *
			Tshmem->forecast_interval);

  Tdata->forecast_end_time = Tdata->plot_end_time;

  for (ifield = 0; ifield < THIST_N_FIELDS; ifield++) {

    double min_val = LARGE_DOUBLE;
    double max_val = -LARGE_DOUBLE;

    for (iscan = 0; iscan < Nscans; iscan++) {
      
      current_val = Tdata->scan[iscan].thist_data[ifield];
      
      forecast_val = current_val +
	Tdata->scan[iscan].thist_dval_dt[ifield] * forecast_lead_time;
      
      min_val = MIN(min_val, current_val);
      max_val = MAX(max_val, current_val);
      
      if (Glob->thist_forecast == SELECTED_ALL ||
	  (Glob->thist_forecast == SELECTED_LIMITED && 
	   Tdata->scan[iscan].time == Glob->track_shmem->time)) {
	
	min_val = MIN(min_val, forecast_val);
	max_val = MAX(max_val, forecast_val);
	
	forecast_end_time = (Tdata->scan[iscan].time +
			     (Tshmem->n_forecast_steps *
			      Tshmem->forecast_interval));
	
	Tdata->forecast_end_time =
	  MAX(Tdata->forecast_end_time, forecast_end_time);
	
      } /* if (Glob->thist_forecast == SELECTED_ALL ... */
      
    } /* iscan */

    Tdata->min_thist_val[ifield] = min_val;
    Tdata->max_thist_val[ifield] = max_val;

  } /* ifield */
  
}

/*******************************************************************
 * load_timeht_data()
 */

static void load_timeht_data(void)

{

  si32 iscan, isimple, ientry, ilayer, jlayer;

  double volume;
  double mass, sum_mass;
  double z, min_z, delta_z;

  storm_file_global_props_t *gprops;
  storm_file_scan_header_t *scan_hdr;
  storm_file_layer_props_t *lprops;
  track_file_entry_t *entry;
  tdata_complete_track_entry_t *track_entry;
  simple_track_params_t *st_params;

  /*
   * allocate memory for timeht data array
   */
  
  for (iscan = 0; iscan < Nscans; iscan++) {

    Tdata->scan[iscan].timeht_data = (double *)
      ucalloc ((ui32) Tdata->max_layers,
	       (ui32) sizeof(double));

    Tdata->scan[iscan].timeht_flag = (int *)
      ucalloc ((ui32) Tdata->max_layers,
	       (ui32) sizeof(int));

  }
  
  /*
   * set labels
   */
  
  switch (Glob->timeht_mode) {

  case FALSE:
    Tdata->timeht_label = "";
    break;
    
  case TIMEHT_MAXZ:
    
    Tdata->timeht_label = "Max dbz";
    break;
    
  case TIMEHT_MEANZ:
    
    Tdata->timeht_label = "Mean dbz";
    break;
    
  case TIMEHT_MASS:
    
    Tdata->timeht_label = "% mass";
    break;
    
  case TIMEHT_VORTICITY:
    
    Tdata->timeht_label = "Vort.";
    break;
    
  } /* switch */
  
  /*
   * loop through track entries
   */
  
  for (isimple = 0;
       isimple < Ct_params->n_simple_tracks; isimple++) {
    
    st_params = Td_index->simple_params[Glob->complex_index] + isimple;

    for (ientry = 0;
	 ientry < st_params->duration_in_scans; ientry++) {
      
      track_entry =
	Td_index->track_entry[Glob->complex_index][isimple] + ientry;
      entry = &track_entry->entry;

      if (entry_valid(entry)) {

	gprops = &track_entry->gprops;
	scan_hdr = &track_entry->scan;
	iscan = entry->scan_num - Start_scan;

	volume = gprops->volume;
	min_z = scan_hdr->min_z;
	delta_z = scan_hdr->delta_z;
	
	lprops = track_entry->lprops;
	  
	for (ilayer = 0;
	     ilayer < gprops->n_layers; ilayer++, lprops++) {
	  
	  z = min_z + (ilayer + gprops->base_layer) * delta_z;
	  
	  jlayer = (si32) ((z - Tdata->scan[iscan].min_z) /
			   Tdata->scan[iscan].delta_z + 0.5);

	  /*
	   * set flag to indicate that there is data at this level
	   */

	  Tdata->scan[iscan].timeht_flag[jlayer] = TRUE;

	  /*
	   * load data 
	   */

	  switch (Glob->timeht_mode) {
	    
	  case TIMEHT_MAXZ:
	    
	    Tdata->scan[iscan].timeht_data[jlayer] =
	      MAX(Tdata->scan[iscan].timeht_data[jlayer],
		  lprops->dbz_max);
	    break;
	    
	  case TIMEHT_MEANZ:
	    
	    Tdata->scan[iscan].timeht_data[jlayer] +=
	      (lprops->dbz_mean * volume);
	    break;
	    
	  case TIMEHT_MASS:

	    mass = lprops->mass;
	    Tdata->scan[iscan].timeht_data[jlayer] += mass * volume;
	    break;
	    
	  case TIMEHT_VORTICITY:
	    
	    Tdata->scan[iscan].timeht_data[jlayer] +=
	      (lprops->vorticity * volume);
	    break;
	    
	  } /* switch */
	  
	} /* ilayer */
	
      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute volume-weighted means
   */
  
  switch (Glob->timeht_mode) {
    
  case TIMEHT_MAXZ:
    break;
    
  case TIMEHT_MEANZ:
  case TIMEHT_VORTICITY:
    
    for (iscan = 0; iscan < Nscans; iscan++) {
      
      for (ilayer = 0; ilayer < Tdata->max_layers; ilayer++) {
	
	Tdata->scan[iscan].timeht_data[ilayer] /=
	  Tdata->scan[iscan].volume;
	
      } /* ilayer */
      
    } /* iscan */

    break;
    
  case TIMEHT_MASS:
    
    for (iscan = 0; iscan < Nscans; iscan++) {
    
      sum_mass = 0.0;
      
      for (ilayer = 0; ilayer < Tdata->max_layers; ilayer++) {
	
	Tdata->scan[iscan].timeht_data[ilayer] /=
	  Tdata->scan[iscan].volume;

	sum_mass += Tdata->scan[iscan].timeht_data[ilayer];
	
      } /* ilayer */
      
      for (ilayer = 0; ilayer < Tdata->max_layers; ilayer++) {
	
	Tdata->scan[iscan].timeht_data[ilayer] *=
	  (100.0 / sum_mass);

      } /* ilayer */
      
    } /* iscan */

    break;
    
  } /* switch */
  
}

/*******************************************************************
 * load_rdist_data()
 */

static void load_rdist_data(void)

{
  
  si32 iscan, interval;
  si32 isimple, ientry;

  double volume;

  simple_track_params_t *st_params;
  storm_file_global_props_t *gprops;
  storm_file_dbz_hist_t *hist;
  track_file_entry_t *entry;

  tdata_complete_track_entry_t *track_entry;

  /*
   * allocate memory for rdist data array
   */
  
  for (iscan = 0; iscan < Nscans; iscan++) {

    Tdata->scan[iscan].rdist_data = (double *)
      ucalloc ((ui32) Tdata->max_dbz_intervals,
	       (ui32) sizeof(double));

    Tdata->scan[iscan].rdist_flag = (int *)
      ucalloc ((ui32) Tdata->max_dbz_intervals,
	       (ui32) sizeof(int));

  }
  
  /*
   * set labels
   */
  
  switch (Glob->rdist_mode) {
    
  case RDIST_VOL:
    
    Tdata->rdist_label = "% vol";
    break;
    
  case RDIST_AREA:
    
    Tdata->rdist_label = "% area";
    break;
    
  } /* switch */
  
  /*
   * loop through track entries
   */
  
  for (isimple = 0;
       isimple < Ct_params->n_simple_tracks; isimple++) {
    
    st_params = Td_index->simple_params[Glob->complex_index] + isimple;
      
    for (ientry = 0;
	 ientry < st_params->duration_in_scans; ientry++) {
	
      track_entry =
	Td_index->track_entry[Glob->complex_index][isimple] + ientry;
      entry = &track_entry->entry;

      if (entry_valid(entry)) {

	gprops = &track_entry->gprops;
	iscan = entry->scan_num - Start_scan;
	volume = gprops->volume;
	
	hist = track_entry->hist;
	  
	for (interval = 0;
	     interval < gprops->n_dbz_intervals; interval++, hist++) {
	  
	  /*
	   * set flag to indicate that there is data in this interval
	   */

	  Tdata->scan[iscan].rdist_flag[interval] = TRUE;

	  switch (Glob->rdist_mode) {
	    
	  case RDIST_VOL:
	    
	    Tdata->scan[iscan].rdist_data[interval] +=
	      hist->percent_volume * volume;
	    break;
	    
	  case RDIST_AREA:
	    
	    Tdata->scan[iscan].rdist_data[interval] +=
	      hist->percent_area * volume;
	    break;
	    
	  } /* switch */
	  
	} /* interval */
	
      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute volume-weighted means
   */
  
  for (iscan = 0; iscan < Nscans; iscan++) {
    
    for (interval = 0;
	 interval < Tdata->max_dbz_intervals; interval++) {
      
      Tdata->scan[iscan].rdist_data[interval] /=
	Tdata->scan[iscan].volume;
      
    } /* interval */
    
  } /* iscan */
  
}
