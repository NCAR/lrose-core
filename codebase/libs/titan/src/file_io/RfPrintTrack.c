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
/*************************************************************************
 *
 * RfPrintTrack.c
 *
 * Track file printing routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1995
 *
 **************************************************************************/

#include <titan/track.h>

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

#define FORECAST_TYPE_STR(a) \
(a == FORECAST_BY_TREND ? "trend" : \
 a == FORECAST_BY_PARABOLA ? "parabola" : \
 a == FORECAST_BY_REGRESSION ? "regression" : \
 "unknown" )

/*---------------------------------------
 */

void RfPrintComplexTrackParams(FILE *out,
			       const char *spacer,
			       int verification_performed,
			       const track_file_params_t *params,
			       const complex_track_params_t *cparams,
			       const si32 *simples_per_complex)
     
{
  
  int i;
  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  fprintf(out, "%sCOMPLEX_TRACK_NUM : %ld\n", spacer,
	  (long) cparams->complex_track_num);
  fprintf(out, "%s  n_simple_tracks : %ld\n", spacer,
	  (long) cparams->n_simple_tracks);
  
  if (cparams->n_simple_tracks > 0 && simples_per_complex != NULL) {
    fprintf(out, "%s  simple_track_nums :", spacer);  
    for (i = 0; i < cparams->n_simple_tracks; i++) {
      if (i == cparams->n_simple_tracks - 1) {
	fprintf(out, " %ld\n", (long) simples_per_complex[i]);
      } else {
	fprintf(out, " %ld,", (long) simples_per_complex[i]);
      }
    } /* i */
  } /* if */
  
  fprintf(out, "%s  start_scan : %ld\n", spacer, (long) cparams->start_scan);
  fprintf(out, "%s  end_scan : %ld\n", spacer, (long) cparams->end_scan);
  
  fprintf(out, "%s  duration_in_scans : %ld\n", spacer,
	  (long) cparams->duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %ld\n", spacer,
	  (long) cparams->duration_in_secs);
  
  fprintf(out, "%s  start_time : %s\n", spacer,
	  utimstr(cparams->start_time));
  fprintf(out, "%s  end_time : %s\n", spacer,
	  utimstr(cparams->end_time));
  
  if (verification_performed) {

    fprintf(out, "%s  n_top_missing : %ld\n", spacer,
	    (long) cparams->n_top_missing);
    
    fprintf(out, "%s  n_range_limited : %ld\n", spacer,
	    (long) cparams->n_range_limited);
    
    fprintf(out, "%s  start_missing : %ld\n", spacer,
	    (long) cparams->start_missing);
    
    fprintf(out, "%s  end_missing : %ld\n", spacer,
	    (long) cparams->end_missing);
    
    fprintf(out, "%s  volume_at_start_of_sampling : %g\n", spacer,
	    cparams->volume_at_start_of_sampling);
    
    fprintf(out, "%s  volume_at_end_of_sampling : %g\n", spacer,
	    cparams->volume_at_end_of_sampling);
    
    RfPrintContTable(out, "  Ellipse verify:", spacer2,
		     &cparams->ellipse_verify);
  
    RfPrintContTable(out, "  Polygon verify:", spacer2,
		     &cparams->polygon_verify);
  
    RfPrintForecastProps(out, "  Forecast bias:", spacer2,
			 params, &cparams->forecast_bias);

    RfPrintForecastProps(out, "  Forecast RMSE:", spacer2,
			 params, &cparams->forecast_rmse);
    
    fprintf(out, "\n");
  
  } /* if (verification_performed) */
  
  fprintf(out, "\n");

  return;
  
}

/*---------------------------------------
 */

void RfPrintComplexTrackParamsXML(FILE *out,
                                  const char *spacer,
                                  int verification_performed,
                                  const track_file_params_t *params,
                                  const complex_track_params_t *cparams,
                                  const si32 *simples_per_complex)
     
