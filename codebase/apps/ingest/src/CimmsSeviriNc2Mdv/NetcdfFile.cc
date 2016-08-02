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
 * Class controlling access to a CIMMS netCDF file.
 *  
 * @date 11/17/2008
 *
 */

#include <iostream>

#include <toolsa/Path.hh>
#include <toolsa/str.h>

#include "NetcdfFile.hh"

using namespace std;


// Global constants

const string NetcdfFile::IMAGE_DATE_VAR_NAME = "imageDate";
const string NetcdfFile::IMAGE_TIME_VAR_NAME = "imageTime";

const string NetcdfFile::X_DIM_NAME = "elems";
const string NetcdfFile::Y_DIM_NAME = "lines";
const string NetcdfFile::BANDS_DIM_NAME = "bands";

const string NetcdfFile::LAT_VAR_NAME = "latitude";
const string NetcdfFile::LON_VAR_NAME = "longitude";

const string NetcdfFile::BAND_NUMS_VAR_NAME = "bands";
const string NetcdfFile::DATA_VAR_NAME = "data";
const string NetcdfFile::BANDS_UNITS_ATT_NAME = "units";


/*********************************************************************
 * Constructors
 */

NetcdfFile::NetcdfFile(const string &file_path,
		       const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _filePath(file_path),
  _ncFile(file_path.c_str()),
  _nx(-1),
  _ny(-1),
  _numBands(-1),
  _latitudes(0),
  _longitudes(0)
{
}


/*********************************************************************
 * Destructor
 */

NetcdfFile::~NetcdfFile()
{
  // Reclaim memory

  delete [] _latitudes;
  delete [] _longitudes;
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

  NcError nc_error(NcError::silent_nonfatal);

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
  
  // Get the latitude/longitude data

  if (!_getLatLon())
    return false;
  
  return true;
}


/*********************************************************************
 * getBandsAsMdv()
 */

bool NetcdfFile::getBandsAsMdv(vector< MdvxField* > &fields,
			       const MdvxPjg &mdv_proj,
			       GridHandler &grid_handler,
			       const int output_grib_code) const
{
  static const string method_name = "NetcdfFile::getBandsAsMdv()";

  // Get the band variable from the netCDF file

  NcVar *bands_var;
  
  if ((bands_var = _ncFile.get_var(DATA_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << DATA_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return false;
  }
  
  NcAtt *units_att;
  
  if ((units_att = bands_var->get_att(BANDS_UNITS_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << BANDS_UNITS_ATT_NAME << " attribute from file "
	 << _filePath << endl;
    
    return false;
  }
  
  string units = units_att->as_string(0);
  delete units_att;
  
  // Process each of the included bands

  for (int band_num = 0; band_num < _numBands; ++band_num)
  {
    grid_handler.init();
    
    // Get the data for this band from the netCDF file

    if (!bands_var->set_cur(band_num, 0, 0))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error setting current band in netCDF file" << endl;
      
      return false;
    }
    
    float *band_nc_data = new float[_nx * _ny];
  
    if (!bands_var->get(band_nc_data, 1, _ny, _nx))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting band " << band_num << " data from file "
	   << _filePath << endl;
    
      delete [] band_nc_data;
      
      return false;
    }
  
    int num_nc_elements = _nx * _ny;
    
    if (_debug)
      cerr << "---> Adding data to grid" << endl;
    
    for (int i = 0; i < num_nc_elements; ++i)
    {
      // Check for valid data

      if (_latitudes[i] < -90.0 || _latitudes[i] > 90.0 ||
	  _longitudes[i] < -180.0 || _longitudes[i] > 180.0)
	continue;
      
      // Add the data value to the grid

      grid_handler.addData(_latitudes[i], _longitudes[i], band_nc_data[i]);
      
    } /* endfor - i */
  
    delete [] band_nc_data;
  
    // Create the MDV field

    if (_debug)
      cerr << "---> Creating MDV field" << endl;
    
    Mdvx::field_header_t field_hdr;
    if (!_updateFieldHeader(band_num, field_hdr, mdv_proj, units,
			    output_grib_code))
      return false;
  
    Mdvx::vlevel_header_t vlevel_hdr;
    memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
    vlevel_hdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
    vlevel_hdr.level[0] = 0.0;
    
    MdvxField *mdv_field;

    if ((mdv_field = new MdvxField(field_hdr, vlevel_hdr,
				   (void *)grid_handler.getGrid())) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating MDV field" << endl;
      
      return false;
    }
    
    fields.push_back(mdv_field);
    
  } /* endfor - band_num */
  
  return true;
}


/*********************************************************************
 * getImageTime()
 */

DateTime NetcdfFile::getImageTime() const
{
  static const string method_name = "NetcdfFile::getImageTime()";
  
  // Get the image date variable from the file

  NcVar *date_var;
  
  if ((date_var = _ncFile.get_var(IMAGE_DATE_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << IMAGE_DATE_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return DateTime::NEVER;
  }
  
  int image_date;
  
  if (!date_var->get(&image_date, 1))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting image date value from file " << _filePath << endl;
    
    return DateTime::NEVER;
  }
  
  // Get the image time variable from the file

  NcVar *time_var;
  
  if ((time_var = _ncFile.get_var(IMAGE_TIME_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << IMAGE_TIME_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return DateTime::NEVER;
  }
  
  int image_time;
  
  if (!time_var->get(&image_time, 1))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting image time value from file " << _filePath << endl;
    
    return DateTime::NEVER;
  }
  
  // Extract the date/time information from the variables

  if (_debug)
  {
    cerr << "image date = " << image_date << endl;
    cerr << "image time = " << image_time << endl;
  }
  
  int year = image_date / 1000;
  int jday = image_date % 1000;
  int month;
  int day;
  
  DateTime::getMonthDay(year, jday, month, day);
  
  int hour = image_time / 10000;
  int minute = (image_time % 10000) / 100;
  int second = image_time % 100;
  
  if (_debug)
  {
    cerr << "Year = " << year << endl;
    cerr << "jday = " << jday << endl;
    cerr << "month = " << month << endl;
    cerr << "day = " << day << endl;
    cerr << "hour = " << hour << endl;
    cerr << "minute = " << minute << endl;
    cerr << "second = " << second << endl;
  }
  
  return DateTime(year, month, day, hour, minute, second);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getDimensions()
 */

bool NetcdfFile::_getDimensions()
{
  static const string method_name = "NetcdfFile::_getDimensions()";
  
  // Get the X/Y dimensions

  NcDim *x_dim;

  if ((x_dim = _ncFile.get_dim(X_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << X_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _nx = x_dim->size();
  
  NcDim *y_dim;

  if ((y_dim = _ncFile.get_dim(Y_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << Y_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _ny = y_dim->size();
  
  if (_debug)
    cerr << "   nx = " << _nx << ", ny = " << _ny << endl;
  
  // Get the number of bands from the file

  NcDim *bands_dim;

  if ((bands_dim = _ncFile.get_dim(BANDS_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << BANDS_DIM_NAME << " dimension from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _numBands = bands_dim->size();
  
  if (_debug)
    cerr << "   num bands = " << _numBands << endl;
  
  return true;
}


/*********************************************************************
 * _getLatLon()
 */

bool NetcdfFile::_getLatLon()
{
  static const string method_name = "NetcdfFile::_getLatLon()";
  
  // Get the latitude values

  NcVar *lat_var;
  
  if ((lat_var = _ncFile.get_var(LAT_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _latitudes = new float[_nx * _ny];
  
  if (!lat_var->get(_latitudes, _ny, _nx))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting latitude data from file " << _filePath << endl;
    
    delete [] _latitudes;
    _latitudes = 0;
    
    return false;
  }
  
  // Get the longitude values

  NcVar *lon_var;
  
  if ((lon_var = _ncFile.get_var(LON_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LON_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return false;
  }
  
  _longitudes = new float[_nx * _ny];
  
  if (!lon_var->get(_longitudes, _ny, _nx))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting longitude data from file " << _filePath << endl;
    
    delete [] _longitudes;
    _longitudes = 0;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateFieldHeader()
 */

bool NetcdfFile::_updateFieldHeader(const int band_index,
				    Mdvx::field_header_t &field_hdr,
				    const MdvxPjg &mdv_proj,
				    const string &units,
				    const int output_grib_code) const
{
  static const string method_name = "CimmsSeviriNc2Mdv::_updateFieldHeader()";

  // Get the band number from the netCDF file

  NcVar *band_nums_var;
  
  if ((band_nums_var = _ncFile.get_var(BAND_NUMS_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << BAND_NUMS_VAR_NAME << " variable from file "
	 << _filePath << endl;
    
    return false;
  }
  
  if (!band_nums_var->set_cur(band_index))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting current band in netCDF file" << endl;
    
    return false;
  }
  
  int band_num;
  
  if (!band_nums_var->get(&band_num, 1))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting band number for band " << band_index
	 << " from file " << _filePath << endl;
    
    return false;
  }
  
  // Update the field header information

  memset(&field_hdr, 0, sizeof(field_hdr));
  
  mdv_proj.syncToFieldHdr(field_hdr);
  
  field_hdr.field_code = output_grib_code;
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  field_hdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = GridHandler::MISSING_DATA_VALUE;
  field_hdr.missing_data_value = GridHandler::MISSING_DATA_VALUE;
  sprintf(field_hdr.field_name_long, "band%d", band_num);
  sprintf(field_hdr.field_name, "band%d", band_num);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  return true;
}
