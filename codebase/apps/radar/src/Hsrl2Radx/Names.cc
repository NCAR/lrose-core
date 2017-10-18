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
//////////////////////////////////////////////////////////
// Names.cc
//
// Strings for field names etc.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2017
//
/////////////////////////////////////////////////////////////

#include "Names.hh"

const string Names::CombinedHighCounts = "CombinedHighCounts";
const string Names::CombinedLowCounts = "CombinedLowCounts";
const string Names::lidar_copolar_combined_backscatter_photon_count =
  "lidar_copolar_combined_backscatter_photon_count";

const string Names::MolecularCounts = "MolecularCounts";
const string Names::lidar_copolar_molecular_backscatter_photon_count =
  "lidar_copolar_molecular_backscatter_photon_count";

const string Names::CrossPolarCounts = "CrossPolarCounts";
const string Names::lidar_crosspolar_combined_backscatter_photon_count =
  "lidar_crosspolar_combined_backscatter_photon_count";

const string Names::VolumeDepolRatio = "VolumeDepolRatio";
const string Names::lidar_volume_depolarization_ratio =
  "lidar_volume_depolarization_ratio";

const string Names::BackScatterRatio = "BackScatterRatio";
const string Names::lidar_backscatter_ratio = "lidar_backscatter_ratio";

const string Names::ParticleDepolRatio = "ParticleDepolRatio";
const string Names::lidar_particle_depolarization_ratio =
  "lidar_particle_depolarization_ratio";

const string Names::BackScatterCoeff = "BackScatterCoeff";
const string Names::lidar_backscatter_coefficient = "lidar_backscatter_coefficient";

const string Names::Extinction = "Extinction";
const string Names::lidar_extinction_coefficient = "lidar_extinction_coefficient";

const string Names::OpticalDepth = "OpticalDepth";
const string Names::lidar_optical_depth = "lidar_optical_depth";

const string Names::height = "height";
const string Names::height_above_mean_sea_level = "height_above_mean_sea_level";

const string Names::temperature = "temperature";
const string Names::air_temperature = "air_temperature";

const string Names::pressure = "pressure";
const string Names::air_pressure = "air_pressure";
const string Names::pressure_from_std_atmos = "pressure_from_std_atmos";