{
  
  int i;
  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  fprintf(out, "%s  <n_simple_tracks> %ld </n_simple_tracks>\n", spacer,
	  (long) cparams->n_simple_tracks);
  if (cparams->n_simple_tracks > 0 && simples_per_complex != NULL) {
      for (i = 0; i < cparams->n_simple_tracks; i++) {
	  fprintf(out, "%s      <simple_track_nums"
		  " num=\"%d\"> %ld </simple_track_nums>\n", spacer, i, 
		  (long) simples_per_complex[i]);
      } 
  }

  fprintf(out, "%s  <start_scan>  %ld </start_scan>\n", spacer, (long) cparams->start_scan);
  fprintf(out, "%s  <end_scan>  %ld </end_scan>\n", spacer, (long) cparams->end_scan);
  
  fprintf(out, "%s  <duration_in_scans> %ld </duration_in_scans>\n", spacer,
	  (long) cparams->duration_in_scans);
  fprintf(out, "%s  <duration_in_secs> %ld </duration_in_secs>\n", spacer,
	  (long) cparams->duration_in_secs);
  
  fprintf(out, "%s  <start_time><unixtime> %d </unixtime></start_time>\n", spacer,
	  cparams->start_time);
  fprintf(out, "%s  <end_time><unixtime> %d </unixtime></end_time>\n", spacer,
	  cparams->end_time);
  
  fprintf(out, "\n");

  return;
  
}

/*------------------------------
 */

void RfPrintContTable(FILE *out,
		      const char *label,
		      const char *spacer,
		      const track_file_contingency_data_t *count)
     
{

  fl32 denom;

  fprintf(out, "\n%s%s\n\n", spacer, label);
  
  fprintf(out, "  %sn_success : %g\n", spacer, count->n_success);
  fprintf(out, "  %sn_failure : %g\n", spacer,count->n_failure);
  fprintf(out, "  %sn_false_alarm : %g\n", spacer, count->n_false_alarm);

  denom = count->n_success + count->n_failure;

  if (denom == 0) {
    fprintf(out, "  %sPOD not computed\n", spacer);
  } else {
    fprintf(out, "  %sPOD = %g\n", spacer, count->n_success / denom);
  }
  
  denom = count->n_success + count->n_false_alarm;

  if (denom == 0) {
    fprintf(out, "  %sFAR not computed\n", spacer);
  } else {
    fprintf(out, "  %sFAR = %g\n", spacer,
	    count->n_false_alarm / denom);
  }

  denom = count->n_success + count->n_failure + count->n_false_alarm;

  if (denom == 0) {
    fprintf(out, "  %sCSI not computed\n", spacer);
  } else {
    fprintf(out, "  %sCSI = %g\n", spacer,
	    count->n_success / denom);
  }

  return;

}

/*----------------------------------
 */

void RfPrintForecastProps(FILE *out,
			  const char *label,
			  const char *space,
			  const track_file_params_t *params,
			  const track_file_forecast_props_t *forecast)
     
{
  
  fprintf(out, "%s%s\n", space, label);
  fprintf(out, "  %sProj_Area_centroid_x : %g\n", space,
	  forecast->proj_area_centroid_x);
  fprintf(out, "  %sProj_Area_centroid_y : %g\n", space,
	  forecast->proj_area_centroid_y);
  fprintf(out, "  %sVol_centroid_z : %g\n", space,
	 forecast->vol_centroid_z);
  fprintf(out, "  %sRefl_centroid_z : %g\n", space,
	 forecast->refl_centroid_z);
  fprintf(out, "  %sDbz_max : %g\n", space,
	  forecast->dbz_max);
  fprintf(out, "  %sTop : %g\n", space,
	  forecast->top);
  fprintf(out, "  %sVolume : %g\n", space,
	  forecast->volume);
  fprintf(out, "  %sPrecip_flux : %g\n", space,
	 forecast->precip_flux);
  fprintf(out, "  %sMass : %g\n", space,
	  forecast->mass);
  fprintf(out, "  %sProj_Area : %g\n", space,
	  forecast->proj_area);
  fprintf(out, "  %sSmoothed_proj_area_centroid_x : %g\n", space,
	  forecast->smoothed_proj_area_centroid_x);
  fprintf(out, "  %sSmoothed_proj_area_centroid_y : %g\n", space,
	  forecast->smoothed_proj_area_centroid_y);
  fprintf(out, "  %sSpeed : %g\n", space,
	  forecast->smoothed_speed);
  fprintf(out, "  %sDirection : %g\n", space,
	  forecast->smoothed_direction);
  fprintf(out, "\n");


  return;

}

