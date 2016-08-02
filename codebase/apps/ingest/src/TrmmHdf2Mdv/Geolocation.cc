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
 * @file Geolocation.cc
 *
 * @class Geolocation
 *
 * Geolocation information for a TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#include <iostream>

#include <rapmath/math_macros.h>

#include "Geolocation.hh"

using namespace std;


// Global constants

const double Geolocation::MISSING_VALUE = -9999.0;

const string Geolocation::GEOLOCATION_SDS_FIELD_NAME = "geolocation";
const string Geolocation::LOCAL_DIR_SDS_FIELD_NAME = "localDirection";
const int Geolocation::LOCAL_DIR_PIXELS_PER_ANGLE = 10;

/*********************************************************************
 * Constructors
 */

Geolocation::Geolocation(const bool debug_flag) :
  SdsField(debug_flag),
  _readSunMagDataFlag(false),
  _numScans(-1),
  _numPixels(-1)
{
}


/*********************************************************************
 * Destructor
 */

Geolocation::~Geolocation()
{
}


/*********************************************************************
 * init()
 */

bool Geolocation::init(HdfFile &hdf_file)
{
  static const string method_name = "Geolocation::init()";
  
  // Read the geolocation data

  if (!_readGeolocationData(hdf_file))
    return false;
  
  // Read the scan time data.  Note that this must be done after reading
  // the geolocation data becuase this method assumes that the records
  // have been initialized.

  if (!_readScanTimeData(hdf_file))
    return false;
  
  if (_readSunMagDataFlag)
  {
    // Read the sun magnitude data from the file

    if (!_readSunMagData(hdf_file))
      return false;

    // Read the solar zenith data from the file

    if (!_readSolarZenithData(hdf_file))
      return false;
  }
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _readGeolocationData()
 */

bool Geolocation::_readGeolocationData(const HdfFile &hdf_file)
{
  static const string method_name = "Geolocation::_readGeolocationData()";
  
  // Read the geolocation data from the TRMM file

  float64 *data;
  vector< int > dimensions;
  
  if ((data =
       _readData(hdf_file, GEOLOCATION_SDS_FIELD_NAME, dimensions)) == 0)
    return false;
  
  // Check for errors in the data

  if (dimensions.size() != 3)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong rank for geolocation dataset: "
	 << dimensions.size() << endl;
    
    delete [] data;
    
    return false;
  }
  
  if (dimensions[2] != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong dimension size for data: " << dimensions[2] << endl;
    
    delete [] data;
    
    return false;
  }
  
  // Set the dataset dimensions

  _numScans = dimensions[0];
  _numPixels = dimensions[1];
  
  // Set the locations

  int num_locations = _numScans * _numPixels;
  
  int index = 0;
  
  for (int loc = 0; loc < num_locations; ++loc)
  {
    location_t location;
    
    location.lat = data[index++];
    location.lon = data[index++];
    location.scan_time = DateTime::NEVER;
    location.sun_mag = MISSING_VALUE;
    location.solar_zenith = MISSING_VALUE;
    location.cos_solar_zenith = MISSING_VALUE;
    
    _locations.push_back(location);
  } /* endfor - i */
  
  delete [] data;
  
  return true;
}


/*********************************************************************
 * _readScanTimeData()
 */

bool Geolocation::_readScanTimeData(HdfFile &hdf_file)
{
  static const string method_name = "Geolocation::_readScanTimeData()";
  
  // Get the scan times from the file.  If the vector is empty, there
  // was an error.

  const vector< DateTime > &scan_times = hdf_file.getScanTimes();

  if (scan_times.size() == 0)
    return false;
  
  if ((int)scan_times.size() != _numScans)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error in scan times in HDF file" << endl;
    cerr << "Expected " << _numScans << " scan times based on geolocation data" << endl;
    cerr << "Found " << scan_times.size() << " scan times" << endl;
    
    return false;
  }
  
  // Loop through the geolocation records, updating the scan times as
  // appropriate.

  for (int scan = 0, geo_index = 0; scan < _numScans; ++scan)
  {
    for (int pixel = 0; pixel < _numPixels; ++pixel, ++geo_index)
    {
      _locations[geo_index].scan_time = scan_times[scan].utime();
    } /* endfor - pixel */
  } /* endfor - scan */
  
  return true;
}


/*********************************************************************
 * _readSunMagData()
 */

