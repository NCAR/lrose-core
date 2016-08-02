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
/***************************************************************************
 * load_props.c
 *
 * Loads up the storm props
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1995
 *
 ****************************************************************************/

#include "storm_ident.h"

void load_gprops(storm_file_global_props_t *gprops,
		 si32 storm_num,
		 si32 n_layers,
		 si32 base_layer,
		 si32 n_dbz_intvls,
		 int range_limited,
		 int top_missing,
		 int hail_present,
		 int second_trip)

{

  /*
   * load up global storm properties
   */
  
  gprops->storm_num = storm_num;
  gprops->n_layers = n_layers;
  gprops->base_layer = base_layer;
  gprops->n_dbz_intervals = n_dbz_intvls;
  
  gprops->range_limited = range_limited;
  gprops->top_missing = top_missing;
  gprops->hail_present = hail_present;
  gprops->second_trip = second_trip;
  
  return;
  
}

void load_lprops(layer_stats_t *layer,
		 storm_file_layer_props_t *lprops)
     
{
  
  lprops->vol_centroid_x = layer->vol_centroid_x;
  lprops->vol_centroid_y = layer->vol_centroid_y;
  lprops->refl_centroid_x = layer->refl_centroid_x;
  lprops->refl_centroid_y = layer->refl_centroid_y;
  
  lprops->area = layer->area;
  lprops->dbz_max = layer->dbz_max;
  lprops->dbz_mean = layer->dbz_mean;
  lprops->mass = layer->mass;
  lprops->rad_vel_mean = layer->vel_mean;
  lprops->rad_vel_sd = layer->vel_sd;
  lprops->vorticity = layer->vorticity;
  
  return;

}

void load_dbz_hist(dbz_hist_entry_t *dbz_hist,
		   storm_file_dbz_hist_t *hist)

{

  hist->percent_volume = dbz_hist->percent_vol;
  hist->percent_area = dbz_hist->percent_area;

}
      
      
