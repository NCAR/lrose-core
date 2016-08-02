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
 *
 * @file RadarFile.hh
 *
 * @class RadarFile
 *
 * Class controlling access to a DeTect radar file.
 *  
 * @date 7/27/2010
 *
 */

#ifndef RadarFile_hh
#define RadarFile_hh

#include <iostream>
#include <vector>

#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>

using namespace std;


class RadarFile
{

public:

  /**
   * @brief Constructor
   *
   * @param[in] input_path The input file path.
   * @param[in] debug Debug flag.
   */

  RadarFile(const string &input_path,
	     const bool debug = false);
  

  /**
   * @brief Destructor
   */

  virtual ~RadarFile();


  //////////////////////////
  // Input/output methods //
  //////////////////////////


  /**
   * @brief Read the input file.
   *
   * @return Returns true on success, false on failure.
   */

  bool readFile();
  

  /**
   * @brief Print the file information to the given stream.
   *
   * @param[in,out] stream The output stream.
   * @param[in] leader String to print at the start of every line.
   */

  void print(ostream &stream, const string &leader) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the number of rays in the tilt.
   *
   * @return Returns the number of rays in the tilt.
   */

  size_t getNumRays() const
  {
    return _numRays;
  }
  

  /**
   * @brief Get the number of gates in the data.
   *
   * @return Returns the number of gates in the data.
   */

  size_t getNumGates() const
  {
    return _numGates;
  }
  

  /**
   * @brief Get the radar altitude in kilometers.
   *
   * @return Returns the radar altitude in kilometers.
   */

  double getRadarAltKm() const
  {
    return _height / 1000.0;
  }
  

  /**
   * @brief Get the radar latitude.
   *
   * @return Returns the radar latitude.
   */

  double getRadarLat() const
  {
    return _lat;
  }
  

  /**
   * @brief Get the radar longitude.
   *
   * @return Returns the radar longitude.
   */

  double getRadarLon() const
  {
    return _lon;
  }
  

  /**
   * @brief Get the tilt number.
   *
   * @return Returns the tilt number.
   */
  
  size_t getTiltNum() const
  {
    return _tiltNum;
  }
  

  /**
   * @brief Get the gate spacing in kilometers.
   *
   * @return Returns the gate spacing in kilometers.
   */

  double getGateSpacingKm() const
  {
    return _gateSpacing / 1000.0;
  }
  

  /**
   * @brief Get the start range in kilometers.
   *
   * @return Returns the start range in kilometers.
   */

  double getStartRangeKm() const
  {
    return _gate1 / 1000.0;
  }
  

  /**
   * @brief Get the missing data value.
   *
   * @return Returns the missing data value.
   */

  double getMissingDataValue() const
  {
    return _missingDataValue;
  }
  

  /**
   * @brief Get the indicated ray from the data.
   *
   * @param[in] ray_index The ray index.
   * @param[out] azimuth The azimuth angle.
   * @param[out] elev_angle The elevation angle.
   * @param[out] data_time The data time.
   * @param[out] ray_data The ray data.
   *
   * @return Returns true on success, false on failure.
   */

  bool getRay(const size_t ray_index,
	      double &azimuth,
	      double &elev_angle,
	      DateTime &data_time,
	      vector< double > &ray_data);
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    ui32 radar_name[4];
    si32 header_scale;
    si32 height;
    si32 lat;
    si32 lon;
    si32 year;
    si32 month;
    si32 day;
    si32 hour;
    si32 minute;
    si32 second;
    si32 vol_num;
    si32 vcp_mode;
    si32 tilt_num;
    si32 elev_angle;
    si32 num_rays;
    si32 num_gates;
    si32 azim1;
    si32 azim_spacing;
    si32 gate1;
    si32 gate_spacing;
    si32 data_scale;
    si32 missing_data_value;
    si32 spare[14];
  } radar_header_t;
  
    
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;

  /**
   * @brief The full path for the input file.
   */

  string _inputPath;
  
  /**
   * @brief The input file pointer.
   */

  FILE *_inputFile;
  
  /**
   * @brief The name of the radar.
   */

  string _radarName;
  
  /**
   * @brief Height.
   */

  double _height;

  /**
   * @brief Radar latitude.
   */

  double _lat;
  
  /**
   * @brief Radar longitude.
   */

  double _lon;
  
  /**
   * @brief Data time.
   */

  DateTime _dataTime;
  
  /**
   * @brief Volume number.
   */

  size_t _volNum;
  
  /**
   * @brief VCP mode.
   */

  int _vcpMode;
  
  /**
   * @brief Tilt number.
   */

  size_t _tiltNum;
  
  /**
   * @brief Elevation angle.
   */

  double _elevAngle;
  
  /**
   * @brief Number of rays.   */

  size_t _numRays;
  
  /**
   * @brief Number of gates.
   */

  size_t _numGates;
  
  /**
   * @brief First azimuth in the data.
   */

  double _azim1;
  
  /**
   * @brief Azimuth spacing.
   */

  double _azimSpacing;
  
  /**
   * @brief First gate in the data.
   */

  double _gate1;
  
  /**
   * @brief Gate spacing.
   */

  double _gateSpacing;
  
  /**
   * @brief Missing data value.
   */

  int _missingDataValue;
  
  /**
   * @brief The tilt data.
   */

  double *_tiltData;
  
};

#endif