/*----------------------------------
 */

void RfPrintForecastPropsXML(FILE *out,
                             const char *label,
                             const char *space,
                             const track_file_params_t *params,
                             const track_file_forecast_props_t *forecast)
     
{
  
  fprintf(out, "%s<!--%s-->\n", space, label);
  fprintf(out, "%s<!--forecast elements are all changes per hour, except speed and dirn-->\n", space);
  fprintf(out, "  %s<proj_area_centroid_x> %g </proj_area_centroid_x>\n", space,
	  forecast->proj_area_centroid_x);
  fprintf(out, "  %s<proj_area_centroid_y> %g </proj_area_centroid_y>\n", space,
	  forecast->proj_area_centroid_y);
  fprintf(out, "  %s<vol_centroid_z> %g </vol_centroid_z>\n", space,
	 forecast->vol_centroid_z);
  fprintf(out, "  %s<refl_centroid_z> %g </refl_centroid_z>\n", space,
	 forecast->refl_centroid_z);
  fprintf(out, "  %s<dbz_max> %g </dbz_max>\n", space,
	  forecast->dbz_max);
  fprintf(out, "  %s<top unit=\"km/hr\"> %g </top>\n", space,
	  forecast->top);
  fprintf(out, "  %s<volume> %g </volume>\n", space,
	  forecast->volume);
  fprintf(out, "  %s<precip_flux> %g </precip_flux>\n", space,
	 forecast->precip_flux);
  fprintf(out, "  %s<mass unit=\"ktons/hr\"> %g </mass>\n", space,
	  forecast->mass);
  fprintf(out, "  %s<proj_area> %g </proj_area>\n", space,
	  forecast->proj_area);
  fprintf(out, "  %s<smoothed_proj_area_centroid_x> %g </smoothed_proj_area_centroid_x>\n", space,
	  forecast->smoothed_proj_area_centroid_x);
  fprintf(out, "  %s<smoothed_proj_area_centroid_y> %g </smoothed_proj_area_centroid_y>\n", space,
	  forecast->smoothed_proj_area_centroid_y);
  fprintf(out, "  %s<speed> %g </speed>\n", space,
	  forecast->smoothed_speed);
  fprintf(out, "  %s<direction> %g </direction>\n", space,
	  forecast->smoothed_direction);
  fprintf(out, "\n");


  return;

}
/*------------------------------
 */

void RfPrintSimpleTrackParams(FILE *out,
			      const char *spacer,
			      const simple_track_params_t *sparams)
     
