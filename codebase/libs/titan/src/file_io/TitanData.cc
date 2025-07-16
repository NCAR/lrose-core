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
////////////////////////////////////////////////////////////////
// TitanData.cc
//
// Titan data classes, structs etc.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2025.
//
////////////////////////////////////////////////////////////////

#include <cassert>
#include <titan/TitanData.hh>

using namespace std;

////////////////////////////////////////////////////////////
// StormParams constructor

TitanData::StormParams::StormParams()

{

  low_dbz_threshold = missingFl32;
  high_dbz_threshold = missingFl32;
  dbz_hist_interval = missingFl32;
  hail_dbz_threshold = missingFl32;
  base_threshold = missingFl32;
  top_threshold = missingFl32;
  min_storm_size = missingFl32;
  max_storm_size = missingFl32;
  morphology_erosion_threshold = missingFl32;
  morphology_refl_divisor = missingFl32;
  min_radar_tops = missingFl32;
  tops_edge_margin = missingFl32;
  z_p_coeff = missingFl32;
  z_p_exponent = missingFl32;
  z_m_coeff = missingFl32;
  z_m_exponent = missingFl32;
  sectrip_vert_aspect = missingFl32;
  sectrip_horiz_aspect = missingFl32;
  sectrip_orientation_error = missingFl32;
  poly_start_az = missingFl32;
  poly_delta_az = missingFl32;
  ltg_count_time = missingFl32;
  ltg_count_margin_km = missingFl32;
  hail_z_m_coeff = missingFl32;
  hail_z_m_exponent = missingFl32;
  hail_mass_dbz_threshold = missingFl32;
  tops_dbz_threshold = missingFl32;
  precip_plane_ht = missingFl32;
  precip_computation_mode = PRECIP_FROM_COLUMN_MAX;

}

