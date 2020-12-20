// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/************************************************************************
 * load_plot_data.c
 *
 * loads the DsTitanServer data for plotting and compute stats
 *
 * Returns 1 if no tracks, 0 otherwise
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Feb 2001
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
#include <titan/TitanPartialTrack.hh>
#include <physics/vil.h>
using namespace std;

static int Nscans, Start_scan;
static time_t Start_time, End_time;

static int entry_valid(const track_file_entry_t *entry,
		       const TitanPartialTrack *partial);
static void load_scan_nums(const TitanPartialTrack *partial);
static void load_basic_data(const TitanPartialTrack *partial);
static void load_thist_data(const TitanPartialTrack *partial);
static void load_timeht_data(const TitanPartialTrack *partial);
static void load_rdist_data(const TitanPartialTrack *partial);
static void load_union_data(const TitanPartialTrack *partial);

int load_plot_data()

{

  if (Glob->verbose) {
    fprintf(stderr, "** load_plot_data_titan **\n");
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
  
  /*
   * find the partial track if required
   */
  
  TitanPartialTrack *partial = NULL;
  if (Glob->track_shmem->track_type == PARTIAL_TRACK) {
    partial = new TitanPartialTrack(Glob->debug);
    if (partial->identify(Glob->track_shmem->partial_track_ref_time,
			  Glob->track_shmem->partial_track_past_period,
			  Glob->track_shmem->partial_track_future_period,
			  Glob->track_shmem->complex_track_num,
			  Glob->track_shmem->simple_track_num,
			  Glob->_dsTitan)) {
      delete partial;
      partial = NULL;
    }
  }

  load_scan_nums(partial);

  /*
   * if there is new track data, get times and other basic data
   */

  if (Glob->plot_data_status == NOT_CURRENT) {
    
    load_basic_data(partial);
    Glob->plot_data_status = CURRENT;

  } /* if (Glob->plot_data_status == NOT_CURRENT) */

  /*
   * get time history graph data
   */

  if (Glob->thist_status == NEEDS_DATA) {

    load_thist_data(partial);
    Glob->thist_status = NEEDS_EXPOSE;

  }

  /*
   * get time height profile data
   */

  if (Glob->timeht_status == NEEDS_DATA) {

    load_timeht_data(partial);
    Glob->timeht_status = NEEDS_EXPOSE;

  }

  /*
   * get reflectivity distribution data
   */

  if (Glob->rdist_status == NEEDS_DATA) {

    load_rdist_data(partial);
    Glob->rdist_status = NEEDS_EXPOSE;

  }

  /*
   * get union sata
   */

  if (Glob->union_status == NEEDS_DATA) {

    load_union_data(partial);
    Glob->union_status = NEEDS_EXPOSE;

  }

  // free up

  if (partial) {
    delete partial;
  }

  return (0);

}

/*******************************************************************
 * load_scan_nums()
 */

static void load_scan_nums(const TitanPartialTrack *partial)

{
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];
  
  if (Glob->track_shmem->track_type == COMPLEX_TRACK) {

    Nscans = ctrack->complex_params().duration_in_scans;
    Start_scan = ctrack->complex_params().start_scan;
    Start_time = ctrack->complex_params().start_time;
    End_time = ctrack->complex_params().end_time;

  } else if (Glob->track_shmem->track_type == SIMPLE_TRACK) {

    const TitanSimpleTrack *strack =
      ctrack->simple_tracks()[Glob->simple_index];
    
    Start_scan = strack->simple_params().start_scan;
    Nscans = strack->simple_params().duration_in_scans;
    Start_time = strack->simple_params().start_time;
    End_time = strack->simple_params().end_time;

  } else if (Glob->track_shmem->track_type == PARTIAL_TRACK) {
    
    if (partial) {

      Start_scan = 1000000;
      int end_scan = -1;
      
      for (size_t isimple = 0;
	   isimple < ctrack->simple_tracks().size(); isimple++) {
    
	const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
	int duration = strack->entries().size();
    
	for (int ientry = 0; ientry < duration; ientry++) {
	  
	  const TitanTrackEntry *tentry = strack->entries()[ientry];
	  const track_file_entry_t &entry = tentry->entry();
	  
	  if (partial->entryIncluded(entry)) {

	    if (entry.scan_num < Start_scan) {
	      Start_scan = entry.scan_num;
	      Start_time = entry.time;
	    }

	    if (entry.scan_num > end_scan) {
	      end_scan = entry.scan_num;
	      End_time = entry.time;
	    }
	    
	  }
	  
	} /* ientry */
	
      } /* isimple */

      Nscans = end_scan - Start_scan + 1;

    } else {

      Nscans = 0;
      Start_scan = 0;
      Start_time = ctrack->complex_params().start_time;
      End_time = ctrack->complex_params().end_time;

    } /* if (partial) */

  }

}