{

  int i;

  fprintf(out, "%sSIMPLE_TRACK_NUM : %ld\n", spacer,
	  (long) sparams->simple_track_num);
  
  fprintf(out, "%s  last_descendant_simple_track_num : %ld\n", spacer,
	  (long) sparams->last_descendant_simple_track_num);
  
  fprintf(out, "%s  start_scan : %ld\n", spacer, (long) sparams->start_scan);
  fprintf(out, "%s  end_scan : %ld\n", spacer, (long) sparams->end_scan);
  fprintf(out, "%s  last_descendant_end_scan : %ld\n", spacer,
	  (long) sparams->last_descendant_end_scan);
  fprintf(out, "%s  scan_origin : %ld\n", spacer, (long) sparams->scan_origin);
  
  fprintf(out, "%s  start_time : %s\n", spacer,
	  utimstr(sparams->start_time));
  fprintf(out, "%s  end_time : %s\n", spacer,
	  utimstr(sparams->end_time));
  fprintf(out, "%s  last_descendant_end_time : %s\n", spacer,
	  utimstr(sparams->last_descendant_end_time));
  fprintf(out, "%s  time_origin : %s\n", spacer,
	  utimstr(sparams->time_origin));
  
  fprintf(out, "%s  history_in_scans : %ld\n", spacer,
	  (long) sparams->history_in_scans);
  fprintf(out, "%s  history_in_secs : %ld\n", spacer,
	  (long) sparams->history_in_secs);
  fprintf(out, "%s  duration_in_scans : %ld\n", spacer,
	  (long) sparams->duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %ld\n", spacer,
	  (long) sparams->duration_in_secs);
  
  fprintf(out, "%s  nparents : %ld\n", spacer, (long) sparams->nparents);
  fprintf(out, "%s  nchildren : %ld\n", spacer, (long) sparams->nchildren);
  
  if (sparams->nparents > 0) {
    fprintf(out, "%s  parents :", spacer);  
    for (i = 0; i < sparams->nparents; i++) {
      if (i == sparams->nparents - 1)
	fprintf(out, " %ld\n", (long) sparams->parent[i]);
      else
	fprintf(out, " %ld,", (long) sparams->parent[i]);
    } /* i */
  } /* if */
  
  if (sparams->nchildren > 0) {
    fprintf(out, "%s  children :", spacer);  
    for (i = 0; i < sparams->nchildren; i++) {
      if (i == sparams->nchildren - 1)
	fprintf(out, " %ld\n", (long) sparams->child[i]);
      else
	fprintf(out, " %ld,", (long) sparams->child[i]);
    } /* i */
  } /* if */
  
  fprintf(out, "%s  complex_track_num : %ld\n", spacer,
	  (long) sparams->complex_track_num);
  fprintf(out, "%s  first_entry_offset : %ld\n", spacer,
	  (long) sparams->first_entry_offset);
  
  fprintf(out, "\n");

}

void RfPrintSimpleTrackParamsXML(FILE *out,
                                 const char *spacer,
                                 const simple_track_params_t *sparams)
     
{

  fprintf(out, "%s  <last_descendant_simple_track_num> %ld </last_descendant_simple_track_num>\n", spacer,
	  (long) sparams->last_descendant_simple_track_num);
  
  fprintf(out, "%s  <start_scan> %ld </start_scan>\n", spacer, (long) sparams->start_scan);
  fprintf(out, "%s  <end_scan> %ld </end_scan>\n", spacer, (long) sparams->end_scan);
  fprintf(out, "%s  <last_descendant_end_scan> %ld </last_descendant_end_scan>\n", spacer,
	  (long) sparams->last_descendant_end_scan);
  fprintf(out, "%s  <scan_origin> %ld </scan_origin>\n", spacer, (long) sparams->scan_origin);
  
  fprintf(out, "%s  <start_time><unixtime> %d </unixtime></start_time>\n", spacer,
	  sparams->start_time);
  fprintf(out, "%s  <end_time><unixtime> %d </unixtime></end_time>\n", spacer,
	  sparams->end_time);
  fprintf(out, "%s  <last_descendant_end_time><unixtime> %d </unixtime></last_descendant_end_time>\n", spacer,
	  sparams->last_descendant_end_time);
  fprintf(out, "%s  <time_origin><unixtime> %d </unixtime></time_origin>\n", spacer,
	  sparams->time_origin);
  
  fprintf(out, "%s  <history_in_scans> %ld </history_in_scans>\n", spacer,
	  (long) sparams->history_in_scans);
  fprintf(out, "%s  <history_in_secs> %ld </history_in_secs>\n", spacer,
	  (long) sparams->history_in_secs);
  fprintf(out, "%s  <duration_in_scans> %ld </duration_in_scans>\n", spacer,
	  (long) sparams->duration_in_scans);
  fprintf(out, "%s  <duration_in_secs> %ld </duration_in_secs>\n", spacer,
	  (long) sparams->duration_in_secs);
  
  fprintf(out, "%s  <nparents> %ld </nparents>\n", spacer, (long) sparams->nparents);
  fprintf(out, "%s  <nchildren> %ld </nchildren>\n", spacer, (long) sparams->nchildren);
  /*
  if (sparams->nparents > 0) {
    fprintf(out, "%s  parents :", spacer);  
    for (i = 0; i < sparams->nparents; i++) {
      if (i == sparams->nparents - 1)
	fprintf(out, " %ld\n", (long) sparams->parent[i]);
      else
	fprintf(out, " %ld,", (long) sparams->parent[i]);
    }
  } 
  
  if (sparams->nchildren > 0) {
    fprintf(out, "%s  children :", spacer);  
    for (i = 0; i < sparams->nchildren; i++) {
      if (i == sparams->nchildren - 1)
	fprintf(out, " %ld\n", (long) sparams->child[i]);
      else
	fprintf(out, " %ld,", (long) sparams->child[i]);
    } 
  } 
  */
  fprintf(out, "%s  <complex_track_num> %ld </complex_track_num>\n", spacer,
	  (long) sparams->complex_track_num);
  
  fprintf(out, "\n");

}

