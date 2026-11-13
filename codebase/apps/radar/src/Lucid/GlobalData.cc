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
  
  debug = 0;
  debug1 = 0;
  debug2 = 0;
    
  display_projection = 0;
  quiet_mode = 0;   
  run_unmapped = 0;   
  use_cosine_correction = 0;
  num_datafields = 0;   
  num_map_overlays = 0; 
  aspect_correction = 1.0;
  epoch_start = 0;      
  epoch_end  = 0;       
  model_run_time = 0;  
  MEM_zero(proj_param);

  orig_wd = nullptr;           
  app_name = nullptr;          
  app_instance = nullptr;      

  prod_mgr = nullptr;
  wind_mgr = nullptr;
  
  MEM_zero(mread);

  // MEM_zero(io_info);
  
  coord_key = 0;
  coord_expt = nullptr;

}

//////////////////////////////////////////
// destructor

GlobalData::~GlobalData()

{

}

