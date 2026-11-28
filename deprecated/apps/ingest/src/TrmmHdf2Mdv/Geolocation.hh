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
 * @file Geolocation.hh
 *
 * @class Geolocation
 *
 * Geolocation information for a TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#ifndef Geolocation_HH
#define Geolocation_HH

#include <string>
#include <vector>

#include "HdfFile.hh"
#include "SdsField.hh"

using namespace std;

/** 
 * @class Geolocation
 */

class Geolocation : public SdsField
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const double MISSING_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  Geolocation(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~Geolocation(void);
  

  /**
   * @brief Initial the geolocation information from the current TRMM file.
   *
   * @param[in] hdf_file The current TRMM file.
   */

  bool init(HdfFile &hdf_file);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Tell the class to read the solar calibration data from the
   *        HDF file.  Must be called before calling init() to have any
   *        effect.
   */

  inline void setReadMagData()
  {
    _readSunMagDataFlag = true;
  }
  

  /**
   * @brief Get the number of scans in the TRMM data.
   *
   * @return Returns the number of scans in the data.
   */

  inline int getNumScans() const
  {
    return _numScans;
  }
  

  /**
   * @brief Get the number of pixels in the TRMM data.
   *
   * @return Returns the number of pixels in the data.
   */

  inline int getNumPixels() const
  {
    return _numPixels;
  }
  

  /**
   * @brief Get the latitude for the given data index.
   *
   * @return Returns the latitude value.
   */

  inline double getLat(const int i) const
  {
    return _locations[i].lat;
  }
  

  /**
   * @brief Get the longitude for the given data index.
   *
   * @return Returns the longitude value.
   */

  inline double getLon(const int i) const
  {
    return _locations[i].lon;
  }
  

  /**
   * @brief Get the scan time for the given data index.
   *
   * @return Returns the scan time.
   */

  inline time_t getScanTime(const int i) const
  {
    return _locations[i].scan_time;
  }
  

  /**
   * @brief Get the sun magnitude for the given data index.
   *
   * @return Returns the sun magnitude.
   */

  inline double getSunMag(const int i) const
  {
    return _locations[i].sun_mag;
  }
  

  /**
   * @brief Get the solar zenith for the given data index.
   *
   * @return Returns the solar zenith.
   */

  inline double getSolarZenith(const int i) const
  {
    return _locations[i].solar_zenith;
  }
  

  /**
   * @brief Get the cosine of the solar zenith for the given data index.
   *
   * @return Returns the cosine of the solar zenith.
   */

  inline double getCosSolarZenith(const int i) const
  {
    return _locations[i].cos_solar_zenith;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Name of the geolocation SDS field in the TRMM datasets.
   */

  static const string GEOLOCATION_SDS_FIELD_NAME;

  /**
   * @brief Name of the local direction SDS field in the TRMM datasets.
   *        This is the field than contains the solar zenith values.
   */

  static const string LOCAL_DIR_SDS_FIELD_NAME;

  /**
   * @brief The number of pixels represented by a single angle in the local
   *        direction data in the TRMM datasets.
   */

  static const int LOCAL_DIR_PIXELS_PER_ANGLE;
  

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double lat;
    double lon;
    time_t scan_time;
    double sun_mag;
    double solar_zenith;
    double cos_solar_zenith;
  } location_t;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Flag indicating whether to try to retieve the solar calibration
   *        data from the HDF file.
   */

  bool _readSunMagDataFlag;
  
  /**
   * @brief Number of scans in the TRMM data.
   */

  int _numScans;
  

  /**
   * @brief Number of pixels in the TRMM data.
   */

  int _numPixels;
  

  /**
   * @brief List of locations of data points in this TRMM file.  The locations
   *        are ordered by [scan][pixel].
   */

  vector< location_t > _locations;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Read the geolocation data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readGeolocationData(const HdfFile &hdf_file);
  

  /**
   * @brief Read the scan time data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readScanTimeData(HdfFile &hdf_file);
  

  /**
   * @brief Read the solar magnitude data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readSunMagData(HdfFile &hdf_file);
  

  /**
   * @brief Read the solar zenith data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readSolarZenithData(const HdfFile &hdf_file);
  

};


#endif