/*------------------------------
 */

void RfPrintTrackEntry(FILE *out,
		       const char *spacer,
		       si32 entry_num,
		       const track_file_params_t *params,
		       const track_file_entry_t *entry)
     
{

  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  fprintf(out, "%sENTRY NUMBER : %ld\n", spacer, (long) entry_num);
  
  fprintf(out, "%s  time : %s\n", spacer, utimstr(entry->time));
  fprintf(out, "%s  time_origin : %s\n", spacer,
	  utimstr(entry->time_origin));
  
  fprintf(out, "%s  scan_origin in storm file : %ld\n", spacer,
	  (long) entry->scan_origin);
  fprintf(out, "%s  scan_num in storm file : %ld\n", spacer,
	  (long) entry->scan_num);
  fprintf(out, "%s  storm_num in storm file : %ld\n", spacer,
	  (long) entry->storm_num);
  
  fprintf(out, "%s  simple_track_num : %ld\n", spacer,
	  (long) entry->simple_track_num);
  fprintf(out, "%s  complex_track_num : %ld\n", spacer,
	  (long) entry->complex_track_num);
  
  fprintf(out, "%s  history_in_scans : %ld\n", spacer,
	  (long) entry->history_in_scans);
  fprintf(out, "%s  history_in_secs : %ld\n", spacer,
	  (long) entry->history_in_secs);
  
  fprintf(out, "%s  duration_in_scans : %ld\n", spacer,
	  (long) entry->duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %ld\n", spacer,
	  (long) entry->duration_in_secs);
  
  fprintf(out, "%s  forecast_valid : %s\n", spacer,
	  BOOL_STR(entry->forecast_valid));
  
  fprintf(out, "%s  prev_entry_offset : %ld\n", spacer,
	  (long) entry->prev_entry_offset);
  fprintf(out, "%s  this_entry_offset : %ld\n", spacer,
	  (long) entry->this_entry_offset);
  fprintf(out, "%s  next_entry_offset : %ld\n", spacer,
	  (long) entry->next_entry_offset);
  
  fprintf(out, "\n");

  RfPrintForecastProps(out, "Entry forecast:",
		       spacer2, params,
		       &entry->dval_dt);

  return;
  
}

/*------------------------------
 */

void RfPrintTrackEntryXML(FILE *out,
                          const char *spacer,
                          si32 entry_num,
                          const track_file_params_t *params,
                          const track_file_entry_t *entry)
     
