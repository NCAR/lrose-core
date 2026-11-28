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

#include "HdfFile.hh"

using namespace std;


// Global constants

const string HdfFile::LAT_VAR_NAME = "Latitude";
const string HdfFile::LON_VAR_NAME = "Longitude";

const string HdfFile::FIELD_LONG_NAME_ATTR_NAME = "long_name";
const string HdfFile::FIELD_UNITS_ATTR_NAME = "units";
const string HdfFile::FIELD_MISSING_VALUE_ATTR_NAME = "_FillValue";

const int HdfFile::MAX_FIELD_NAME_LEN = 64;

const double HdfFile::LAT_OFFSET = -89.5;
const double HdfFile::LON_OFFSET = -179.5;


/*********************************************************************
 * Constructors
 */

HdfFile::HdfFile(const string &file_path, const bool debug_flag) :
  _debug(debug_flag),
  _filePath(file_path),
  _fileId(-1),
  _numLatitudes(0),
  _latitudes(0),
  _numLongitudes(0),
  _longitudes(0)
{
}


/*********************************************************************
 * Destructor
 */

HdfFile::~HdfFile()
{
  // Close the SDS interface if it was successfully opened

  if (_sdId != -1)
    SDend(_sdId);

  // Close the file if it was successfully opened

  if (_fileId != -1)
    Hclose(_fileId);

  // Reclaim memory

  delete [] _latitudes;
  delete [] _longitudes;
}


/*********************************************************************
 * init()
 */

