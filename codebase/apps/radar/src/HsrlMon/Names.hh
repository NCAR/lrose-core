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
/////////////////////////////////////////////////////////////
// Names.hh
//
// Strings for names in NetCDF
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2017
//
/////////////////////////////////////////////////////////////

#ifndef _NAMES_HH
#define _NAMES_HH

#include <string>
using namespace std;

class Names {
  
public:
  
  // constructor

  Names ();

  // Destructor

  ~Names();

  // strings for names

  static const string CombinedHighCounts;
  static const string CombinedLowCounts;
  static const string lidar_copolar_combined_backscatter_photon_count;

  static const string MolecularCounts;
  static const string lidar_copolar_molecular_backscatter_photon_count;

  static const string CrossPolarCounts;
  static const string lidar_crosspolar_combined_backscatter_photon_count;

  static const string VolumeDepolRatio;
  static const string lidar_volume_depolarization_ratio;

  static const string BackScatterRatio;
  static const string lidar_backscatter_ratio;

  static const string ParticleDepolRatio;
  static const string lidar_particle_depolarization_ratio;

  static const string BackScatterCoeff;
  static const string lidar_backscatter_coefficient;

  static const string ExtinctionCoeff;
  static const string lidar_extinction_coefficient;

  static const string OpticalDepth;
  static const string lidar_optical_depth;

  static const string Height;
  static const string height_above_mean_sea_level;
  
  static const string Temperature;
  static const string air_temperature;
  
  static const string Pressure;
  static const string air_pressure;
  static const string pressure_from_std_atmos;

protected:
private:

};

#endif



