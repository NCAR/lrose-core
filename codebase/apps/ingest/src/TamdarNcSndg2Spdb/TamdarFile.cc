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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:06 $
//   $Id: TamdarFile.cc,v 1.9 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TamdarFile: Class for controlling access to the netCDF tamdar files.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cerrno>
#include <cfloat>
#include <cstdio>
#include <math.h>
#include <string.h>

#include <toolsa/os_config.h>
#include <physics/thermo.h>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>

#include "TamdarFile.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

TamdarFile::TamdarFile() :
  _debug(false),
  _filePath(""),
  _numRecsDimName(""),
  _tailLenDimName(""),
  _missingDataValueAttName(""),
  _latitudeVarName(""),
  _longitudeVarName(""),
  _altitudeVarName(""),
  _temperatureVarName(""),
  _windDirVarName(""),
  _windSpeedVarName(""),
  _relHumVarName(""),
  _dewPointVarName(""),
  _tailNumberVarName(""),
  _dataSourceVarName(""),
  _soundingFlagVarName(""),
  _launchTimesVarName(""),
  _objectInitialized(false),
  _fileInitialized(false)
{
}

  
/*********************************************************************
 * Destructor
 */

TamdarFile::~TamdarFile()
{
  _tamdarFile->close();
  
  delete _tamdarFile;
}


/*********************************************************************
 * initialize() - Initialize the TamdarFile object.  This method MUST
 *                be called before any other methods are called.
 *
 * Returns true on success, false on failure
 */

bool TamdarFile::initialize(const string &num_recs_dim_name,
			    const string &tail_len_dim_name,
			    const string &missing_data_value_att_name,
			    const string &latitude_var_name,
			    const string &longitude_var_name,
			    const string &altitude_var_name,
			    const string &temperature_var_name,
			    const string &wind_dir_var_name,
			    const string &wind_speed_var_name,
			    const string &rel_hum_var_name,
			    const string &dew_point_var_name,
			    const string &tail_number_var_name,
			    const string &data_source_var_name,
			    const string &sounding_flag_var_name,
			    const string &launch_times_var_name,
			    const bool debug_flag)
{
  static const string method_name = "TamdarFile::initialize()";
  
  PMU_auto_register("Initializing TAMDAR file");
  
  _objectInitialized = false;
  
  _debug = debug_flag;
  
  _numRecsDimName = num_recs_dim_name;
  _tailLenDimName = tail_len_dim_name;
  
  _missingDataValueAttName = missing_data_value_att_name;
  
  _latitudeVarName = latitude_var_name;
  _longitudeVarName = longitude_var_name;
  _altitudeVarName = altitude_var_name;
  _temperatureVarName = temperature_var_name;
  _windDirVarName = wind_dir_var_name;
  _windSpeedVarName = wind_speed_var_name;
  _relHumVarName = rel_hum_var_name;
  _dewPointVarName = dew_point_var_name;
  _tailNumberVarName = tail_number_var_name;
  _dataSourceVarName = data_source_var_name;
  _soundingFlagVarName = sounding_flag_var_name;
  _launchTimesVarName = launch_times_var_name;
  
  _objectInitialized = true;
  
  return true;
}


/*********************************************************************
 * initializeFile() - Initialize the TAMDAR file information.  This 
 *                    method MUST be called at the beginning of processing
 *                    any file.
 *
 * Returns true on success, false on failure
 */

bool TamdarFile::initializeFile(const string &tamdar_file_path)
{
  static const string method_name = "TamdarFile::initialize()";
  
  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "PROGRAMMING ERROR" << endl;
    cerr << "Must call initialize() before calling any other TamdarFile method" << endl;
    
    return false;
  }
  
  delete _tamdarFile;
  _fileInitialized = false;
  
  _filePath = tamdar_file_path;
  
  _tamdarFile = new NcFile(_filePath.c_str());
  
  if (!_tamdarFile->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Tamdar file isn't valid: " << _filePath << endl;

    _tamdarFile->close();
    return false;
  }

  _fileInitialized = true;
  
  return true;
}


/*********************************************************************
 * writeAsSpdb() - Write out the tamdar file in SPDB format.
 *
 * Returns true on success, false on failure
 */