bool HdfFile::init()
{
  static const string method_name = "HdfFile::init()";
  
  if (Hishdf(_filePath.c_str()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _filePath << " is not a valid HDF file, or file not found" << endl;
    
    return false;
  } 

  // Open connections to the TRMM HDF file

  if ((_fileId = Hopen (_filePath.c_str(), DFACC_READ, 0)) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file : " << _filePath << endl;

    return false;
  }

  // Open HDF SDS Interface

  if ((_sdId = SDstart(_filePath.c_str(), DFACC_READ)) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDstart failed." << endl;
    HEprint (stderr, 0);

    return false;
  }

  // Obtain number of SDSs and Global Attributes in input file

  int32 n_datasets;
  int32 n_file_attrs;
  
  if (SDfileinfo(_sdId, &n_datasets, &n_file_attrs) != 0)
  {
    cerr << "SDfileinfo failed." << endl;
    return false;
  }

  // List out SDSs contained in input HDF file

  if (_debug)
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
  
  // Get the lat/lon information from the file

  if (!_getLatLon())
    return false;
  
  return true;
}


/*********************************************************************
 * readSdsData()
 */

fl64 *HdfFile::readSdsData(const string &field_name,
			   vector< int > &dimensions) const
{
  static const string method_name = "HdfFile::readSdsData()";
  
  // Clear out the dimensions vector

  dimensions.clear();
  
  // Get the index of the field within the file

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
  
  // Read the data based on the data type

  fl64 *data;
  
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
  
  return data;
}


/*********************************************************************
 * readSdsData()
 */

fl64 *HdfFile::readSdsData(const string &field_name,
			   vector< int > &dimensions,
			   string &field_long_name,
			   string &field_units,
			   double &missing_data_value) const
{
  static const string method_name = "HdfFile::readSdsData()";
  
  // Clear out the dimensions vector

  dimensions.clear();
  
  // Get the index of the field within the file

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
  
  // Get the field attributes that we want to send back to the user.

  if (!_getStringAttribute(sds_id, FIELD_LONG_NAME_ATTR_NAME,
			   field_long_name))
  {
    SDendaccess(sds_id);
    
    return false;
  }
  
  if (!_getStringAttribute(sds_id, FIELD_UNITS_ATTR_NAME,
			   field_units))
  {
    SDendaccess(sds_id);
    
    return false;
  }
  
  if (!_getFloatAttribute(sds_id, FIELD_MISSING_VALUE_ATTR_NAME,
			  missing_data_value))
  {
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
  
  // Read the data based on the data type

  fl64 *data;
  
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
 * _getLatLon()
 */

bool HdfFile::_getLatLon()
{
  static const string method_name = "HdfFile::_getLatLon()";
  
  vector< int > dimensions;
  
  // Read the latitudes

  float64 *latitudes;
  
  if ((latitudes = readSdsData(LAT_VAR_NAME, dimensions)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << LAT_VAR_NAME << " variable from file: "
	 << _filePath << endl;
    
    return false;
  }
  
  if (dimensions.size() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of dimensions in " << LAT_VAR_NAME
	 << " variable" << endl;
    cerr << "Expected 1 dimension, found " << dimensions.size()
	 << " dimensions" << endl;
    
    delete [] latitudes;
    
    return false;
  }
  
  _numLatitudes = dimensions[0];
  
  delete [] _latitudes;
  _latitudes = new float[_numLatitudes];
  
  for (int i = 0; i < _numLatitudes; ++i)
    _latitudes[i] = (float)latitudes[i];
  
  delete [] latitudes;
  
  // Read the longitudes

  float64 *longitudes;
  
  if ((longitudes = readSdsData(LON_VAR_NAME, dimensions)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << LON_VAR_NAME << " variable from file: "
	 << _filePath << endl;
    
    return false;
  }
  
  if (dimensions.size() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of dimensions in " << LON_VAR_NAME
	 << " variable" << endl;
    cerr << "Expected 1 dimension, found " << dimensions.size()
	 << " dimensions" << endl;
    
    delete [] longitudes;
    
    return false;
  }
  
  _numLongitudes = dimensions[0];
  
  delete [] _longitudes;
  _longitudes = new float[_numLongitudes];
  
  for (int i = 0; i < _numLongitudes; ++i)
    _longitudes[i] = (float)longitudes[i];
  
  delete [] longitudes;
  
  // Determine the lat/lon projection used for this data

  double min_lat = _latitudes[0] + LAT_OFFSET;
  double min_lon = _longitudes[0] + LON_OFFSET;
  double minx = min_lon;
  double miny = min_lat;
  double dx = _longitudes[1] - _longitudes[0];
  double dy = _latitudes[1] - _latitudes[0];
  int nx = _numLongitudes;
  int ny = _numLatitudes;
  
  _proj.initLatlon(nx, ny, 1,
		   dx, dy, 1.0,
		   minx, miny, 0.0);
  
  return true;
}


/*********************************************************************
 * _getFloatAttribute()
 */

bool HdfFile::_getFloatAttribute(const int32 sds_id,
				 const string &attr_name,
				 double &float_attr) const
{
  static const string method_name = "HdfFile::_getFloatAttribute()";

  int32 attr_index;

  if ((attr_index = SDfindattr(sds_id, attr_name.c_str())) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error finding " << attr_name << " attribute" << endl;
    
    return false;
  }
  
  char attr_name_buffer[MAX_FIELD_NAME_LEN];
  int32 data_type;
  int32 n_values;
  
  if (SDattrinfo(sds_id, attr_index,
		 attr_name_buffer, &data_type, &n_values) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting info about " << attr_name << " attribute" << endl;
    
    return false;
  }
  
  if (data_type != DFNT_FLOAT32)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incorrect data type for string attribute" << endl;
    cerr << "Expected type " << DFNT_FLOAT32 << endl;
    cerr << "Found type " << data_type << endl;
    
    return false;
  }
  
  if (n_values != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of values for attribute " << attr_name << endl;
    cerr << "Expected 1 value, found " << n_values << " values" << endl;
    
    return false;
  }
  
  float32 attr_buffer;
  
  if (SDreadattr(sds_id, attr_index,
		 (void *)&attr_buffer)  == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving " << attr_name << " attribute" << endl;

    return false;
  }

  if (_debug)
    cerr << attr_name << " = <" << attr_buffer << ">" << endl;
  
  float_attr = attr_buffer;
  
  return true;
}


/*********************************************************************
 * _getStringAttribute()
 */

bool HdfFile::_getStringAttribute(const int32 sds_id,
				  const string &attr_name,
				  string &string_attr) const
{
  static const string method_name = "HdfFile::_getStringAttribute()";

  int32 attr_index;

  if ((attr_index = SDfindattr(sds_id, attr_name.c_str())) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error finding " << attr_name << " attribute" << endl;
    
    return false;
  }
  
  char attr_name_buffer[MAX_FIELD_NAME_LEN];
  int32 data_type;
  int32 n_values;
  
  if (SDattrinfo(sds_id, attr_index,
		 attr_name_buffer, &data_type, &n_values) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting info about " << attr_name << " attribute" << endl;
    
    return false;
  }
  
  if (data_type != DFNT_CHAR)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incorrect data type for string attribute" << endl;
    cerr << "Expected type " << DFNT_CHAR << endl;
    cerr << "Found type " << data_type << endl;
    
    return false;
  }
  
  char attr_buffer[n_values + 1];
  memset(attr_buffer, 0, n_values + 1);
  
  if (SDreadattr(sds_id, attr_index,
		 (void *)attr_buffer)  == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving " << attr_name << " attribute" << endl;

    return false;
  }

  if (_debug)
    cerr << attr_name << " = <" << attr_buffer << ">" << endl;
  
  string_attr = attr_buffer;
  
  return true;
}


/*********************************************************************
 * _readFloat32Data()
 */

fl64 *HdfFile::_readFloat32Data(const string &field_name,
				int32 sds_id,
				int32 num_elements,
				int32 *edges, int32 *stride,
				int32 *start) const
{
  static const string method_name = "HdfFile::_readFloat32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  float32 *float32_data = new float32[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, float32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] float32_data;
    
    return 0;
  }

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)float32_data[i];
  
  delete [] float32_data;
  
  return return_data;
}


/*********************************************************************
 * _readFloat64Data()
 */

fl64 *HdfFile::_readFloat64Data(const string &field_name,
				int32 sds_id,
				int32 num_elements,
				int32 *edges, int32 *stride,
				int32 *start) const
{
  static const string method_name = "HdfFile::_readFloat32Data()";

  // Allocate space for the data

  float64 *float64_data = new float64[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, float64_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] float64_data;
    
    return 0;
  }

  // Convert the data to fl64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)float64_data[i];
  
  delete [] float64_data;
  
  return return_data;
}


