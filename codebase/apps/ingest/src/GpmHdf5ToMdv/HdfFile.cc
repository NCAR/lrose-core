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
 * @file HdfFile.cc
 *
 * @class HdfFile
 *
 * Class controlling access to a TRMM HDF file.
 *  
 * @date 10/31/2008
 *
 */

#include <iostream>

#include <toolsa/Path.hh>
#include <rapmath/math_macros.h>

#include "HdfFile.hh"

using namespace std;


// Global constants

const int HdfFile::MAX_FIELD_NAME_LEN = 64;
const int HdfFile::MAX_FIELD_LIST_LEN = 1024;
const double HdfFile::MISSING_VALUE = -9999.0;

const string HdfFile::LATITUDE_SDS_FIELD_NAME = "Latitude";
const string HdfFile::LONGITUDE_SDS_FIELD_NAME = "Longitude";
//const string HdfFile::GEOLOCATION_SDS_FIELD_NAME = "geolocation";
const string HdfFile::LOCAL_DIR_SDS_FIELD_NAME = "localDirection";
const int HdfFile::LOCAL_DIR_PIXELS_PER_ANGLE = 10;
const string HdfFile::SCAN_TIME_YEAR = "Year";
const string HdfFile::SCAN_TIME_MONTH = "Month";
const string HdfFile::SCAN_TIME_DAY = "DayOfMonth";
const string HdfFile::SCAN_TIME_HOUR = "Hour";
const string HdfFile::SCAN_TIME_MINUTE = "Minute";
const string HdfFile::SCAN_TIME_SECOND = "Second";

const string HdfFile::SOLAR_CAL_VDATA_NAME = "solarCal";
const string HdfFile::SOLAR_CAL_SUN_MAG_FIELD_NAME = "sunMag";


/*********************************************************************
 * Constructors
 */