/*******************************************************************
 * entry_valid()
 *
 * returns TRUE if this entry is valid for the track
 * type selected.
 */

static int entry_valid(const track_file_entry_t *entry,
		       const TitanPartialTrack *partial)

{

  /*
   * complex track?
   */

  if (Glob->track_shmem->track_type == COMPLEX_TRACK) {
    return (TRUE);
  }

  /*
   * simple track?
   */

  if (Glob->track_shmem->track_type == SIMPLE_TRACK) {
    if (entry->simple_track_num == Glob->track_shmem->simple_track_num) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  /*
   * partial track
   */

  if (partial && partial->entryIncluded(*entry)) {
    return (TRUE);
  } else {
    return (FALSE);
  }

}
  
/*******************************************************************
 * load_basic_data()
 */

static void load_basic_data(const TitanPartialTrack *partial)

{
  
  static int first_call = TRUE;
  static int nscans_allocated;
  
  int time_range = End_time - Start_time;
  double mean_dt;
  if (Nscans > 1) {
    mean_dt = (double) time_range / (double) (Nscans - 1);
  } else {
    mean_dt = 1.0;
  }

  thist_track_data_t &tdata = Glob->tdata;

  tdata.plot_start_time = Start_time - (int) (mean_dt / 2.0);
  tdata.plot_end_time = End_time + (int) (mean_dt / 2.0);
  tdata.nscans = Nscans;
  tdata.scan_duration = (int) (mean_dt + 0.5);

  /*
   * if first call, allocate memory. Otherwise, reallocate.
   */
  
  if (first_call) {
    
    tdata.scan = (thist_scan_data_t *)
      umalloc((ui32) (Nscans * sizeof(thist_scan_data_t)));
    
    nscans_allocated = Nscans;
    
    first_call = FALSE;
    
  } else { 
    
    for (int iscan = 0; iscan < nscans_allocated; iscan++) {
      
      ufree((char *) tdata.scan[iscan].timeht_data);
      ufree((char *) tdata.scan[iscan].timeht_flag);
      ufree((char *) tdata.scan[iscan].rdist_data);
      ufree((char *) tdata.scan[iscan].rdist_flag);
      
    }
    
    tdata.scan = (thist_scan_data_t *)
      urealloc((char *) tdata.scan,
	       (ui32) (Nscans * sizeof(thist_scan_data_t)));

    nscans_allocated = Nscans;

  } /* if (first_call) */
  
  /*
   * initialize
   */
  
  tdata.max_dbz_intervals = 0;
  tdata.max_layers = 0;
  tdata.max_top = 0;
  tdata.min_base = LARGE_DOUBLE;
  
  memset ((void *)  tdata.scan,
          (int) 0, (size_t) (Nscans * sizeof(thist_scan_data_t)));
  
  for (int iscan = 0; iscan < Nscans; iscan++) {
    
    tdata.scan[iscan].top = 0;
    tdata.scan[iscan].dbz_max = -LARGE_DOUBLE;
    tdata.scan[iscan].base = LARGE_DOUBLE;
    tdata.scan[iscan].min_z = LARGE_DOUBLE;
    tdata.scan[iscan].delta_z = LARGE_DOUBLE;
    
  } /* iscan */
  

  const storm_file_params_t &sparams = Glob->_dsTitan.storm_params();
  tdata.dbz_hist_interval = sparams.dbz_hist_interval;
  tdata.dbz_threshold = sparams.low_dbz_threshold;
  
  /*
   * loop through track entries
   */
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];

  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {
    
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const track_file_entry_t &entry = tentry->entry();
      
      if (entry_valid(&entry, partial)) {

	const storm_file_global_props_t &gprops = tentry->gprops();
	const storm_file_scan_header_t &scan_hdr = tentry->scan();
	int scan_index = entry.scan_num - Start_scan;

	tdata.scan[scan_index].time = entry.time;
	
	double volume = gprops.volume;
	tdata.scan[scan_index].volume += volume;
	
	double delta_z = scan_hdr.delta_z;
	double min_z = scan_hdr.min_z;
	double max_z = min_z +
	  (gprops.base_layer + gprops.n_layers - 1.0) * delta_z;
	
	tdata.scan[scan_index].delta_z =
	  MIN(delta_z, tdata.scan[scan_index].delta_z);
	
	tdata.scan[scan_index].min_z =
	  MIN(tdata.scan[scan_index].min_z, min_z);
	
	tdata.scan[scan_index].max_z =
	  MAX(tdata.scan[scan_index].max_z, max_z);
	
	double dbz_max = gprops.dbz_max;
	
	if (dbz_max > tdata.scan[scan_index].dbz_max) {

	  double ht_of_dbz_max = gprops.ht_of_dbz_max;
	  tdata.scan[scan_index].ht_of_dbz_max = ht_of_dbz_max;
	  tdata.scan[scan_index].dbz_max = dbz_max;

	}

	tdata.scan[scan_index].vol_centroid_z +=
	  gprops.vol_centroid_z * volume;
	
	tdata.scan[scan_index].refl_centroid_z +=
	  gprops.refl_centroid_z * volume;
	
	double top = gprops.top;
	tdata.scan[scan_index].top =
	  MAX(top, tdata.scan[scan_index].top);
	tdata.max_top = MAX(top, tdata.max_top);

	double base = gprops.base;
	tdata.scan[scan_index].base =
	  MIN(base, tdata.scan[scan_index].base);
	tdata.min_base = MIN(base, tdata.min_base);
	
	tdata.scan[scan_index].n_dbz_intervals =
	  MAX(tdata.scan[scan_index].n_dbz_intervals,
	      gprops.n_dbz_intervals);

      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {
    
    /*
     * compute number of layers - this is done in this way
     * because the delta_z value may vary, and the delta_z value
     * used is the minimum one.
     */
    
    tdata.scan[iscan].n_layers =
      (int) ((tdata.scan[iscan].max_z - tdata.scan[iscan].min_z) /
	      tdata.scan[iscan].delta_z + 1.5);
    
    tdata.max_layers = MAX(tdata.max_layers,
			    tdata.scan[iscan].n_layers);
    
    tdata.max_dbz_intervals =
      MAX(tdata.max_dbz_intervals, 
	  tdata.scan[iscan].n_dbz_intervals);
    
    /*
     * compute centroid values - they are volume weighted
     */
    
    tdata.scan[iscan].vol_centroid_z /= tdata.scan[iscan].volume;
    tdata.scan[iscan].refl_centroid_z /= tdata.scan[iscan].volume;
    
  } /* iscan */
  
}

/*******************************************************************
 * load_thist_data()
 */

static void load_thist_data(const TitanPartialTrack *partial)

{

  thist_track_data_t &tdata = Glob->tdata;

  /*
   * initialize
   */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {
    
    memset(tdata.scan[iscan].thist_data, 0,
	   THIST_N_FIELDS * sizeof(double));
    memset(tdata.scan[iscan].thist_dval_dt, 0,
	   THIST_N_FIELDS * sizeof(double));
    
  } /* iscan */

  /*
   * set label and unit strings
   */
  
  tdata.thist_label[THIST_VOL_FIELD] = (char *) "Vol";
  tdata.thist_units[THIST_VOL_FIELD] = (char *) "km3";
  tdata.thist_label[THIST_AREA_FIELD] = (char *) "Area";
  tdata.thist_units[THIST_AREA_FIELD] = (char *) "km2";
  tdata.thist_label[THIST_PFLUX_FIELD] = (char *) "Pflux";
  tdata.thist_units[THIST_PFLUX_FIELD] = (char *) "m3/s";
  tdata.thist_label[THIST_MASS_FIELD] = (char *) "Mass";
  tdata.thist_units[THIST_MASS_FIELD] = (char *) "ktons";
  tdata.thist_label[THIST_VIL_FIELD] = (char *) "Vil";
  tdata.thist_units[THIST_VIL_FIELD] = (char *) "kg/m2";

  /*
   * loop through track entries
   */
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];
  
  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {
    
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const track_file_entry_t &entry = tentry->entry();
      
      if (entry_valid(&entry, partial)) {
	
	const storm_file_global_props_t &gprops = tentry->gprops();
	const storm_file_scan_header_t &scan_hdr = tentry->scan();
	const track_file_forecast_props_t &fprops = entry.dval_dt;
	int scan_index = entry.scan_num - Start_scan;
	
	tdata.scan[scan_index].thist_data[THIST_VOL_FIELD] +=
	  gprops.volume;
	tdata.scan[scan_index].thist_dval_dt[THIST_VOL_FIELD] +=
	  fprops.volume / 3600.0;
	
	tdata.scan[scan_index].thist_data[THIST_AREA_FIELD] +=
	  gprops.proj_area;
	tdata.scan[scan_index].thist_dval_dt[THIST_AREA_FIELD] +=
	  fprops.proj_area / 3600.0;
	
	tdata.scan[scan_index].thist_data[THIST_PFLUX_FIELD] +=
	  gprops.precip_flux;
	tdata.scan[scan_index].thist_dval_dt[THIST_PFLUX_FIELD] +=
	  fprops.precip_flux / 3600.0;
	  
	tdata.scan[scan_index].thist_data[THIST_MASS_FIELD] +=
	  gprops.mass;
	tdata.scan[scan_index].thist_dval_dt[THIST_MASS_FIELD] +=
	  fprops.mass / 3600.0;

	/*
	 * vil
	 */

	const vector<storm_file_layer_props_t> &lprops = tentry->lprops();
	vil_init();
	for (int ilayer = 0; ilayer < gprops.n_layers; ilayer++) {
	  vil_add(lprops[ilayer].dbz_max, scan_hdr.delta_z);
	}
	tdata.scan[scan_index].thist_data[THIST_VIL_FIELD] += vil_compute();

      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute max and min values
   */
  
  double forecast_lead_time = (Glob->track_shmem->n_forecast_steps *
			       Glob->track_shmem->forecast_interval);

  tdata.forecast_end_time = tdata.plot_end_time;

  for (int ifield = 0; ifield < THIST_N_FIELDS; ifield++) {

    double min_val = LARGE_DOUBLE;
    double max_val = -LARGE_DOUBLE;

    for (int iscan = 0; iscan < Nscans; iscan++) {
      
      double current_val = tdata.scan[iscan].thist_data[ifield];
      
      double forecast_val = current_val +
	tdata.scan[iscan].thist_dval_dt[ifield] * forecast_lead_time;
      
      min_val = MIN(min_val, current_val);
      max_val = MAX(max_val, current_val);
      
      if (Glob->thist_forecast == SELECTED_ALL ||
	  (Glob->thist_forecast == SELECTED_LIMITED && 
	   tdata.scan[iscan].time == Glob->track_shmem->time)) {
	
	min_val = MIN(min_val, forecast_val);
	max_val = MAX(max_val, forecast_val);
	
	time_t forecast_end_time = (tdata.scan[iscan].time +
				    (Glob->track_shmem->n_forecast_steps *
				     Glob->track_shmem->forecast_interval));
	
	tdata.forecast_end_time =
	  MAX(tdata.forecast_end_time, forecast_end_time);
	
      } /* if (Glob->thist_forecast == SELECTED_ALL ... */
      
    } /* iscan */

    tdata.min_thist_val[ifield] = min_val;
    tdata.max_thist_val[ifield] = max_val;

  } /* ifield */
  
}

/*******************************************************************
 * load_timeht_data()
 */

static void load_timeht_data(const TitanPartialTrack *partial)

{

  thist_track_data_t &tdata = Glob->tdata;

  /*
   * allocate memory for timeht data array
   */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {

    tdata.scan[iscan].timeht_data = (double *)
      ucalloc ((ui32) tdata.max_layers,
	       (ui32) sizeof(double));

    tdata.scan[iscan].timeht_flag = (int *)
      ucalloc ((ui32) tdata.max_layers,
	       (ui32) sizeof(int));

  }
  
  /*
   * set labels
   */
  
  switch (Glob->timeht_mode) {

  case FALSE:
    tdata.timeht_label = (char *) "";
    break;
    
  case TIMEHT_MAXZ:
    
    tdata.timeht_label = (char *) "Max dbz";
    break;
    
  case TIMEHT_MEANZ:
    
    tdata.timeht_label = (char *) "Mean dbz";
    break;
    
  case TIMEHT_MASS:
    
    tdata.timeht_label = (char *) "% mass";
    break;
    
  case TIMEHT_VORTICITY:
    
    tdata.timeht_label = (char *) "Vort.";
    break;
    
  } /* switch */
  
  /*
   * loop through track entries
   */
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];

  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {
    
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const track_file_entry_t &entry = tentry->entry();
      
      if (entry_valid(&entry, partial)) {

	const storm_file_global_props_t &gprops = tentry->gprops();
	const storm_file_scan_header_t &scan_hdr = tentry->scan();
	int scan_index = entry.scan_num - Start_scan;

	double volume = gprops.volume;
	double min_z = scan_hdr.min_z;
	double delta_z = scan_hdr.delta_z;
	
	const vector<storm_file_layer_props_t> &lprops = tentry->lprops();

	for (int ilayer = 0; ilayer < gprops.n_layers; ilayer++) {
	  
	  double z = min_z + (ilayer + gprops.base_layer) * delta_z;
	  
	  int jlayer = (int) ((z - tdata.scan[scan_index].min_z) /
			      tdata.scan[scan_index].delta_z + 0.5);
	  
	  /*
	   * set flag to indicate that there is data at this level
	   */
	  
	  tdata.scan[scan_index].timeht_flag[jlayer] = TRUE;

	  /*
	   * load data 
	   */

	  switch (Glob->timeht_mode) {
	    
	  case TIMEHT_MAXZ:
	    
	    tdata.scan[scan_index].timeht_data[jlayer] =
	      MAX(tdata.scan[scan_index].timeht_data[jlayer],
		  lprops[ilayer].dbz_max);
	    break;
	    
	  case TIMEHT_MEANZ:
	    
	    tdata.scan[scan_index].timeht_data[jlayer] +=
	      (lprops[ilayer].dbz_mean * volume);
	    break;
	    
	  case TIMEHT_MASS:

	    tdata.scan[scan_index].timeht_data[jlayer] +=
	      lprops[ilayer].mass * volume;
	    break;
	    
	  case TIMEHT_VORTICITY:
	    
	    tdata.scan[scan_index].timeht_data[jlayer] +=
	      (lprops[ilayer].vorticity * volume);
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
    
    for (int iscan = 0; iscan < Nscans; iscan++) {
      
      for (int ilayer = 0; ilayer < tdata.max_layers; ilayer++) {
	
	tdata.scan[iscan].timeht_data[ilayer] /=
	  tdata.scan[iscan].volume;
	
      } /* ilayer */
      
    } /* iscan */

    break;
    
  case TIMEHT_MASS:
    
    for (int iscan = 0; iscan < Nscans; iscan++) {
    
      double sum_mass = 0.0;
      
      for (int ilayer = 0; ilayer < tdata.max_layers; ilayer++) {
	
	tdata.scan[iscan].timeht_data[ilayer] /=
	  tdata.scan[iscan].volume;

	sum_mass += tdata.scan[iscan].timeht_data[ilayer];
	
      } /* ilayer */
      
      for (int ilayer = 0; ilayer < tdata.max_layers; ilayer++) {
	
	tdata.scan[iscan].timeht_data[ilayer] *=
	  (100.0 / sum_mass);

      } /* ilayer */
      
    } /* iscan */

    break;
    
  } /* switch */
  
}

/*******************************************************************
 * load_rdist_data()
 */

static void load_rdist_data(const TitanPartialTrack *partial)

{
  
  thist_track_data_t &tdata = Glob->tdata;

  /*
   * allocate memory for rdist data array
   */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {

    tdata.scan[iscan].rdist_data = (double *)
      ucalloc ((ui32) tdata.max_dbz_intervals,
	       (ui32) sizeof(double));

    tdata.scan[iscan].rdist_flag = (int *)
      ucalloc ((ui32) tdata.max_dbz_intervals,
	       (ui32) sizeof(int));

  }
  
  /*
   * set labels
   */
  
  switch (Glob->rdist_mode) {
    
  case RDIST_VOL:
    
    tdata.rdist_label = (char *) "% vol";
    break;
    
  case RDIST_AREA:
    
    tdata.rdist_label = (char *) "% area";
    break;
    
  } /* switch */
  
  /*
   * loop through track entries
   */
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];
  
  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {
    
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const track_file_entry_t &entry = tentry->entry();
      
      if (entry_valid(&entry, partial)) {
	
	const storm_file_global_props_t &gprops = tentry->gprops();
	int scan_index = entry.scan_num - Start_scan;
	double volume = gprops.volume;
	
	const vector<storm_file_dbz_hist_t> &hist = tentry->hist();
	  
	for (int ii = 0; ii < gprops.n_dbz_intervals; ii++) {
	  
	  /*
	   * set flag to indicate that there is data in this interval
	   */

	  tdata.scan[scan_index].rdist_flag[ii] = TRUE;

	  switch (Glob->rdist_mode) {
	    
	  case RDIST_VOL:
	    
	    tdata.scan[scan_index].rdist_data[ii] +=
	      hist[ii].percent_volume * volume;
	    break;
	    
	  case RDIST_AREA:
	    
	    tdata.scan[scan_index].rdist_data[ii] +=
	      hist[ii].percent_area * volume;
	    break;
	    
	  } /* switch */
	  
	} /* ii */
	
      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute volume-weighted means
   */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {
    
    for (int ii = 0;
	 ii < tdata.max_dbz_intervals; ii++) {
      tdata.scan[iscan].rdist_data[ii] /=tdata.scan[iscan].volume;
    }
    
  } /* iscan */
  
}

/*******************************************************************
 * load_union_data()
 */

static void load_union_data(const TitanPartialTrack *partial)

{

  thist_track_data_t &tdata = Glob->tdata;

  /*
   * initialize
   */
  
  for (int iscan = 0; iscan < Nscans; iscan++) {
    memset(tdata.scan[iscan].union_data, 0,
	   UNION_N_FIELDS * sizeof(double));
  } /* iscan */
  
  /*
   * set label and unit strings
   */
  
  if (Glob->union_mode == UNION_MODE_FLOATS) {
    tdata.union_label[UNION_0_FIELD] = (char *) "float_0";
    tdata.union_units[UNION_0_FIELD] = (char *) "unknown";
    tdata.union_label[UNION_1_FIELD] = (char *) "float_1";
    tdata.union_units[UNION_1_FIELD] = (char *) "unknown";
    tdata.union_label[UNION_2_FIELD] = (char *) "float_2";
    tdata.union_units[UNION_2_FIELD] = (char *) "unknown";
    tdata.union_label[UNION_3_FIELD] = (char *) "float_3";
    tdata.union_units[UNION_3_FIELD] = (char *) "unknown";
  } else if (Glob->union_mode == UNION_MODE_HAIL) {
    tdata.union_label[UNION_0_FIELD] = (char *) "FOKR";
    tdata.union_units[UNION_0_FIELD] = (char *) "Cat";
    tdata.union_label[UNION_1_FIELD] = (char *) "poh";
    tdata.union_units[UNION_1_FIELD] = (char *) "%";
    tdata.union_label[UNION_2_FIELD] = (char *) "HMass";
    tdata.union_units[UNION_2_FIELD] = (char *) "ktons";
    tdata.union_label[UNION_3_FIELD] = (char *) "vihm";
    tdata.union_units[UNION_3_FIELD] = (char *) "kg/m2";
  }

  /*
   * loop through track entries
   */
  
  const TitanComplexTrack *ctrack =
    Glob->_dsTitan.complex_tracks()[Glob->complex_index];
  
  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {
    
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const track_file_entry_t &entry = tentry->entry();
      
      if (entry_valid(&entry, partial)) {
	
	const storm_file_global_props_t &gprops = tentry->gprops();
	int scan_index = entry.scan_num - Start_scan;
	
	if (Glob->union_mode == UNION_MODE_FLOATS) {

	  tdata.scan[scan_index].union_data[UNION_0_FIELD] +=
	    gprops.add_on.float_data[0];
	  
	  tdata.scan[scan_index].union_data[UNION_1_FIELD] +=
	    gprops.add_on.float_data[1];
	  
	  tdata.scan[scan_index].union_data[UNION_2_FIELD] +=
	    gprops.add_on.float_data[2];
	  
	  tdata.scan[scan_index].union_data[UNION_3_FIELD] +=
	    gprops.add_on.float_data[3];

	} else if (Glob->union_mode == UNION_MODE_HAIL) {

	  tdata.scan[scan_index].union_data[UNION_0_FIELD] =
	    MAX(tdata.scan[scan_index].union_data[UNION_0_FIELD],
		gprops.add_on.hail_metrics.FOKRcategory);
	  
	  tdata.scan[scan_index].union_data[UNION_1_FIELD] =
	    MAX(tdata.scan[scan_index].union_data[UNION_1_FIELD],
		gprops.add_on.hail_metrics.waldvogelProbability * 100.0);
	  
	  tdata.scan[scan_index].union_data[UNION_2_FIELD] +=
	    gprops.add_on.hail_metrics.hailMassAloft;
	  
	  tdata.scan[scan_index].union_data[UNION_3_FIELD] =
	    MAX(tdata.scan[scan_index].union_data[UNION_3_FIELD],
		gprops.add_on.hail_metrics.vihm);

	}

      } /* if (entry_valid ... */
	
    } /* ientry */
      
  } /* isimple */
  
  /*
   * compute max and min values
   */
  
  for (int ifield = 0; ifield < UNION_N_FIELDS; ifield++) {

    double min_val = LARGE_DOUBLE;
    double max_val = -LARGE_DOUBLE;

    for (int iscan = 0; iscan < Nscans; iscan++) {
      
      double current_val = tdata.scan[iscan].union_data[ifield];
      
      if (current_val != 0) {
	min_val = MIN(min_val, current_val);
      }
      max_val = MAX(max_val, current_val);
      
    } /* iscan */

    tdata.min_union_val[ifield] = min_val;
    tdata.max_union_val[ifield] = max_val;

  } /* ifield */
  
}