{

  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  fprintf(out, "%s  <time_track><unixtime>  %d </unixtime></time_track>\n", spacer, entry->time);
  fprintf(out, "%s  <time_origin><unixtime> %d </unixtime></time_origin>\n", spacer,
	  entry->time_origin);
  
  fprintf(out, "%s  <scan_origin_in_storm_file> %ld </scan_origin_in_storm_file>\n", spacer,
	  (long) entry->scan_origin);
  fprintf(out, "%s  <scan_num_in_storm_file> %ld </scan_num_in_storm_file>\n", spacer,
	  (long) entry->scan_num);
  fprintf(out, "%s  <storm_num_in_storm_file> %ld </storm_num_in_storm_file>\n", spacer,
	  (long) entry->storm_num);
  
  fprintf(out, "%s  <simple_track_num> %ld </simple_track_num>\n", spacer,
	  (long) entry->simple_track_num);
  fprintf(out, "%s  <complex_track_num> %ld </complex_track_num>\n", spacer,
	  (long) entry->complex_track_num);
  
  fprintf(out, "%s  <history_in_scans> %ld </history_in_scans>\n", spacer,
	  (long) entry->history_in_scans);
  fprintf(out, "%s  <history_in_secs> %ld </history_in_secs>\n", spacer,
	  (long) entry->history_in_secs);
  
  fprintf(out, "%s  <duration_in_scans> %ld </duration_in_scans>\n", spacer,
	  (long) entry->duration_in_scans);
  fprintf(out, "%s  <duration_in_secs> %ld </duration_in_secs>\n", spacer,
	  (long) entry->duration_in_secs);
  
  fprintf(out, "%s  <forecast_valid> %s </forecast_valid>\n", spacer,
	  BOOL_STR(entry->forecast_valid));
  /* irrelevant!
  fprintf(out, "%s  <prev_entry_offset> %ld </prev_entry_offset>\n", spacer,
	  (long) entry->prev_entry_offset);
  fprintf(out, "%s  <this_entry_offset> %ld </this_entry_offset>\n", spacer,
	  (long) entry->this_entry_offset);
  fprintf(out, "%s  <next_entry_offset> %ld </next_entry_offset>\n", spacer,
	  (long) entry->next_entry_offset);
  */
  fprintf(out, "\n");

  RfPrintForecastPropsXML(out, "Entry forecast",
		       spacer2, params,
		       &entry->dval_dt);

  return;
  
}
/*--------------------------------
 */

void RfPrintTrackHeader(FILE *out,
			const char *spacer,
			const track_file_header_t *header)
     
{

  char spacer2[128];
  const track_file_params_t *params;

  params = &header->params;
  sprintf(spacer2, "%s  ", spacer);
   
  /*
   * print out header
   */

  RfPrintTrackParams(out, spacer, params);

  fprintf(out, "%sTRACK FILE HEADER :\n", spacer);

  fprintf(out, "%s  Major revision num : %ld\n", spacer,
	  (long) header->major_rev);
  fprintf(out, "%s  Minor revision num : %ld\n", spacer,
	  (long) header->minor_rev);
  
  fprintf(out, "%s  File time : %s\n", spacer,
	  utimstr(header->file_time));
  
  fprintf(out, "%s  Header file name : %s\n", spacer,
	  header->header_file_name);
  
  fprintf(out, "%s  Data file name : %s\n", spacer,
	  header->data_file_name);
  
  fprintf(out, "%s  Storm header file name : %s\n", spacer,
	  header->storm_header_file_name);
  
  fprintf(out, "%s  Number of simple tracks : %ld\n",
	  spacer, (long) header->n_simple_tracks);

  fprintf(out, "%s  Number of complex tracks : %ld\n",
	  spacer, (long) header->n_complex_tracks);
  
  fprintf(out, "%s  n_samples_for_forecast_stats : %ld\n",
	  spacer, (long) header->n_samples_for_forecast_stats);

  fprintf(out, "%s  n_scans : %ld\n",
	  spacer, (long) header->n_scans);

  fprintf(out, "%s  last_scan_num : %ld\n",
	  spacer, (long) header->last_scan_num);

  fprintf(out, "%s  max_simple_track_num : %ld\n",
	  spacer, (long) header->max_simple_track_num);

  fprintf(out, "%s  max_complex_track_num : %ld\n",
	  spacer, (long) header->max_complex_track_num);

  fprintf(out, "%s  data_file_size : %ld\n",
	  spacer, (long) header->data_file_size);

  fprintf(out, "%s  max_parents : %ld\n", spacer,
	  (long) header->max_parents);

  fprintf(out, "%s  max_children : %ld\n", spacer,
	  (long) header->max_children);

  fprintf(out, "%s  max_nweights_forecast : %ld\n", spacer,
	  (long) header->max_nweights_forecast);

  fprintf(out, "%s  nbytes_char : %ld\n", spacer,
	  (long) header->nbytes_char);

  fprintf(out, "\n");
  
  if (header->verify.verification_performed) {

    RfPrintContTable(out, "  Ellipse verify:", spacer2,
		     &header->ellipse_verify);
  
    RfPrintContTable(out, "  Polygon verify:", spacer2,
		     &header->polygon_verify);
  
    RfPrintForecastProps(out, "  Forecast bias:", spacer2,
			 params, &header->forecast_bias);

    RfPrintForecastProps(out, "  Forecast RMSE:", spacer2,
			 params, &header->forecast_rmse);

    RfPrintTrackVerify(out, spacer2, &header->verify);
    
    fprintf(out, "\n");
  
  } /* if (header ... */
  
  return;

}

