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
/**************************************************************************
 * load_props.c
 *
 * loads the forecast vals struct, computing the forecast based upon
 * the current storm.
 *
 * The routine tests to see if the forecast is valid. It is considered
 * valid if the forecast volume exceeds the volume threshold.
 *
 * Returns TRUE for valid forecast, FALSE otherwise
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 *************************************************************************/

#include "verify_tracks.h"

int load_props(storm_file_handle_t *s_handle,
	       track_file_handle_t *t_handle,
	       track_file_entry_t *entry,
	       storm_file_global_props_t *gprops,
	       double lead_time,
	       track_file_forecast_props_t *f_props)

{

  long i;

  double dval_dt, delta_val;
  double f_volume;
  double lead_time_hr;
  double growth_period_hr;
  double forecast_area, current_area;
  double darea_dt;
  double f_props_val;

  fl32 *current_ptr;
  fl32 *f_props_ptr;
  fl32 *dval_dt_ptr;

  track_file_forecast_props_t current;
  track_file_forecast_props_t *fprops;
  
  fprops = &entry->dval_dt;
  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = Glob->forecast_growth_period / 3600.0;

  /*
   * compute the forecast area
   */

  current_area = gprops->proj_area;
  darea_dt = fprops->proj_area;
  forecast_area = current_area + lead_time_hr * darea_dt;

  /*
   * return now if the forecast area is less than 1
   */

  if (forecast_area < 1.0)
    return (FALSE);
  
  /*
   * set current vals in struct
   */

  current.dbz_max = gprops->dbz_max;
  current.volume = gprops->volume;
  current.precip_flux = gprops->precip_flux;
  current.mass = gprops->mass;
  current.proj_area = gprops->proj_area;

  /*
   * weight the posiitional parameters by volume
   */

  current.proj_area_centroid_x = gprops->proj_area_centroid_x;
  current.proj_area_centroid_y = gprops->proj_area_centroid_y;
  current.vol_centroid_z = gprops->vol_centroid_z;
  current.refl_centroid_z = gprops->refl_centroid_z;
  current.top = gprops->top;

  /*
   * compute forecast volume
   */

  f_volume = (current.volume + fprops->volume * lead_time_hr);

  /* 
   * return FALSE if volume less than 1 
   * *rjp 10 Apr 2002
   * Note This needs to be checked further. 
   * fprops->volume should be not be negative at this point.
   */

  if (f_volume < 1.0)
    return(FALSE) ;    


  /*
   * move through the structs, using pointers to the
   * data elements
   */
  
  current_ptr = (fl32 *) &current;
  f_props_ptr = (fl32 *) f_props;
  dval_dt_ptr = (fl32 *) fprops;

  for (i = 0; i < N_MOVEMENT_PROPS_FORECAST; i++) {
    
    /*
     * compute the forecast movement props based on
     * current vals and forecast
     * rates of change - these are weighted by volume
     */
    
    delta_val = *dval_dt_ptr * lead_time_hr;
    f_props_val = (*current_ptr + delta_val);
    
    *f_props_ptr += f_props_val * f_volume;
    
    current_ptr++;
    f_props_ptr++;
    dval_dt_ptr++;
    
  } /* i */
  
  for (i = 0; i < N_GROWTH_PROPS_FORECAST; i++) {
    
    /*
     * compute the forecast growth props based on current vals and forecast
     * rate of change. If forecast value goes negative, set to zero.
     */
    
    dval_dt = *dval_dt_ptr;

    if (Glob->parabolic_growth) {

      /*
       * parabolic growth, linear decay
       */

      if (dval_dt < 0.0) {
	
	/*
	 * linear decay - negative growth rate
	 */
	
	delta_val = dval_dt * lead_time_hr;
	
      } else {
	
	/*
	 * parabolic growth curve - positive growth rate
	 */
	
	delta_val = (dval_dt * lead_time_hr -
		     (dval_dt * lead_time_hr * lead_time_hr) /
		     (growth_period_hr * 2.0));

      }
	
    } else {
      
      /*
       * linear growth / decay
       */
      
      delta_val = dval_dt * lead_time_hr;
      
    } /* if (Glob->parabolic_growth) */
      
    f_props_val = (*current_ptr + delta_val);
    
    if (f_props_val < 0.0)
      f_props_val = 0.0;
    
    *f_props_ptr += f_props_val;
    
    current_ptr++;
    f_props_ptr++;
    dval_dt_ptr++;
    
  } /* i */
  
  return (TRUE);

}