/*********************************************************************
 * _readInt8Data()
 */

fl64 *HdfFile::_readInt8Data(const string &field_name,
			     int32 sds_id,
			     int32 num_elements,
			     int32 *edges, int32 *stride,
			     int32 *start) const
{
  static const string method_name = "HdfFile::_readInt8Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  int8 *int8_data = new int8[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, int8_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int8_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

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
			      int32 sds_id,
			      int32 num_elements,
			      int32 *edges, int32 *stride,
			      int32 *start) const
{
  static const string method_name = "HdfFile::_readInt16Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  int16 *int16_data = new int16[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, int16_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int16_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

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
			      int32 sds_id,
			      int32 num_elements,
			      int32 *edges, int32 *stride,
			      int32 *start) const
{
  static const string method_name = "HdfFile::_readInt32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  int32 *int32_data = new int32[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, int32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] int32_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

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
			      int32 sds_id,
			      int32 num_elements,
			      int32 *edges, int32 *stride,
			      int32 *start) const
{
  static const string method_name = "HdfFile::_readUint8Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  uint8 *uint8_data = new uint8[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, uint8_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint8_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

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
			       int32 sds_id,
			       int32 num_elements,
			       int32 *edges, int32 *stride,
			       int32 *start) const
{
  static const string method_name = "HdfFile::_readUint16Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  uint16 *uint16_data = new uint16[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, uint16_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint16_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

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
			       int32 sds_id,
			       int32 num_elements,
			       int32 *edges, int32 *stride,
			       int32 *start) const
{
  static const string method_name = "HdfFile::_readUint32Data()";

  // Allocate space for the temporary array that stores the data in its
  // original format

  uint32 *uint32_data = new uint32[num_elements];
  
  // Read the data from the file

  if (SDreaddata(sds_id, start, 0, edges, uint32_data) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SDreaddata failed for field: " << field_name << endl;

    delete [] uint32_data;
    
    return 0;
  }

  // Convert the data to float64 format for returning to the main program

  fl64 *return_data = new fl64[num_elements];
  
  for (int i = 0; i < num_elements; ++i)
    return_data[i] = (fl64)uint32_data[i];
  
  delete [] uint32_data;
  
  return return_data;
}
