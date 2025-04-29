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
///////////////////////////////////////////////////////////////
// GlobalData.cc
//
// Global data for Lucid display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Fen 2025
//
///////////////////////////////////////////////////////////////

#include "GlobalData.hh"
#include <toolsa/toolsa_macros.h>

// singleton instance is global

GlobalData *GlobalData::_instance = (GlobalData *) NULL;

using namespace std;

//////////////////////////////////////////////////////
// Constructor

GlobalData::GlobalData()

{
  
  hcan_pdev = nullptr;
  vcan_pdev = nullptr;
    
  debug = 0;
  debug1 = 0;
  debug2 = 0;
    
  display_projection = 0;
  quiet_mode = 0;   
  report_mode = 0;   
  run_unmapped = 0;   
  use_cosine_correction = 0;
  drawing_mode = 0;
  MEM_zero(product_detail_threshold);
  MEM_zero(product_detail_adjustment);

  mark_latest_client_location = 0; 
  forecast_mode = 0;     
  data_format = 0; 
  
  num_colors = 0;       
  num_draw_colors = 0;  
  map_overlay_color_index_start = 0;
  finished_init = 0;    

  num_datafields = 0;   
  num_menu_fields = 0;  
  num_field_menu_cols = 0;  
  num_map_overlays = 0; 
  num_render_heights = 0;
  num_cache_zooms = 0;
  cur_render_height = 0; 
  cur_field_set = 0;     
  save_im_win = 0;       
  image_needs_saved = 0; 
  generate_filename = 0; 
  max_time_list_span = 0; 

  pan_in_progress = 0;    
  zoom_in_progress = 0;   
  route_in_progress = 0;  
  data_timeout_secs = 0;  
  data_status_changed = 0;
  series_save_active = 0; 

  num_field_labels = 0;
  MEM_zero(field_index);
  movieframe_time_mode = 0; 
  aspect_correction = 1.0;
  MEM_zero(height_array);

  redraw_vert = 0;
  time_has_changed = 0;
  field_has_changed = 0;
  zoom_has_changed = 0;
  vsect_has_changed = 0;
  ht_has_changed = 0;

  prev_time = 0;
  prev_field = 0;
  prev_ht = 0;

  selected_time = 0;
  selected_field = 0;
  selected_ht = 0;

  last_event_time = 0;  
  epoch_start = 0;      
  epoch_end  = 0;       
  model_run_time = 0;  
  data_request_time = 0; 

  projection_type = nullptr;
  MEM_zero(proj_param);
  
  demo_time = nullptr;

  orig_wd = nullptr;           
  frame_label = nullptr;          
  app_name = nullptr;          
  app_instance = nullptr;      

  MEM_zero(data_info);

  prod_mgr = nullptr;
  
  MEM_zero(mread);

  // MEM_zero(io_info);
  
  r_context = nullptr;    
  station_loc = nullptr;    
  remote_ui = nullptr;   
  
  coord_key = 0;
  coord_expt = nullptr;

  h_copy_flag = 0;
  v_copy_flag = 0;

}

//////////////////////////////////////////
// destructor

GlobalData::~GlobalData()

{

}

