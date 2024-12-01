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
///////////////////////////////////////////////////////////////
// MetRecord.cc
//
// Field data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include "MetRecord.hh"
#include <qtplot/ColorMap.hh>

///////////////////////////////////////////////
// constructor

MetRecord::MetRecord()

{
  
  // initialize
  
  plane = 0;
  currently_displayed = 0;
  auto_render = 0;
  render_method = 0;
  composite_mode = 0;
  auto_scale = 0;
  use_landuse = 0;
  num_display_pts = 0;
  last_collected = 0;
  h_data_valid = 0;
  v_data_valid = 0;
  time_list_valid = 0;
  vert_type = 0;
  alt_offset = 0.0;
  detail_thresh_min = 0.0;
  detail_thresh_max = 0.0;
  
  // MEM_zero(vert);
  
  ht_pixel = 0.0;
  y_intercept = 0.0;
  
  last_elev = NULL;
  elev_size = 0;
  
  h_last_scale = 0.0;
  h_last_bias = 0.0;
  h_last_missing = 0.0;
  h_last_bad = 0.0;
  h_last_transform = 0;
  v_last_scale = 0.0;
  v_last_bias = 0.0;
  v_last_missing = 0.0;
  v_last_bad = 0.0;
  v_last_transform = 0;
  
  cscale_min = 0.0;
  cscale_delta = 0.0;
  overlay_min = 0.0;
  overlay_max = 0.0;
  cont_low = 0.0;
  cont_high = 0.0;
  cont_interv = 0.0;
  
  time_allowance = 0.0;
  time_offset = 0.0;
  
  MEM_zero(units_label_cols);
  MEM_zero(units_label_rows);
  MEM_zero(units_label_sects);
  MEM_zero(vunits_label_cols);
  MEM_zero(vunits_label_rows);
  MEM_zero(vunits_label_sects);
  MEM_zero(field_units);
  MEM_zero(button_name);
  MEM_zero(legend_name);
  MEM_zero(field_label);
  MEM_zero(url);
  MEM_zero(color_file);
  
  h_data = NULL;
  v_data = NULL;
  
  h_fl32_data = NULL;
  v_fl32_data = NULL;
  
  h_date.setToNever();
  v_date.setToNever();
  
  proj = NULL;
  
  h_mdvx = NULL;
  h_mdvx_int16 = NULL;
  
  MEM_zero(h_mhdr);
  MEM_zero(h_fhdr);
  MEM_zero(h_vhdr); 
  
  MEM_zero(ds_fhdr);
  MEM_zero(ds_vhdr);
  
  v_mdvx = NULL;
  v_mdvx_int16 = NULL;
  
  MEM_zero(v_mhdr);
  MEM_zero(v_fhdr);
  MEM_zero(v_vhdr);
  
  colorMap = NULL;
  
}