bool TamdarFile::writeAsSpdb(const string &spdb_url,
			     const bool sort_points_on_output)
{
  static const string method_name = "TamdarFile::writeAsSpdb()";
  
  PMU_auto_register("Writing TAMDAR data as SPDB");
  
  // Make sure the object was initialized

  if (!_objectInitialized || !_fileInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Object not initialized" << endl;
    
    return false;
  }

  // Retrieve the TAMDAR soundings from the file

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAddUnique);
	
  if (!_retrieveSoundings(spdb, sort_points_on_output))
    return false;
  
  spdb.put(spdb_url,
	   SPDB_SNDG_PLUS_ID,
	   SPDB_SNDG_PLUS_LABEL);
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getByteFieldVar() - Get the specified byte values from the
 *                      netCDF file.
 *
 * Returns a pointer to the byte values on success, 0 on failure.
 */

NcValues *TamdarFile::_getByteFieldVar(const string &field_name) const
{
  static const string method_name = "TamdarFile::_getByteFieldVar()";

  NcVar *field = 0;

  if ((field = _tamdarFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != ncByte)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a float variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  NcValues *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from tamdar file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getCharFieldVar() - Get the specified character values from the
 *                      netCDF file.
 *
 * Returns a pointer to the character values on success, 0 on failure.
 */

NcValues *TamdarFile::_getCharFieldVar(const string &field_name) const
{
  static const string method_name = "TamdarFile::_getCharFieldVar()";

  NcVar *field = 0;

  if ((field = _tamdarFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != ncChar)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a character variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  NcValues *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from tamdar file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getDoubleFieldVar() - Get the specified double values from the
 *                        netCDF file.
 *
 * Returns a pointer to the double values on success, 0 on failure.
 */

NcValues *TamdarFile::_getDoubleFieldVar(const string &field_name) const
{
  static const string method_name = "TamdarFile::_getDoubleFieldVar()";

  NcVar *field = 0;

  if ((field = _tamdarFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != ncDouble)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a double variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  NcValues *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from tamdar file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getFloatFieldVar() - Get the specified float values from the
 *                       netCDF file.
 *
 * Returns a pointer to the float values on success, 0 on failure.
 */

NcValues *TamdarFile::_getFloatFieldVar(const string &field_name,
					float &missing_data_value) const
{
  static const string method_name = "TamdarFile::_getFloatFieldVar()";

  // Get the variable object from the netCDF file

  NcVar *field = 0;

  if ((field = _tamdarFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != ncFloat)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a float variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  // Get the actual variable values from the file

  NcValues *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from tamdar file: " << _filePath << endl;
    
    return 0;
  }
  
  // Get the missing data value for this variable

  missing_data_value = _getVarFloatAtt(*field, _missingDataValueAttName);
  
  return field_values;
}


/*********************************************************************
 * _getIntFieldVar() - Get the specified integer values from the
 *                     netCDF file.
 *
 * Returns a pointer to the integer values on success, 0 on failure.
 */

NcValues *TamdarFile::_getIntFieldVar(const string &field_name) const
{
  static const string method_name = "TamdarFile::_getIntFieldVar()";

  NcVar *field = 0;

  if ((field = _tamdarFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != ncInt)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not an integer variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  NcValues *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from tamdar file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getVarFloatAtt() - Get the specified attribute from the given
 *                     netCDF variable as a float.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global FLOAT_MISSING_DATA_VALUE on failure.
 */

float TamdarFile::_getVarFloatAtt(const NcVar &variable,
				  const string &att_name) const
{
  static const string method_name = "TamdarFile::_getVarFloatAtt()";
  
  NcAtt *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << endl;
    cerr << "Tamdar file: " << _filePath << endl;
    
//    return FLOAT_MISSING_DATA_VALUE;
    return 0.0;
  }
  
  NcValues *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << endl;
    cerr << "Tamdar file: " << _filePath << endl;
    
//    return FLOAT_MISSING_DATA_VALUE;
    return 0.0;
  }
  
  float att_value = att_values->as_float(0);
  
  delete attribute;
  delete att_values;
  
  return att_value;
}


/*********************************************************************
 * _retrieveSoundings() - Retrieve the soundings from the netCDF file.
 *
 * Returns true on success, false on failure.
 */

bool TamdarFile::_retrieveSoundings(Spdb &spdb,
				    const bool sort_points_on_output)
{
  static const string method_name = "TamdarFile::_retrieveSoundings()";
  
  // First, get the number of records in the file.  We have to go through
  // each record to see if it's a TAMDAR sounding.

  NcDim *num_recs_dim;
  if ((num_recs_dim = _tamdarFile->get_dim(_numRecsDimName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << _numRecsDimName
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  int num_recs = num_recs_dim->size();

  if (_debug)
    cerr << "---> File has " << num_recs << " records" << endl;
  
  // Now get the number of characters stored for the tail number

  NcDim *tail_len_dim;
  if ((tail_len_dim = _tamdarFile->get_dim(_tailLenDimName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << _tailLenDimName
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  int tail_len = tail_len_dim->size();

  if (_debug)
    cerr << "     Tail len = " << tail_len << " chars" << endl;
  
  // Retrieve the needed variables

  float latitude_missing_data_value;
  float longitude_missing_data_value;
  float altitude_missing_data_value;
  float temperature_missing_data_value;
  float wind_dir_missing_data_value;
  float wind_speed_missing_data_value;
  float rh_missing_data_value;
  float dew_point_missing_data_value;
  
  NcValues *latitudes = _getFloatFieldVar(_latitudeVarName,
					  latitude_missing_data_value);
  NcValues *longitudes = _getFloatFieldVar(_longitudeVarName,
					   longitude_missing_data_value);
  NcValues *altitudes = _getFloatFieldVar(_altitudeVarName,
					  altitude_missing_data_value);
  NcValues *temperatures = _getFloatFieldVar(_temperatureVarName,
					     temperature_missing_data_value);
  NcValues *wind_dirs = _getFloatFieldVar(_windDirVarName,
					  wind_dir_missing_data_value);
  NcValues *wind_speeds = _getFloatFieldVar(_windSpeedVarName,
					    wind_speed_missing_data_value);
  NcValues *rhs = _getFloatFieldVar(_relHumVarName,
				    rh_missing_data_value);
  NcValues *dew_points = _getFloatFieldVar(_dewPointVarName,
					   dew_point_missing_data_value);
  
  NcValues *tail_numbers = _getCharFieldVar(_tailNumberVarName);
  NcValues *data_sources = _getByteFieldVar(_dataSourceVarName);
  NcValues *sounding_flags = _getIntFieldVar(_soundingFlagVarName);
  NcValues *launch_times = _getDoubleFieldVar(_launchTimesVarName);
  
  if (latitudes == 0 ||
      longitudes == 0 ||
      altitudes == 0 ||
      temperatures == 0 ||
      wind_dirs == 0 ||
      wind_speeds == 0 ||
      rhs == 0 ||
      dew_points == 0 ||
      data_sources == 0 ||
      sounding_flags == 0 ||
      launch_times == 0)
  {
    delete latitudes;
    delete longitudes;
    delete altitudes;
    delete temperatures;
    delete wind_dirs;
    delete wind_speeds;
    delete rhs;
    delete dew_points;
    delete tail_numbers;
    delete data_sources;
    delete sounding_flags;
    delete launch_times;
  
    return false;
  }
  
  // Loop through the data in the file, processing the TAMDAR soundings
  // as we encounter them.

  string prev_tail_number = "";
  int prev_sounding_flag = 0;
  
  double lat_total = 0.0;
  double lon_total = 0.0;
  double num_pts = 0;
  
  Sndg sounding;
  
  for (int i = 0; i < num_recs; ++i)
  {
    // TAMDAR soundings have data source set to 4 and sounding flag
    // set to either 1 or -1
    
    int sounding_flag = sounding_flags->as_int(i);
    
    if (data_sources->as_ncbyte(i) != 4 ||
	sounding_flag == 0)
      continue;
    
    char *tail_number_chars = tail_numbers->as_string(i * tail_len);
    string tail_number = tail_number_chars;
    delete [] tail_number_chars;
    
    if (tail_number != prev_tail_number ||
	sounding_flag != prev_sounding_flag)
    {
      // Output the previous sounding information

      if (num_pts > 0)
      {
	if (_debug)
	{
	  cerr << "*** Writing out sounding" << endl;
	}

	// Update the lat/lon values in the header

	Sndg::header_t sounding_header = sounding.getHeader();
	
	sounding_header.nPoints = (si32)num_pts;
	sounding_header.lat = lat_total / num_pts;
	sounding_header.lon = lon_total / num_pts;
	
	sounding.setHeader(sounding_header);
	
	// Sort the sounding points, if requested

	if (sort_points_on_output)
	  _sortPoints(sounding);
	
	// Write out the sounding

	sounding.assemble();
	
	spdb.addPutChunk(0,
			 sounding_header.launchTime,
			 sounding_header.launchTime,  // add some offset
			 sounding.getBufLen(),
			 sounding.getBufPtr());
      }
      
      // Set up the next sounding

      Sndg::header_t sounding_header;
      memset(&sounding_header, 0, sizeof(sounding_header));
	
      sounding_header.launchTime = (si32)launch_times->as_double(i);
      sounding_header.nPoints = 0;
      sounding_header.sourceId = 0;
      sounding_header.leadSecs = 0;
      sounding_header.lat = 0.0;    // Will be filled in later
      sounding_header.lon = 0.0;    // Will be filled in later
      sounding_header.alt = 0.0;
      sounding_header.version = 1;
      STRcopy(sounding_header.sourceName, "TAMDAR", Sndg::SRC_NAME_LEN);
      STRcopy(sounding_header.sourceFmt, "netCDF", Sndg::SRC_FMT_LEN);
      STRcopy(sounding_header.siteName, tail_number.c_str(),
	      Sndg::SITE_NAME_LEN);
	
      sounding.setHeader(sounding_header);
	
      // Clear out the sounding vectors

      sounding.clearPoints();
      
      // Save the new sounding information

      prev_tail_number = tail_number;
      prev_sounding_flag = sounding_flag;
	
      lat_total = 0.0;
      lon_total = 0.0;
      num_pts = 0;
    }
    
    if (_debug)
    {
      cerr << "*** Found TAMDAR sounding!" << endl;
      cerr << "    tail number = " << tail_number << endl;
      cerr << "    sounding_flag = " << sounding_flag << endl;
      cerr << "    latitude = " << latitudes->as_float(i) << endl;
      cerr << "    longitude = " << longitudes->as_float(i) << endl;
      cerr << "    altitude = " << altitudes->as_float(i) << endl;
      cerr << "    temperature = " << temperatures->as_float(i) << endl;
      cerr << "    wind_dir = " << wind_dirs->as_float(i) << endl;
      cerr << "    wind_speed = " << wind_speeds->as_float(i) << endl;
      cerr << "    rh = " << rhs->as_float(i) << endl;
      cerr << "    dew_point = " << dew_points->as_float(i) << endl;
    }
    
    // Save the current sounding values

    Sndg::point_t sounding_data;
    
    float latitude = latitudes->as_float(i);
    float longitude = longitudes->as_float(i);
    float altitude = altitudes->as_float(i);
    float wind_dir = wind_dirs->as_float(i);
    float wind_speed = wind_speeds->as_float(i);
    float rh = rhs->as_float(i);
    float temperature = temperatures->as_float(i);
    float dew_point = dew_points->as_float(i);
    
    // Must have valid lat/lon values in order to use this point

    if (latitude == latitude_missing_data_value ||
	longitude == longitude_missing_data_value)
      continue;
    
    sounding_data.time = Sndg::VALUE_UNKNOWN;

    if (altitude == altitude_missing_data_value)
    {
      sounding_data.pressure = Sndg::VALUE_UNKNOWN;
      sounding_data.altitude = Sndg::VALUE_UNKNOWN;
    }
    else
    {
      sounding_data.pressure = PHYmeters2mb(altitude);
      sounding_data.altitude = altitude;
    }
    
    if (wind_dir == wind_dir_missing_data_value ||
	wind_speed == wind_speed_missing_data_value)
    {
      sounding_data.u = Sndg::VALUE_UNKNOWN;
      sounding_data.v = Sndg::VALUE_UNKNOWN;
    }
    else
    {
      sounding_data.u = -wind_speed * sin(wind_dir * DEG_TO_RAD);
      sounding_data.v = -wind_speed * cos(wind_dir * DEG_TO_RAD);
    }
    
    sounding_data.w = Sndg::VALUE_UNKNOWN;
    
    if (rh == rh_missing_data_value)
      sounding_data.rh = Sndg::VALUE_UNKNOWN;
    else
      sounding_data.rh = rh;

    if (temperature == temperature_missing_data_value)
      sounding_data.temp = Sndg::VALUE_UNKNOWN;
    else
      sounding_data.temp = TEMP_K_TO_C(temperature);
    
    if (dew_point == dew_point_missing_data_value)
      sounding_data.dewpt = Sndg::VALUE_UNKNOWN;
    else
      sounding_data.dewpt = TEMP_K_TO_C(dew_point);
    
    if (wind_speed == wind_speed_missing_data_value)
      sounding_data.windSpeed = Sndg::VALUE_UNKNOWN;
    else
      sounding_data.windSpeed = wind_speed;
    
    if (wind_dir == wind_dir_missing_data_value)
      sounding_data.windDir = Sndg::VALUE_UNKNOWN;
    else
      sounding_data.windDir = wind_dir;
    
    sounding_data.ascensionRate = Sndg::VALUE_UNKNOWN;
    sounding_data.longitude = longitude;
    sounding_data.latitude = latitude;
    sounding_data.pressureQC = Sndg::VALUE_UNKNOWN;
    sounding_data.tempQC = Sndg::VALUE_UNKNOWN;
    sounding_data.humidityQC = Sndg::VALUE_UNKNOWN;
    sounding_data.uwindQC = Sndg::VALUE_UNKNOWN;
    sounding_data.vwindQC = Sndg::VALUE_UNKNOWN;
    sounding_data.ascensionRateQC = Sndg::VALUE_UNKNOWN;

    for (int j = 0; j < Sndg::PT_SPARE_FLOATS; ++j)
      sounding_data.spareFloats[j] = Sndg::VALUE_UNKNOWN;
    
    sounding.addPoint(sounding_data);

    lat_total += latitude;
    lon_total += longitude;
    ++num_pts;
  } /* endfor - i */
  
  // Reclaim memory

  delete latitudes;
  delete longitudes;
  delete altitudes;
  delete temperatures;
  delete wind_dirs;
  delete wind_speeds;
  delete rhs;
  delete dew_points;
  delete tail_numbers;
  delete data_sources;
  delete sounding_flags;
  delete launch_times;
  
  // Output the final sounding information

  if (num_pts > 0)
  {
    if (_debug)
    {
      cerr << "*** Writing out sounding" << endl;
    }

    // Update the lat/lon values in the header

    Sndg::header_t sounding_header = sounding.getHeader();
	
    sounding_header.lat = lat_total / num_pts;
    sounding_header.lon = lon_total / num_pts;
	
    sounding.setHeader(sounding_header);
	
    // Write out the sounding

    sounding.assemble();
	
    spdb.addPutChunk(0,
		     sounding_header.launchTime,
		     sounding_header.launchTime,  // add some offset
		     sounding.getBufLen(),
		     sounding.getBufPtr());
  }

  return true;
}


/*********************************************************************
 * _sortPoints() - Sort the points in the sounding by pressure.
 */

void TamdarFile::_sortPoints(Sndg &sounding) const
{
  vector< Sndg::point_t > points = sounding.getPoints();
  vector< Sndg::point_t > sorted_points;
  
  vector< Sndg::point_t >::const_iterator point;
  
  for (point = points.begin(); point != points.end(); ++point)
  {
    vector< Sndg::point_t >::iterator sorted_point;
    
    for (sorted_point = sorted_points.begin();
	 sorted_point != sorted_points.end(); ++ sorted_point)
    {
      if (sorted_point->pressure <= point->pressure)
	break;
    } /* endfor - sorted_point */
    
    sorted_points.insert(sorted_point, *point);
    
  } /* endfor - point */

  sounding.setPoints(sorted_points);
}
