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
#include <netcdf.h>

#include <toolsa/Path.hh>

#include "HdfFile.hh"

using namespace std;


// Global constants

const int HdfFile::MAX_FIELD_NAME_LEN = 64;
const int HdfFile::MAX_FIELD_LIST_LEN = 1024;

const string HdfFile::SCAN_TIME_VDATA_NAME = "scan_time";
const string HdfFile::SCAN_TIME_FIELD_LIST1 = "year,month,dayOfMonth,hour,minute,second,dayOfYear";
const string HdfFile::SCAN_TIME_FIELD_LIST2 = "scanTime";

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
  _scanTimesRead(false)
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

  // Close the FDATA interface

  Vend(_fileId);
  
  // Close the file if it was successfully opened

  if (_fileId != -1)
    Hclose(_fileId);
}


/*********************************************************************
 * init()
 */

bool HdfFile::init()
{
  static const string method_name = "HdfFile::init()";
  
  if (_verbose)
    cerr << "Checking file to make sure it is an HDF file" << endl;
  
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
  
  return true;
}


/*********************************************************************
 * getScanTimeRange()
 */

bool HdfFile::getScanTimeRange(DateTime &begin_time, DateTime &end_time)
{
  static const string method_name = "HdfFile::getScanTimes()";
  
  // Read the scan times data

  if (!_readScanTimes())
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
 * getSunMags()
 */

bool HdfFile::getSunMags(vector< double> &sun_mags)
{
  static const string method_name = "HdfFile::getSunMags()";
  
  // Initialize the return vector

  sun_mags.clear();
  
  // Get the reference number for this VDATA

  int32 vdata_ref;
  
  if ((vdata_ref = VSfind(_fileId, SOLAR_CAL_VDATA_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find " << SOLAR_CAL_VDATA_NAME << " VDATA in HDF file: "
	 << _filePath << endl;
    
    return false;
  }
  
  // Attach the VDATA

  int32 vdata_id;
  
  if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error attaching to " << SOLAR_CAL_VDATA_NAME << " vdata." << endl;
    
    return false;
  }
  
  // Get the information about the scan time VDATA

  int32 n_records;
  int32 interlace;
  
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
  
  // Prepare the sun magnitudes vector

  sun_mags.reserve(n_records);
  
  // Read the sun mag values

  if (VSsetfields(vdata_id, SOLAR_CAL_SUN_MAG_FIELD_NAME.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting <" << SOLAR_CAL_SUN_MAG_FIELD_NAME
	 << "> field list for " << SOLAR_CAL_VDATA_NAME << " VDATA" << endl;
    
    return false;
  }
  
  float64 *sun_mag_values = new float64[n_records];
  
  if (VSread(vdata_id, (uint8 *)sun_mag_values,
	     n_records, interlace) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading sun mag values from HDF file" << endl;
    
    delete [] sun_mag_values;
    
    return false;
  }
  
  // Set the data in our sun mags vector

  for (int i = 0; i < n_records; ++i)
  {
    sun_mags.push_back((double)sun_mag_values[i]);
  } /* endfor - i */
  
  delete [] sun_mag_values;
  
  // Detach the scan time VDATA

  if (VSdetach(vdata_id) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error detaching from " << SOLAR_CAL_VDATA_NAME << " vdata." << endl;
    
    return false;
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
 * _readScanTimes1()
 */

bool HdfFile::_readScanTimes1(const int32 vdata_id,
			      const int32 n_records, const int32 interlace)
{
  static const string method_name = "HdfFile::_readScanTimes1()";
  
  // Read the scan time values

  int8 *scan_fields = new int8[9 * n_records];
  
  if (VSsetfields(vdata_id, SCAN_TIME_FIELD_LIST1.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting <" << SCAN_TIME_FIELD_LIST1
	 << "> field list for " << SCAN_TIME_VDATA_NAME << " VDATA" << endl;
    
    delete [] scan_fields;
    
    return false;
  }
  
  if (VSread(vdata_id, (uint8 *)scan_fields,
	     n_records, interlace) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading scan time fields" << endl;
    
    delete [] scan_fields;
    
    return false;
  }
  
  // Unpack the fields

  int16 *years = new int16[n_records];
  int8 *months = new int8[n_records];
  int8 *days = new int8[n_records];
  int8 *hours = new int8[n_records];
  int8 *minutes = new int8[n_records];
  int8 *seconds = new int8[n_records];
  int16 *days_of_year = new int16[n_records];
  
  VOIDP buf_ptrs[7];
  buf_ptrs[0] = years;
  buf_ptrs[1] = months;
  buf_ptrs[2] = days;
  buf_ptrs[3] = hours;
  buf_ptrs[4] = minutes;
  buf_ptrs[5] = seconds;
  buf_ptrs[6] = days_of_year;
  
  if (VSfpack(vdata_id, _HDF_VSUNPACK, SCAN_TIME_FIELD_LIST1.c_str(),
	      scan_fields, 9 * n_records * sizeof(int8), n_records,
	      SCAN_TIME_FIELD_LIST1.c_str(), buf_ptrs) != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error unpacking scan time fields" << endl;
    
    delete [] scan_fields;
    delete [] years;
    delete [] months;
    delete [] days;
    delete [] hours;
    delete [] minutes;
    delete [] seconds;
    delete [] days_of_year;
  
    return false;
  }
  
  delete [] scan_fields;

  // Pull out the time information and put it into our vector

  for (int i = 0; i < n_records; ++i)
  {
    _scanTimes.push_back(DateTime(years[i], months[i], days[i],
				  hours[i], minutes[i], seconds[i]));
  } /* endfor - i */
  
  // Clean up memory

  delete [] years;
  delete [] months;
  delete [] days;
  delete [] hours;
  delete [] minutes;
  delete [] seconds;
  delete [] days_of_year;
  
  return true;
}


/*********************************************************************
 * _readScanTimes2()
 */

bool HdfFile::_readScanTimes2(const int32 vdata_id,
			      const int32 n_records, const int32 interlace)
{
  static const string method_name = "HdfFile::_readScanTimes2()";
  
  // Get the date information from the file name

  DateTime file_time;
  
  if ((file_time = _getDateFromFilename()) == DateTime::NEVER)
    return false;
  
  // Read the scan time values

  if (VSsetfields(vdata_id, SCAN_TIME_FIELD_LIST2.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting <" << SCAN_TIME_FIELD_LIST2
	 << "> field list for " << SCAN_TIME_VDATA_NAME << " VDATA" << endl;
    
    return false;
  }
  
  float64 *secs_of_day = new float64[n_records];
  
  if (VSread(vdata_id, (uint8 *)secs_of_day,
	     n_records, interlace) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading scan time fields" << endl;
    
    delete [] secs_of_day;
    
    return false;
  }
  
  // Set the data in our scan times vector

  if (secs_of_day[0] > secs_of_day[n_records - 1])
    file_time -= 86400;
  float64 prev_secs_of_day = 0.0;
  
  for (int i = 0; i < n_records; ++i)
  {
    float64 curr_secs_of_day = secs_of_day[i];
    
    if (i != 0 && curr_secs_of_day < prev_secs_of_day)
      file_time += 86400;
    
    _scanTimes.push_back(file_time + (int)secs_of_day[i]);

    prev_secs_of_day = curr_secs_of_day;
  } /* endfor - i */
  
  delete [] secs_of_day;
  
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
 * _readScanTimes()
 */

bool HdfFile::_readScanTimes()
{
  static const string method_name = "HdfFile::_readScanTimes()";
  
  if (_scanTimesRead)
    return true;
  
  // Clear out the scan times vector

  _scanTimes.clear();
  
  // Get the reference number for this VDATA

  int32 vdata_ref;
  
  if ((vdata_ref = VSfind(_fileId, SCAN_TIME_VDATA_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find " << SCAN_TIME_VDATA_NAME << " VDATA in HDF file: "
	 << _filePath << endl;
    
    return false;
  }
  
  // Attach the scan time VDATA

  int32 vdata_id;
  
  if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error attaching to " << SCAN_TIME_VDATA_NAME << " vdata." << endl;
    
    return false;
  }
  
  // Get the information about the scan time VDATA

  char fields[MAX_FIELD_LIST_LEN];
  
  if (VSQueryfields(vdata_id, fields) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting scan_time field names from TRMM file" << endl;
    
    VSdetach(vdata_id);
    
    return false;
  }
  
  // Get the information about the scan time VDATA

  int32 n_records;
  int32 interlace;
  
  if (VSQueryinterlace(vdata_id, &interlace) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting interlace method for scan_time VDATA" << endl;
    
    return false;
  }
  
  if (VSQuerycount(vdata_id, &n_records) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting number of scan_time VDATA records" << endl;
    
    return false;
  }
  
  if (_verbose)
    cerr << "  scan_time fields: <" << fields << ">" << endl;
  
  // Prepare the scan times vector

  _scanTimes.reserve(n_records);
  
  // Retrieve the scan times based on how the times are stored in the
  // TRMM file

  string field_list = fields;
  
  if (field_list == SCAN_TIME_FIELD_LIST1)
  {
    if (_verbose)
      cerr << "---> Using scan time field list 1" << endl;
    
    if (!_readScanTimes1(vdata_id, n_records, interlace))
    {
      VSdetach(vdata_id);
      _scanTimes.clear();
      return false;
    }
  }
  else if (field_list == SCAN_TIME_FIELD_LIST2)
  {
    if (_verbose)
      cerr << "---> Using scan time field list 2" << endl;
    
    if (!_readScanTimes2(vdata_id, n_records, interlace))
    {
      VSdetach(vdata_id);
      _scanTimes.clear();
      return false;
    }
  }
  else
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process scan time data with the following list of fields:" << endl;
    cerr << "   <" << fields << ">" << endl;
    
    VSdetach(vdata_id);
    _scanTimes.clear();
    return false;
  }
  
  // Detach the scan time VDATA

  if (VSdetach(vdata_id) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error detaching from " << SCAN_TIME_VDATA_NAME << " vdata." << endl;
    
    _scanTimes.clear();
    return false;
  }
  
//  if (_verbose)
//  {
//    cerr << "Scan times;" << endl;
//    
//    vector< DateTime >::const_iterator scan_time;
//    
//    for (scan_time = _scanTimes.begin(); scan_time != _scanTimes.end();
//	 ++scan_time)
//      cerr << "   " << *scan_time << endl;
//  }
  
  _scanTimesRead = true;
  
  return true;
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
