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
 * @file NetcdfFile.cc
 *
 * @class NetcdfFile
 *
 * Class controlling access to a HRRR 15 minute netCDF file.
 *  
 * @date 3/30/2009
 *
 */

#include <iostream>

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>
#include <toolsa/str.h>

#include "NetcdfFile.hh"

using namespace std;


// Global constants

const string NetcdfFile::X_DIM_NAME = "west_east";
const string NetcdfFile::Y_DIM_NAME = "south_north";
const string NetcdfFile::TIME_DIM_NAME = "num_snapshots";

const string NetcdfFile::CEN_LAT_ATT_NAME = "CEN_LAT";
const string NetcdfFile::CEN_LON_ATT_NAME = "CEN_LON";
const string NetcdfFile::LAT1_ATT_NAME = "TRUELAT1";
const string NetcdfFile::LAT2_ATT_NAME = "TRUELAT2";
const string NetcdfFile::DX_ATT_NAME = "DX";
const string NetcdfFile::DY_ATT_NAME = "DY";
const string NetcdfFile::STAND_LON_ATT_NAME = "STAND_LON";

const string NetcdfFile::YEAR_ATT_NAME = "JULYR";
const string NetcdfFile::JDAY_ATT_NAME = "JULDAY";
const string NetcdfFile::BASE_TIME_ATT_NAME = "GMT";


/*********************************************************************
 * Constructors
 */