/*--------------------------------
 */

void RfPrintTrackHeaderArrays(FILE *out,
			      const char *spacer,
			      const track_file_header_t *header,
			      const si32 *complex_track_nums,
			      const si32 *complex_track_offsets,
			      const si32 *simple_track_offsets,
			      const si32 *nsimples_per_complex,
			      const si32 *simples_per_complex_offsets,
			      const si32 **simples_per_complex,
			      const track_file_scan_index_t *scan_index)
     
{
  
  int i, icomplex;
  si32 complex_num;
  si32 n_complex_tracks, n_simple_tracks;
  si32 nsimples;
  
  n_complex_tracks = header->n_complex_tracks;
  n_simple_tracks = header->n_simple_tracks;

  fprintf(out, "%sHeader file arrays:\n", spacer);
  fprintf(out, "%s------------------\n\n", spacer);

  /*
   * write out complex_track nums
   */

  fprintf(out, "\n");
  fprintf(out, "%sCOMPLEX_TRACK_NUMS: ", spacer);
  for (i = 0; i < n_complex_tracks; i++) {
    fprintf(out, "%d ", 
	    (int) complex_track_nums[i]);
  }
  fprintf(out, "\n");

  fprintf(out, "\n");
  fprintf(out, "%sCOMPLEX_TRACK_OFFSETS: ", spacer);
  for (i = 0; i < n_complex_tracks; i++) {
    complex_num = complex_track_nums[i];
    fprintf(out, "%d ", 
	    (int) complex_track_offsets[complex_num]);
  }
  fprintf(out, "\n");

  fprintf(out, "\n");
  fprintf(out, "%sSIMPLE_TRACK_OFFSETS: ", spacer);
  for (i = 0; i < n_simple_tracks; i++) {
    fprintf(out, "%d ", 
	    (int) simple_track_offsets[i]);
  }
  fprintf(out, "\n");
  
  fprintf(out, "\n");
  fprintf(out, "%sSCAN_ENTRY_OFFSETS (time, n_entries, offset):\n", spacer);
  for (i = 0; i < header->n_scans; i++) {
    fprintf(out, "%s  %s %d %d\n", spacer,
	    utimstr(scan_index[i].utime),
	    scan_index[i].n_entries,
	    scan_index[i].first_entry_offset);
  }
  fprintf(out, "\n");

  fprintf(out, "\n");
  fprintf(out, "%sSIMPLES_PER_COMPLEX_OFFSETS: ", spacer);
  for (i = 0; i < n_complex_tracks; i++) {
    complex_num = complex_track_nums[i];
    fprintf(out, "%d ", 
	    (int) simples_per_complex_offsets[complex_num]);
  }
  fprintf(out, "\n");
  
  /*
   * write out nsimples_per_complex
   */
  
  fprintf(out, "\n");
  fprintf(out, "%sNSIMPLES_PER_COMPLEX: ", spacer);
  for (i = 0; i < n_complex_tracks; i++) {
    complex_num = complex_track_nums[i];
    fprintf(out, "%d ", 
	    (int) nsimples_per_complex[complex_num]);
  }
  fprintf(out, "\n");
  fprintf(out, "\n");
  
  /*
   * write out nsimples_per_complex
   */
  
  for (icomplex = 0; icomplex < n_complex_tracks; icomplex++) {
    
    complex_num = complex_track_nums[icomplex];
    
    fprintf(out, "%sSIMPLES_PER_COMPLEX, track %d: ", spacer, complex_num);
    nsimples = nsimples_per_complex[complex_num];
    for (i = 0; i < nsimples; i++) {
      fprintf(out, "%d ",
	      (int) simples_per_complex[complex_num][i]);
    }
    fprintf(out, "\n");
  } /* icomplex */

  fprintf(out, "\n\n");

  return;

}