HdfFile::HdfFile(const string &file_path,
		 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _filePath(file_path),
  _fileId(-1),
  _sdId(-1),
  _readSunMagDataFlag(false),
  _numScans(-1),
  _numPixels(-1),
  _scanTimesRead(false)
{
}


/*********************************************************************
 * Destructor
 */

HdfFile::~HdfFile()
{

#ifdef JUNK
  // Close the SDS interface if it was successfully opened

  if (_sdId != -1)
    SDend(_sdId);

  // Close the FDATA interface

  Vend(_fileId);
  
  // Close the file if it was successfully opened

  if (_fileId != -1)
    Hclose(_fileId);
#endif

}


/*********************************************************************
 * init()
 */

bool HdfFile::init()
{
  static const string method_name = "HdfFile::init()";
  
  if (_verbose)
    cerr << "Checking file to make sure it is an HDF file" << endl;

#ifdef JUNk
  if (Hishdf(_filePath.c_str()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _filePath << " is not a valid HDF file, or file not found" << endl;
    
    return false;
  } 

  // Open connections to the TRMM HDF file

  if (_verbose)
    cerr << "Opening connection to file" << endl;
  
  if ((_fileId = Hopen (_filePath.c_str(), DFACC_READ, 0)) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file : " << _filePath << endl;

    return false;
  }

  // Start HDF SDS and Vdata Interfaces

  if (_verbose)
    cerr << "Starting SDS interface" << endl;
  
  if ((_sdId = SDstart(_filePath.c_str(), DFACC_READ)) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDstart failed." << endl;
    HEprint (stderr, 0);

    return false;
  }

  if (_verbose)
    cerr << "Starting VDATA interface" << endl;
  
  if (Vstart(_fileId) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening VDATA interface for HDF file: " << _filePath << endl;
    
    return false;
  }
  
  // Obtain number of SDSs and Global Attributes in input file

  if (_verbose)
    cerr << "Getting HDF file information" << endl;
  
  int32 n_datasets;
  int32 n_file_attrs;
  
  if (SDfileinfo(_sdId, &n_datasets, &n_file_attrs) != 0)
  {
    cerr << "SDfileinfo failed." << endl;
    return false;
  }

  // List out SDSs contained in input HDF file

  if (_verbose)
  {
    cerr << endl << " Number of SDS arrays in file: " << n_datasets
	 << endl << endl;

    for (int index = 0; index < n_datasets; index++)
    {
      int32 num_element = 1;
      int32 sds_id = SDselect(_sdId, index); 

      char sds_name[MAX_FIELD_NAME_LEN];
      int32 rank;
      int32 dim_sizes[MAX_VAR_DIMS];
      int32 num_type;
      int32 attributes;
      
      if (SDgetinfo(sds_id, sds_name, &rank, dim_sizes,
		    &num_type, &attributes) != 0)
      {
	cerr << "Error getting SD info for dataset" << endl;
	return false;
      }

      float64 gain;
      float64 gain_err;
      float64 offset;
      float64 offset_err;
      int32 cal_data_type;
      bool cal_data_available = false;
      
      if (SDgetcal(sds_id, &gain, &gain_err,
		   &offset, &offset_err, &cal_data_type) == 0)
      {
	cal_data_available = true;
      }
 
      cerr << endl << " ****************************************" << endl;
      cerr << "  SDS name  = " << sds_name << endl;
      cerr << "  SDS type  = " << num_type << endl;
      cerr << "  SDS rank  = " << rank << endl;
      if (cal_data_available)
	cerr << "  SDS scale = " << gain << endl;
      else
	cerr << "  SDS scale = UNAVAILABLE" << endl;
      cerr << "  SDS dims  = ";
      
      for (int j = 0; j < rank; j++)
      {
	cerr << "  " << dim_sizes[j];
	num_element *= dim_sizes[j];
      }
      cerr << endl;
 
      SDendaccess(sds_id);
    } /* endfor - index */
  }

#endif
  
  return true;
}

/*********************************************************************
 * readDataHeaders()
 */

bool HdfFile::readDataHeaders()
{
  static const string method_name = "HdfFile::readDataHeaders()";
  
  // Read the geolocation data

  if (!_readGeolocationData())
    return false;
  
  // Read the scan time data.  Note that this must be done after reading
  // the geolocation data becuase this method assumes that the records
  // have been initialized.

  if (!_readScanTimeData())
    return false;
  
  if (_readSunMagDataFlag)
  {
    // Read the sun magnitude data from the file

    if (!_readSunMagData())
      return false;

    // Read the solar zenith data from the file

    if (!_readSolarZenithData())
      return false;
  }
  
  return true;
}

/*********************************************************************
 * getScanTimeRange()
 */

bool HdfFile::getScanTimeRange(DateTime &begin_time, DateTime &end_time)
{

  static const string method_name = "HdfFile::getScanTimes()";
  
  // Read the scan times data

  if (!_readScanTimeData())
    return false;
  
  begin_time = _scanTimes[0];
  end_time = _scanTimes[_scanTimes.size() - 1];
  
  if (_debug || _verbose)
  {
    cerr << "Begin time = " << begin_time << endl;
    cerr << "End time = " << end_time << endl;
  }
  
  return true;
}



/*********************************************************************
 * readSdsData()
 */

fl64 *HdfFile::readSdsData(const string &field_name,
			   vector< int > &dimensions) const
{
  static const string method_name = "HdfFile::readSdsData()";
  
  // Get the index of the field within the file

#ifdef JUNK
  int sds_index;

  if ((sds_index = SDnametoindex(_sdId, field_name.c_str())) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving data from TRMM file for field: "
	 << field_name << endl;

    return false;
  }

  // Get the field identifier

  int sds_id = SDselect(_sdId, sds_index);
  
  // Get all of the information about the field
  int iscoord = SDiscoordvar(sds_id);
  char sds_name[MAX_FIELD_NAME_LEN];
  int32 num_type;
  int32 attributes;
  int32 rank;
  int32 dim_sizes[MAX_VAR_DIMS];
  
  if (SDgetinfo(sds_id, sds_name, &rank, dim_sizes, &num_type,
		&attributes) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting field information from TRMM file" << endl;
    cerr << "Field name: " << field_name << endl;
    
    SDendaccess(sds_id);
    
    return false;
  }
  
  // Calculate the size of the dataset.  While we're here, set up the
  // read arrays to tell HDF we want to read all of the data.  Also set
  // the dimensions vector for returning to the caller.

  int32 num_elements = 1;
  
  int32 *edges = new int32[rank];
  int32 *stride = new int32[rank];
  int32 *start = new int32[rank];
  
  for (int j = 0; j < rank; ++j)
  {
    dimensions.push_back(dim_sizes[j]);

    num_elements *= dim_sizes[j];
    
    edges[j] = dim_sizes[j];
    stride[j] = 1;
    start[j] = 0;
  }
#endif
  
  // Read the data based on the data type

  fl64 *data = NULL;

#ifdef JUNK  
  switch (num_type)
  {
  case DFNT_FLOAT32 :
    data = _readFloat32Data(field_name, sds_id,
			    num_elements, edges, stride, start);
    break;
    
  case DFNT_FLOAT64 :
    data = _readFloat64Data(field_name, sds_id,
			    num_elements, edges, stride, start);
    break;
    
  case DFNT_INT8 :
    data = _readInt8Data(field_name, sds_id,
			 num_elements, edges, stride, start);
    break;
    
  case DFNT_UINT8 :
    data = _readUint8Data(field_name, sds_id,
			  num_elements, edges, stride, start);
    break;
    
  case DFNT_INT16 :
    data = _readInt16Data(field_name, sds_id,
			  num_elements, edges, stride, start);
    break;
    
  case DFNT_UINT16 :
    data = _readUint16Data(field_name, sds_id,
			   num_elements, edges, stride, start);
    break;
    
  case DFNT_INT32 :
    data = _readInt32Data(field_name, sds_id,
			  num_elements, edges, stride, start);
    break;
    
  case DFNT_UINT32 :
    data = _readUint32Data(field_name, sds_id,
			   num_elements, edges, stride, start);
    break;
    
  case DFNT_CHAR8 :
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process character data field: " << field_name << endl;
    return false;
    
  default :
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid data type for field: " << field_name << endl;
    return false;
    
  } /* endswitch - num_type */
  
  SDendaccess(sds_id);

#endif
  
  return data;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getDateFromFilename()
 */

DateTime HdfFile::_getDateFromFilename() const
{
  static const string method_name = "HdfFile::_getDateFromFilename()";

  // Extract the file name from the path

  Path file_path(_filePath);
  string filename = file_path.getFile();
  
  // Extract the date string from the filename.  The filename should be of
  // the format <prod id>.<date string>.*

  char *filename_tokens = new char[filename.length() + 1];
  memcpy(filename_tokens, filename.c_str(), filename.length() + 1);
  
  char *token;
  token = strtok(filename_tokens, ".");    // prod id
  if ((token = strtok(0, ".")) == 0)         // date string
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting date string from filename: " << filename << endl;
    
    return DateTime::NEVER;
  }
  
  // Extract the date information from the date string.  The date string is
  // of the format <year><month><day> where <year> is the 2-digit year,
  // <month> is the month 1-12 and <day> is the day 1-31.

  string date_string = token;
  
  int year = atoi(date_string.substr(0, 2).c_str());
  if (year < 75)
    year += 2000;
  else
    year += 1900;
  int month = atoi(date_string.substr(2, 2).c_str());
  int day = atoi(date_string.substr(4, 2).c_str());
  
  return DateTime(year, month, day);
}

/*********************************************************************
 * _readGeolocationData()
 */

bool HdfFile::_readGeolocationData()
{
  static const string method_name = "HdfFile::_readGeolocationData()";
  
  // Read the geolocation data from the TRMM file

  fl64 *lat_data;
  fl64 *lon_data;
  vector< int > lat_dimensions;
  vector< int > lon_dimensions;
  
  if ((lat_data = readSdsData(LATITUDE_SDS_FIELD_NAME, lat_dimensions)) == 0)
    return false;

  if ((lon_data = readSdsData(LONGITUDE_SDS_FIELD_NAME, lon_dimensions)) == 0)
    return false;
    

  // Check for errors in the data

  if (lat_dimensions.size() != 2 || lon_dimensions.size() != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong rank for geolocation dataset: "
	 << lat_dimensions.size() << "," << lon_dimensions.size() << endl;
    
    delete [] lat_data;
    delete [] lon_data;
    
    return false;
  }
  
  // Set the dataset dimensions

  _numScans = lat_dimensions[0];
  _numPixels = lon_dimensions[1];
  
  // Set the locations

  int num_locations = _numScans * _numPixels;
  
  int index = 0;
  
  for (int loc = 0; loc < num_locations; ++loc)
  {
    location_t location;
    
    location.lat = lat_data[index];
    location.lon = lon_data[index];
    index++;

    location.scan_time = DateTime::NEVER;
    location.sun_mag = MISSING_VALUE;
    location.solar_zenith = MISSING_VALUE;
    location.cos_solar_zenith = MISSING_VALUE;
    
    _locations.push_back(location);
  } /* endfor - i */
  
  delete [] lat_data;
  delete [] lon_data;
  
  return true;
}


/*********************************************************************
 * _readScanTimes()
 */

bool HdfFile::_readScanTimeData()
{
  static const string method_name = "HdfFile::_readScanTimeData()";
  
  if (_scanTimesRead)
    return true;
  
  // Clear out the scan times vector

  _scanTimes.clear();
  

  // get components of scan time
  vector< int > year_dimensions, month_dimensions, day_dimensions;
  vector< int > hour_dimensions, minute_dimensions, second_dimensions;
  fl64 *year_data = readSdsData(SCAN_TIME_YEAR, year_dimensions);
  fl64 *month_data = readSdsData(SCAN_TIME_MONTH, month_dimensions);
  fl64 *day_data = readSdsData(SCAN_TIME_DAY, day_dimensions);
  fl64 *hour_data = readSdsData(SCAN_TIME_HOUR, hour_dimensions);
  fl64 *minute_data = readSdsData(SCAN_TIME_MINUTE, minute_dimensions);
  fl64 *second_data = readSdsData(SCAN_TIME_SECOND, second_dimensions);

  // Check for errors in the data sizes
  if (year_dimensions.size() != 1 || month_dimensions.size() != 1 || day_dimensions.size() != 1 
      || hour_dimensions.size() != 1 || minute_dimensions.size() != 1 || second_dimensions.size() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Scan time component dimension sizes do not all = 1: "
	 << year_dimensions.size() << ", " << month_dimensions.size() << ", " << day_dimensions.size() 
	 << ", " << hour_dimensions.size() << ", "<< minute_dimensions.size() << ", " << second_dimensions.size() << endl;
    
    delete [] year_data;
    delete [] month_data;
    delete [] day_data;
    delete [] hour_data;
    delete [] minute_data;
    delete [] second_data;
    
    return false;
  }

  if (year_dimensions[0] != _numScans || month_dimensions[0] != _numScans || day_dimensions[0] != _numScans 
      || hour_dimensions[0] != _numScans || minute_dimensions[0] != _numScans || second_dimensions[0] != _numScans)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Scan time component sizes do not all = numScans (" << _numScans << ") : "
	 << year_dimensions[0] << ", " << month_dimensions[0] << ", " << day_dimensions[0] 
	 << ", " << hour_dimensions[0] << ", "<< minute_dimensions[0] << ", " << second_dimensions[0] << endl;
    
    delete [] year_data;
    delete [] month_data;
    delete [] day_data;
    delete [] hour_data;
    delete [] minute_data;
    delete [] second_data;
    
    return false;
  }

  // Pull out the time information and put it into our vector

  for (int i = 0; i < _numScans; i++)
  {
    _scanTimes.push_back(DateTime(year_data[i], month_data[i], day_data[i],
				  hour_data[i], minute_data[i], second_data[i]));
  } /* endfor - i */

  if (_verbose)
  {
    cerr << "Scan times;" << endl;
    
    vector< DateTime >::const_iterator scan_time;
    
    for (scan_time = _scanTimes.begin(); scan_time != _scanTimes.end();
	 ++scan_time)
      cerr << "   " << *scan_time << endl;
  }
  
  // Loop through the geolocation records, updating the scan times as
  // appropriate.

  for (int scan = 0, geo_index = 0; scan < _numScans; ++scan)
  {
    for (int pixel = 0; pixel < _numPixels; ++pixel, ++geo_index)
    {
      _locations[geo_index].scan_time = _scanTimes[scan].utime();
    } /* endfor - pixel */
  } /* endfor - scan */
  

  _scanTimesRead = true;

  delete [] year_data;
  delete [] month_data;
  delete [] day_data;
  delete [] hour_data;
  delete [] minute_data;
  delete [] second_data;

  return true;
}


/*********************************************************************
 * _readSunMagData()
 */

bool HdfFile::_readSunMagData()
{
  static const string method_name = "HdfFile::getSunMags()";
  
#ifdef JUNK
  
  // Get the reference number for this VDATA

  int32_t vdata_ref;
  
  if ((vdata_ref = VSfind(_fileId, SOLAR_CAL_VDATA_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find " << SOLAR_CAL_VDATA_NAME << " VDATA in HDF file: "
	 << _filePath << endl;
    
    return false;
  }
  
  // Attach the VDATA

  int32_t vdata_id;
  
  if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error attaching to " << SOLAR_CAL_VDATA_NAME << " vdata." << endl;
    
    return false;
  }
  
  // Get the information about the scan time VDATA

  int32_t n_records;
  int32_t interlace;
  
  if (VSQueryinterlace(vdata_id, &interlace) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting interlace method for solar cal VDATA" << endl;
    
    return false;
  }
  
  if (VSQuerycount(vdata_id, &n_records) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting number of solar cal VDATA records" << endl;
    
    return false;
  }
  
  // Read the sun mag values

  if (VSsetfields(vdata_id, SOLAR_CAL_SUN_MAG_FIELD_NAME.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting <" << SOLAR_CAL_SUN_MAG_FIELD_NAME
	 << "> field list for " << SOLAR_CAL_VDATA_NAME << " VDATA" << endl;
    
    return false;
  }
  
  fl64 *sun_mag_values = new fl64[n_records];
  
  if (VSread(vdata_id, (uint8 *)sun_mag_values,
	     n_records, interlace) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading sun mag values from HDF file" << endl;
    
    delete [] sun_mag_values;
    
    return false;
  }
   
  // Loop through the geolocation records, updating the sun magnitudes as
  // appropriate.

  for (int scan = 0, geo_index = 0; scan < _numScans; ++scan)
  {    
    for (int pixel = 0; pixel < _numPixels; ++pixel, ++geo_index)
    {
      _locations[geo_index].sun_mag = (double)sun_mag_values[scan];
    } /* endfor - pixel */
  } /* endfor - scan */

  delete [] sun_mag_values;

  // Detach the scan time VDATA

  if (VSdetach(vdata_id) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error detaching from " << SOLAR_CAL_VDATA_NAME << " vdata." << endl;
    
    return false;
  }

#endif
  
  return true;
}


/*********************************************************************
 * _readSolarZenithData()
 */

bool HdfFile::_readSolarZenithData()
{
  static const string method_name = "HdfFile::_readSolarZenithData()";
  
  // Read the solar zenith data from the TRMM file

  fl64 *data;
  vector< int > dimensions;
  
  if ((data = readSdsData(LOCAL_DIR_SDS_FIELD_NAME, dimensions)) == 0)
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


/*********************************************************************
 * _readFloat32Data()
 */

fl64 *HdfFile::_readFloat32Data(const string &field_name,
				int32_t sds_id,
				int32_t num_elements,
				int32_t *edges, int32_t *stride,
				int32_t *start) const
{
  static const string method_name = "HdfFile::_readFloat32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  fl32 *float32_data = new fl32[num_elements];

#ifdef JUNK
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, stride, edges, (VOIDP)float32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] float32_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)float32_data[i];

  
  delete [] float32_data;
  
  return return_data;
}


/*********************************************************************
 * _readFl64Data()
 */

fl64 *HdfFile::_readFl64Data(const string &field_name,
				int32_t sds_id,
				int32_t num_elements,
				int32_t *edges, int32_t *stride,
				int32_t *start) const
{
  static const string method_name = "HdfFile::_readFloat64Data()";

  // Allocate space for the data

  fl64 *fl64_data = new fl64[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, fl64_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] fl64_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)fl64_data[i];
  
  delete [] fl64_data;
  
  return return_data;
}


/*********************************************************************
 * _readInt8Data()
 */

fl64 *HdfFile::_readInt8Data(const string &field_name,
			     int32_t sds_id,
			     int32_t num_elements,
			     int32_t *edges, int32_t *stride,
			     int32_t *start) const
{
  static const string method_name = "HdfFile::_readInt8Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  si08 *int8_data = new si08[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, int8_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int8_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)int8_data[i];
  
  delete [] int8_data;
  
  return return_data;
}


/*********************************************************************
 * _readInt16Data()
 */

fl64 *HdfFile::_readInt16Data(const string &field_name,
			      int32_t sds_id,
			      int32_t num_elements,
			      int32_t *edges, int32_t *stride,
			      int32_t *start) const
{
  static const string method_name = "HdfFile::_readInt16Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  si16 *int16_data = new si16[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, int16_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int16_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)int16_data[i];
  
  delete [] int16_data;
  
  return return_data;
}


/*********************************************************************
 * _readInt32Data()
 */

fl64 *HdfFile::_readInt32Data(const string &field_name,
			      int32_t sds_id,
			      int32_t num_elements,
			      int32_t *edges, int32_t *stride,
			      int32_t *start) const
{
  static const string method_name = "HdfFile::_readInt32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  int32_t *int32_data = new int32_t[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, int32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int32_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)int32_data[i];
  
  delete [] int32_data;
  
  return return_data;
}

/*********************************************************************
 * _readUint8Data()
 */

fl64 *HdfFile::_readUint8Data(const string &field_name,
			      int32_t sds_id,
			      int32_t num_elements,
			      int32_t *edges, int32_t *stride,
			      int32_t *start) const
{
  static const string method_name = "HdfFile::_readUint8Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  ui08 *uint8_data = new ui08[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, uint8_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint8_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)uint8_data[i];
  
  delete [] uint8_data;
  
  return return_data;
}


/*********************************************************************
 * _readUint16Data()
 */

fl64 *HdfFile::_readUint16Data(const string &field_name,
			       int32_t sds_id,
			       int32_t num_elements,
			       int32_t *edges, int32_t *stride,
			       int32_t *start) const
{
  static const string method_name = "HdfFile::_readUint16Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  ui16 *uint16_data = new ui16[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, uint16_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint16_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)uint16_data[i];
  
  delete [] uint16_data;
  
  return return_data;
}


/*********************************************************************
 * _readUint32Data()
 */

fl64 *HdfFile::_readUint32Data(const string &field_name,
			       int32_t sds_id,
			       int32_t num_elements,
			       int32_t *edges, int32_t *stride,
			       int32_t *start) const
{
  static const string method_name = "HdfFile::_readUint32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  uint32_t *uint32_data = new uint32_t[num_elements];
  
  // Read the data from the file

#ifdef JUNK
  if (SDreaddata(sds_id, start, 0, edges, uint32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint32_data;
    
    return 0;
  }
#endif

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)uint32_data[i];
  
  delete [] uint32_data;
  
  return return_data;
}