bool Geolocation::_readSunMagData(HdfFile &hdf_file)
{
  static const string method_name = "Geolocation::_readSunMagData()";
  
  // Get the sun magnitudes from the file.  If the vector is empty, there
  // was an error.

  vector< double > sun_mags;

  if (!hdf_file.getSunMags(sun_mags))
    return false;

  if (sun_mags.size() == 0)
    return false;
  
  if ((int)sun_mags.size() != _numScans)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error in solar calibration data in HDF file" << endl;
    cerr << "Expected " << _numScans << " solar cal values based on geolocation data" << endl;
    cerr << "Found " << sun_mags.size() << " solar cal values" << endl;
    
    return false;
  }
  
  // Loop through the geolocation records, updating the sun magnitudes as
  // appropriate.

  for (int scan = 0, geo_index = 0; scan < _numScans; ++scan)
  {
//    cerr << "Solar mag [" << scan << "] = " << sun_mags[scan] << " m" << endl;
    
    for (int pixel = 0; pixel < _numPixels; ++pixel, ++geo_index)
    {
      _locations[geo_index].sun_mag = sun_mags[scan];
    } /* endfor - pixel */
  } /* endfor - scan */
  
  return true;
}


/*********************************************************************
 * _readSolarZenithData()
 */

bool Geolocation::_readSolarZenithData(const HdfFile &hdf_file)
{
  static const string method_name = "Geolocation::_readSolarZenithData()";
  
  // Read the solar zenith data from the TRMM file

  float64 *data;
  vector< int > dimensions;
  
  if ((data =
       _readData(hdf_file, LOCAL_DIR_SDS_FIELD_NAME, dimensions)) == 0)
    return false;
  
  // Check for errors in the data

  if (dimensions.size() != 4)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong rank for local direction dataset: "
	 << dimensions.size() << endl;
    
    delete [] data;
    
    return false;
  }
  
  if (dimensions[0] != _numScans)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong first dimension for local direction data." << endl;
    cerr << "Expected " << _numScans << " elements." << endl;
    cerr << "Found " << dimensions[0] << " elements." << endl;
    
    delete [] data;
    
    return false;
  }

  int num_local_dir_pixels = _numPixels / LOCAL_DIR_PIXELS_PER_ANGLE + 1;
  
  if (dimensions[1] != num_local_dir_pixels)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong second dimension for local direction data." << endl;
    cerr << "Expected " << num_local_dir_pixels << " elements." << endl;
    cerr << "Found " << dimensions[1] << " elements." << endl;
    
    delete [] data;
    
    return false;
  }
  
  if (dimensions[2] != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong third dimension for local direction data." << endl;
    cerr << "Expected 2 elements." << endl;
    cerr << "Found " << dimensions[2] << " elements." << endl;
    
    delete [] data;
    
    return false;
  }
  
  if (dimensions[3] != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong fourth dimension for local direction data." << endl;
    cerr << "Expected 2 elements." << endl;
    cerr << "Found " << dimensions[3] << " elements." << endl;
    
    delete [] data;
    
    return false;
  }
  
  // Set the solar zenith values

  int last_local_pixel =
    ((num_local_dir_pixels-1) * LOCAL_DIR_PIXELS_PER_ANGLE) - 1;

  for (int scan = 0; scan < _numScans; ++scan)
  {
    for (int loc_pixel = 0; loc_pixel < num_local_dir_pixels-1; ++loc_pixel)
    {
      // Want data point between [scan][loc_pixel][1][0] and
      // [scan][loc_pixel+1][1][0]

      int prev_data_index = (scan * num_local_dir_pixels * 2 * 2) +
	(loc_pixel * 2 * 2) + (1 * 2) + 0;
      int next_data_index = (scan * num_local_dir_pixels * 2 * 2) +
	((loc_pixel+1) * 2 * 2) + (1 * 2) + 0;
      
      double prev_solar_zenith = data[prev_data_index];
      double next_solar_zenith = data[next_data_index];
      double zenith_delta = (next_solar_zenith - prev_solar_zenith) /
	(double)LOCAL_DIR_PIXELS_PER_ANGLE;
      
      for (int i = 0; i < LOCAL_DIR_PIXELS_PER_ANGLE; ++i)
      {
	int pixel = (loc_pixel * LOCAL_DIR_PIXELS_PER_ANGLE) + i;

	double solar_zenith = prev_solar_zenith + (zenith_delta * (double)i);

	int loc_index = (scan * _numPixels) + pixel;
	
	_locations[loc_index].solar_zenith = solar_zenith;
	_locations[loc_index].cos_solar_zenith = cos(solar_zenith * DEG_TO_RAD);
	
      } /* endfor - i */
      
    } /* endfor - loc_pixel */
    
    // The last pixels in the geolocation data get filled in with the
    // last value from the local direction data.

    int last_loc_index = (scan * _numPixels) + last_local_pixel;
    
    for (int pixel = last_local_pixel + 1; pixel < _numPixels; ++pixel)
    {
      int loc_index = (scan * _numPixels) + pixel;
      
      _locations[loc_index].solar_zenith =
	_locations[last_loc_index].solar_zenith;
      _locations[loc_index].cos_solar_zenith =
	_locations[last_loc_index].cos_solar_zenith;
    } /* endfor - pixel */
    
  } /* endfor - scan */
  
  delete [] data;
  
  return true;
}