/*-------------------------------
 */

void RfPrintTrackParams(FILE *out,
			const char *spacer,
			const track_file_params_t *params)
     
{
  
  si32 i;

  fprintf(out, "%sTRACK FILE PARAMETERS:\n", spacer);

  if (params->grid_type == TITAN_PROJ_FLAT) {
    fprintf(out, "%s  Gridtype : flat\n", spacer);
  } else if (params->grid_type == TITAN_PROJ_LATLON) {
    fprintf(out, "%s  Gridtype : latlon\n", spacer);
  }

  fprintf(out, "%s  Nweights_forecast : %ld\n", spacer,
	  (long) params->nweights_forecast);
  fprintf(out, "%s  Forecast type : %s\n", spacer,
	  FORECAST_TYPE_STR(params->forecast_type));

  fprintf(out, "%s  Forecast weights :", spacer);
  for (i = 0; i < params->nweights_forecast; i++) {
    fprintf(out, " %.2f",
	    params->forecast_weights[i]);
  }
  fprintf(out, "\n");
  
  fprintf(out, "%s  Parabolic_growth_period : %g\n", spacer,
	  params->parabolic_growth_period);
  
  fprintf(out, "%s  Weight_distance : %g\n", spacer,
	  params->weight_distance);
  
  fprintf(out, "%s  Weight_delta_cube_root_volume : %g\n", spacer,
	  params->weight_delta_cube_root_volume);
  
  fprintf(out, "%s  Max tracking speed (km/hr) : %g\n", spacer,
	  params->max_tracking_speed);

  fprintf(out, "%s  Max delta time (secs) : %ld\n", spacer,
	  (long) params->max_delta_time);

  fprintf(out, "%s  Min history for valid forecast (secs) : %ld\n", spacer,
	  (long) params->min_history_for_valid_forecast);

  fprintf(out, "%s  Max speed for valid forecast (km/hr) : %g\n", spacer,
	  params->max_speed_for_valid_forecast);

  fprintf(out, "%s  Spatial smoothing : %s\n", spacer,
	  (params->spatial_smoothing? "TRUE" : "FALSE"));

  fprintf(out, "%s  Use_runs_for_overlaps : %s\n", spacer,
	  (params->use_runs_for_overlaps? "TRUE" : "FALSE"));

  fprintf(out, "%s  Scale_forecasts_by_history : %s\n", spacer,
	  (params->scale_forecasts_by_history? "TRUE" : "FALSE"));
  
  fprintf(out, "%s  Smoothing radius (km) : %g\n", spacer,
	  params->smoothing_radius);

  fprintf(out, "%s  Min_fraction_overlap : %g\n", spacer,
	  params->min_fraction_overlap);

  fprintf(out, "%s  Min_sum_fraction_overlap : %g\n", spacer,
	  params->min_sum_fraction_overlap);

  fprintf(out, "\n");

  return;
  
}

/*---------------------------------------
 * RfPrintTrackVerify()
 */

void RfPrintTrackVerify(FILE *out,
			const char *spacer,
			const track_file_verify_t *verify)
     
{

  char spacer2[128];

  fprintf(out, "%sVerification params :\n", spacer);
  
  fprintf(out, "%s  forecast_lead_time (secs) : %ld\n", spacer,
	  (long) verify->forecast_lead_time);
  
  fprintf(out, "%s  forecast_lead_time_margin (secs) : %ld\n", spacer,
	  (long) verify->forecast_lead_time_margin);
  
  fprintf(out, "%s  forecast_min_history (secs) : %ld\n", spacer,
	  (long) verify->forecast_min_history);
  
  fprintf(out, "%s  verify_before_forecast_time : %s\n", spacer, 
	  BOOL_STR(verify->verify_before_forecast_time));
  
  fprintf(out, "%s  verify_after_track_dies : %s\n", spacer, 
	  BOOL_STR(verify->verify_after_track_dies));
  
  sprintf(spacer2, "%s  ", spacer);
  TITAN_print_grid(out, spacer2, &verify->grid);
  
}

    
