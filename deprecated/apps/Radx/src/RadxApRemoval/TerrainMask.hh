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
/**
 * @file TerrainMask.hh
 *
 * @class TerrainMask
 *
 * TerrainMask creates a terrain mask for the current radar dataset.
 *  
 * @date 7/22/2002
 */

#ifndef TerrainMask_HH
#define TerrainMask_HH

#include <string>
#include <vector>

#include <dataport/port_types.h>

#include <Mdv/DsMdvx.hh>

/** 
 * @class TerrainMask
 */

class TerrainMask 
{
public:

  //////////////////////
  // Public constants //
  //////////////////////

  /** 
   * @brief Terrain mask value used to indicate fuzzy data.
   */

  static const float  FUZZY_VALUE;

  /** 
   * @brief Terrain mask value used to indicate land data.
   */

  static const float  LAND_VALUE;

  /** 
   * @brief Terrain mask value used to indicate water data.
   */

  static const float  WATER_VALUE;
   
  /** 
   * @brief Terrain mask value used to indicate missing data.
   */

  static const float  MISSING_VALUE;
   

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  TerrainMask();

  /**
   * @brief Destructor
   */

  ~TerrainMask();
   
  /**
   * @brief Initialize the object information.
   *
   * @param[in] terrain_url URL for the MDV dataset containing the terrain
   *                        information.
   * @param[in] terrain_field_name Field name of the terrain field in the
   *                               terrain dataset.
   * @param[in] max_num_gates The maximum number of gates in any tilt in the
   *                          input radar data.
   * @param[in] gate_spacing Gate spacing of the input radar data in km.
   * @param[in] delta_az Beam width of input radar data in degrees.
   * @param[in] radar_lat Latitude for the radar.
   * @param[in] radar_lon Longitude for the radar.
   */

  int init(const char* terrain_url, const char* terrain_field_name,
	   const int max_num_gates, const double gate_spacing,
	   const double delta_az,
	   const double radar_lat, const double radar_lon);

  /**
   * @brief Write the terrain mask data to the given file path.
   *
   * @param[in] file_path Output file path.
   */

  void writeMask(const char* file_path) const;

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the terrain mask data.
   *
   * @return Returns a pointer to the terrain mask data.  This pointer is
   *         owned by the TerrainMask object and should not be deleted by
   *         the calling method.
   */

  fl32 *getMask() const { return _terrainMask; }

  /**
   * @brief Get the number of gates in the terrain data.
   *
   * @return Returns the number of gates in the terrain data.
   */

  int getNumGates() const { return _nGates; }

  /**
   * @brief Get the number of beams in the terrain data.
   *
   * @return Returns the number of beams in the terrain data.
   */

  int getNumBeams() const { return _nBeams; }

private:

  /////////////////////
  // Private members //
  /////////////////////

  /** 
   * @brief Terrain mask data.  There is a value for every gate in every
   *        beam.  The values are FUZZY_VALUE, LAND_VALUE or WATER_VALUE.
   */

  fl32 *_terrainMask;

  /** 
   * @brief Number of gates in the terrain mask data.
   */

  int _nGates;

  /** 
   * @brief Number of beams in the terrain mask data.
   */

  int _nBeams;

  /** 
   * @brief Gate spacing in the terrain mask data.
   */

  double _gateSpacing;

  /** 
   * @brief Delta azimuth (beam width) in the terrain mask data.
   */

  double _deltaAzimuth;

  /** 
   * @brief Radar latitude.
   */

  double _radarLat;

  /** 
   * @brief Radar longitude.
   */

  double _radarLon;

  /** 
   * @brief Input terrain file.
   */

  DsMdvx _inputMdvx;

};

#endif