NetcdfFile::NetcdfFile(const string &file_path,
		       const int forecast_interval_secs,
		       const int start_lead_secs,
		       const Mdvx::compression_type_t output_compression_type,
		       const Mdvx::scaling_type_t output_scaling_type,
		       const double output_scale,
		       const double output_bias,
		       const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _forecastIntervalSecs(forecast_interval_secs),
  _startLeadSeconds(start_lead_secs),
  _outputCompressionType(output_compression_type),
  _outputScalingType(output_scaling_type),
  _outputScale(output_scale),
  _outputBias(output_bias),
  _filePath(file_path),
  _ncFile(file_path.c_str()),
  _nx(-1),
  _ny(-1),
  _numTimes(-1)
{
}


/*********************************************************************
 * Destructor
 */

NetcdfFile::~NetcdfFile()
{
}


/*********************************************************************
 * init()
 */

bool NetcdfFile::init()
{
  static const string method_name = "NetcdfFile::init()";
  
  // Create an error object so that the netCDF library doesn't exit when an
  // error is encountered.  This object is not explicitly used in the below
  // code, but is used implicitly by the netCDF library.

  Nc3Error nc_error(Nc3Error::silent_nonfatal);

  // Check the input file
  
  if (!_ncFile.is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid netCDF file: " << _filePath << endl;
    
    return false;
  }
  
  // Get the dimensions of the data

  if (!_getDimensions())
    return false;
  
  // Get the projection information.  This must be called after 
  // _getDimensions().

  if (!_getProjection())
    return false;
  
  // Get the base time information

  if (!_getBaseTime())
    return false;
  
  return true;
}


/*********************************************************************
 * createMdvFiles()
 */

bool NetcdfFile::createMdvFiles(const string &output_url) const
{
  static const string method_name = "NetcdfFile::createMdvFiles()";

  // Create a file for each forecast time

  for (int itime = 0; itime < _numTimes; ++itime)
  {
    int forecast_secs = _startLeadSeconds + itime * _forecastIntervalSecs;
    DateTime forecast_time = _baseTime + forecast_secs;
    
    if (_debug)
      cerr << "---> Creating " << forecast_secs
	   << " second forecast valid at " << forecast_time << endl;
    
    // Create the output file for this forecast time.  If there is an
    // error, go ahead and try to create the next forecast file.

    if (!_createMdvFile(output_url, itime, forecast_secs, forecast_time))
      continue;
    
  } /* endfor - itime */
  
  return false;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createMdvFile()
 */

bool NetcdfFile::_createMdvFile(const string &output_url,
				const int forecast_index,
				const int forecast_secs,
				const DateTime &forecast_time) const
{
  static const string method_name = "NetcdfFile::_createMdvFile()";
  
  // Create the MDV file

  DsMdvx mdvx;
  
  _setMasterHeader(mdvx, forecast_time);

  // Create each of the fields and add them to the file

  vector< NetcdfField* >::const_iterator field_iter;
  
  for (field_iter = _fieldList.begin(); field_iter != _fieldList.end();
       ++field_iter)
  {
    NetcdfField *nc_field = *field_iter;
    
    if (_debug)
      cerr << "     Creating field: " << nc_field->getFieldName() << endl;
    
    MdvxField *mdv_field =
      nc_field->createMdvField(_ncFile, _inputProj,
			       forecast_index, forecast_secs,
			       forecast_time);
    
    if (mdv_field == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating MDV field for: "
	   << nc_field->getFieldName() << endl;
      
      continue;
    }
    
    if (_outputScalingType == Mdvx::SCALING_NONE)
      mdv_field->convertType(Mdvx::ENCODING_FLOAT32,
			     _outputCompressionType,
			     _outputScalingType);
    else
      mdv_field->convertType(Mdvx::ENCODING_INT16,
			     _outputCompressionType,
			     _outputScalingType, _outputScale, _outputBias);
    
    mdvx.addField(mdv_field);
    
  } /* endfor - field_name */
  
  // Write the output file

  mdvx.setWriteAsForecast();
  
  if (mdvx.writeToDir(output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing forecast to URL: " << output_url << endl;
    cerr << "Forecast secs: " << forecast_secs << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _getBaseTime()
 */

bool NetcdfFile::_getBaseTime()
{
  static const string method_name = "NetcdfFile::_getBaseTime()";
  
  // Get the time attributes from the file

  int year = _getGlobalAttAsInt(YEAR_ATT_NAME);
  
  if (year == NC_FILL_INT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << YEAR_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  int jday = _getGlobalAttAsInt(JDAY_ATT_NAME);
  
  if (jday == NC_FILL_INT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << JDAY_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  float base_time = _getGlobalAttAsFloat(BASE_TIME_ATT_NAME);
  
  if (base_time == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << BASE_TIME_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  if (_debug)
    cerr << "   year = " << year
	 << ", jday = " << jday
	 << ", gmt = " << base_time << endl;
  
  // Calculate the hour/min/sec from the GMT fraction

  int hour = (int)base_time;
  base_time -= (float)hour;
  
  int min = (int)(60.0 * base_time);
  base_time -= (float)min / 60.0;
  
  int sec = (int)(3600.0 * base_time);
  

  // Set the base time

  _baseTime.setByDayOfYear(year, jday, hour, min, sec);
  
  if (_debug)
    cerr << "   base time = " << _baseTime << endl;
  
  return true;
}


/*********************************************************************
 * _getDimensions()
 */

bool NetcdfFile::_getDimensions()
{
  static const string method_name = "NetcdfFile::_getDimensions()";
  
  // Get the X/Y dimensions

  _nx = _getDimension(X_DIM_NAME);
  
  if (_nx <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << X_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _ny = _getDimension(Y_DIM_NAME);
  
  if (_ny <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << Y_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  // Get the number of data times from the file

  _numTimes = _getDimension(TIME_DIM_NAME);
  
  if (_numTimes <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TIME_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  if (_debug)
    cerr << "   nx = " << _nx << ", ny = " << _ny << ", numTimes = " 
	 << _numTimes << endl;
  
  return true;
}


/*********************************************************************
 * _getProjection()
 */

bool NetcdfFile::_getProjection()
{
  static const string method_name = "NetcdfFile::_getProjection()";
  
  // Get the center lat/lon from the file

  float center_lat = _getGlobalAttAsFloat(CEN_LAT_ATT_NAME);
  
  if (center_lat == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << CEN_LAT_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  float center_lon = _getGlobalAttAsFloat(CEN_LON_ATT_NAME);
  
  if (center_lon == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << CEN_LON_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  // Get the reference latitude values

  float lat1 = _getGlobalAttAsFloat(LAT1_ATT_NAME);
  
  if (lat1 == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT1_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  float lat2 = _getGlobalAttAsFloat(LAT2_ATT_NAME);
  
  if (lat2 == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT2_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }

  // standard longitude - longitude of the projection origin

  float standard_lon = _getGlobalAttAsFloat(STAND_LON_ATT_NAME);
  if (standard_lon == NC_FILL_FLOAT)
  {
    standard_lon = center_lon;
  }
  
  // Get the dx/dy values from the file.  These values are specified in
  // meters.

  float dx_m = _getGlobalAttAsFloat(DX_ATT_NAME);
  
  if (dx_m == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << DX_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  float dy_m = _getGlobalAttAsFloat(DY_ATT_NAME);
  
  if (dy_m == NC_FILL_FLOAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << DY_ATT_NAME
	 << " global attribute from file " << _filePath << endl;
    
    return false;
  }
  
  if (_debug)
    cerr << "  center_lat = " << center_lat
	 << ", center_lon = " << center_lon
	 << ", standard_lon = " << standard_lon
	 << ", dx_m = " << dx_m
	 << ", dy_m = " << dy_m << endl;
  
  // Set the projection information

  double dx_km = dx_m / 1000.0;
  double dy_km = dy_m / 1000.0;
  
  double minx = (-(dx_km * _nx) / 2.0) + (dx_km / 2.0);
  double miny = (-(dy_km * _ny) / 2.0) + (dy_km / 2.0);
  
  if (_debug)
    cerr << "   minx = " << minx << ", miny = " << miny << endl;
  
  _inputProj.initLc2(center_lat, standard_lon,
		     lat1, lat2);
  _inputProj.setGrid(_nx, _ny, dx_km, dy_km, minx, miny);
  _inputProj.setOffsetOrigin(center_lat, center_lon);

  if (_debug) {
    cerr << "====== input projection =====" << endl;
    _inputProj.print(cerr);
    _inputProj.printCoord(_inputProj.getCoord(), cerr);
  }

  return true;
}


/*********************************************************************
 * _setMasterHeader()
 */

void NetcdfFile::_setMasterHeader(DsMdvx &mdvx,
				  const DateTime &forecast_time) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = _baseTime.utime();
  master_hdr.time_begin = master_hdr.time_gen;
  master_hdr.time_end = master_hdr.time_gen;
  master_hdr.time_centroid = master_hdr.time_gen;
  master_hdr.time_expire = master_hdr.time_gen;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = master_hdr.native_vlevel_type;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.forecast_time = forecast_time.utime();
  master_hdr.forecast_delta =
    master_hdr.forecast_time - master_hdr.time_centroid;
  master_hdr.sensor_lon = _inputProj.getPjgMath().getOriginLon();
  master_hdr.sensor_lat = _inputProj.getPjgMath().getOriginLat();
  STRcopy(master_hdr.data_set_info, "Hrrr15minNcf2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Hrrr15minNcf2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _filePath.c_str(), MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}
