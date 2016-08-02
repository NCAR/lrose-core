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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:00 $
//   $Id: FieldHandler.cc,v 1.2 2016/03/07 01:23:00 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FieldHandler: Base class for classes that handle fields in the NSSL
 *               mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <netcdf.h>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/MdvxPjg.hh>
#include "FieldHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

FieldHandler::FieldHandler(const string field_name,
			   const bool debug_flag) :
  _debug(debug_flag),
  _fieldName(field_name)
{
}


/**********************************************************************
 * Destructor
 */

FieldHandler::~FieldHandler(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _extractDimensionValue() - Extract the value for the given dimension
 *                            from the given netCDF file.
 *
 * Returns the dimension value on success, -1 on failure.
 */

int FieldHandler::_extractDimensionValue(const int nc_file_id,
					 const string &dim_name,
					 const bool print_error)
{
  int dim_id;

  int status = nc_inq_dimid(nc_file_id, dim_name.c_str(), &dim_id);
  if (status != NC_NOERR)
  {
    if (print_error)
      cerr << "Failed to get dimension ID for " << dim_name << endl;
    return -1;
  }
 
  size_t dim_value;
 
  status = nc_inq_dimlen(nc_file_id, dim_id, &dim_value);
  if (status != NC_NOERR)
  {
    if (print_error)
      cerr << "Failed to get dimension length for " << dim_name << endl;
    return -1;
  }
 
  return (int)dim_value;
 
}


/**********************************************************************
 * _extractFloatGlobalAtt() - Extract the given float global attribute
 *                            from the given netCDF file.
 *
 * Returns true on success, false on failure.
 */

bool FieldHandler::_extractFloatGlobalAtt(const int nc_file_id,
					  const string &att_name,
					  float &att_value)
{
  if (nc_get_att_float(nc_file_id, NC_GLOBAL,
		       att_name.c_str(), &att_value) != NC_NOERR)
  {
    cerr << "Error extracting global attribute " << att_name << endl;
    return false;
  }
  
  return true;
}


/**********************************************************************
 * _extractFloatVariableArray() - Extract the data for the given float
 *                                variable that is an array.  Returns
 *                                the value of the given missing data
 *                                attribute and scales the data using 
 *                                the given scale attribute, if specified.
 *
 * Returns a pointer to the extracted data on successs, 0 on failure.
 */

float *FieldHandler::_extractFloatVariableArray(const int nc_file_id,
						const string &var_name,
						const int array_size,
						const bool print_error) const
{
  float missing_value;
  
  return _extractFloatVariableArray(nc_file_id, var_name, array_size,
				    "", missing_value, "", print_error);
}


float *FieldHandler::_extractFloatVariableArray(const int nc_file_id,
						const string &var_name,
						const int array_size,
						const string &missing_value_att_name,
						float &missing_value,
						const string scale_att_name,
						const bool print_error) const
{
  // Get access to the given variable

  int var_id;

  if (nc_inq_varid(nc_file_id, var_name.c_str(), &var_id) != NC_NOERR)
  {
    if (print_error)
      cerr << "Failed to get ID for variable " << var_name << endl;
    return 0;
  }

  // Read the data

  float *data = new float[array_size];

  if (nc_get_var_float(nc_file_id, var_id, data) != NC_NOERR)
  {
    if (print_error)
      cerr << "Failed to read variable " << var_name << endl;
    delete [] data;
    return 0;
  }

  // Get the missing value.

  if (missing_value_att_name != "")
  {	
    if (nc_get_att_float(nc_file_id, var_id,
			 missing_value_att_name.c_str(), &missing_value)
	!= NC_NOERR)
    {
      if (print_error)
	cerr << "Failed to get missing_value attribute ("
	     << missing_value_att_name << ") for field " << var_name << endl;
      delete [] data;
      return 0;
    }
      
    if(_debug)
	cerr << "MissingData = " << missing_value << endl;
  }
  
  // Get the scale value. Apply it if we get it.

  if (scale_att_name != "")
  {
    float scale_val;

    if (nc_get_att_float(nc_file_id, var_id,
			 scale_att_name.c_str(), &scale_val) != NC_NOERR)
    {
      if (_debug)
	cerr << "Did not get scale value - leaving data unscaled." << endl;
    }
    else
    {
      if (_debug)
	  cerr << "scale value = " << scale_val <<  endl;

      for (int i = 0; i < array_size; ++i)
      {
	if (missing_value_att_name != "" &&
	    data[i] == missing_value)
	  continue;
	
	  data[i] = data[i] / scale_val;
      } /* endfor - i */
    }
  }

  return data;
}


/**********************************************************************
 * _extractIntVariableArray() - Extract the data for the given integer
 *                              variable that is an array.  Returns
 *                              the value of the given missing data
 *                              attribute and scales the data using 
 *                              the given scale attribute, if specified.
 *
 * Returns a pointer to the extracted data on successs, 0 on failure.
 */

int *FieldHandler::_extractIntVariableArray(const int nc_file_id,
					    const string &var_name,
					    const int array_size)
{
  int missing_value;
  
  return _extractIntVariableArray(nc_file_id, var_name, array_size,
				  "", missing_value);
}


int *FieldHandler::_extractIntVariableArray(const int nc_file_id,
					    const string &var_name,
					    const int array_size,
					    const string &missing_value_att_name,
					    int &missing_value)
{
  // Get access to the given variable

  int var_id;

  if (nc_inq_varid(nc_file_id, var_name.c_str(), &var_id) != NC_NOERR)
  {
    cerr << "Failed to get ID for variable " << var_name << endl;
    return 0;
  }

  // Read the data

  int *data = new int[array_size];

  if (nc_get_var_int(nc_file_id, var_id, data) != NC_NOERR)
  {
    cerr << "Failed to read variable " << var_name << endl;
    delete [] data;
    return 0;
  }

  // Get the missing value.

  if (missing_value_att_name != "")
  {
    if (nc_get_att_int(nc_file_id, var_id,
		       missing_value_att_name.c_str(), &missing_value)
	!= NC_NOERR)
    {
      cerr << "Failed to get missing_value attribute ("
	   << missing_value_att_name << ") for field " << var_name << endl;
      delete [] data;
      return 0;
    }
  }
  
  return data;
}


/**********************************************************************
 * _extractStringVariableArray() - Extract the data for the given string
 *                                 variable that is an array.
 *
 * Returns a vector containing all of the strings in the variable.  If
 * there was an error, the returned vector will be empty.
 */

vector< string> FieldHandler::_extractStringVariableArray(const int nc_file_id,
							  const string &var_name,
							  const string &string_size_dim_name,
							  const int num_strings)
{
  vector< string > file_strings;
  
  // Allocate space for the text buffer that we will get from
  // the netCDF file.

  int string_size = _extractDimensionValue(nc_file_id,
					   string_size_dim_name);
  if (string_size < 0)
    return file_strings;
  
  char *text_buffer = new char[num_strings * string_size];
  
  // Get a pointer to the text array variable

  int var_id;
  
  if (nc_inq_varid(nc_file_id, var_name.c_str(), &var_id) != NC_NOERR)
  {
    cerr << "Failed to get ID for variable " << var_name << endl;
    delete [] text_buffer;
    return file_strings;
  }

  // Get the text buffer from the netCDF file

  if (nc_get_var_text(nc_file_id, var_id, text_buffer) != NC_NOERR)
  {
    cerr << "Failed to read variable " << var_name << endl;
    delete [] text_buffer;
    return file_strings;
  }

  // Unpack the strings

  char *string_buffer = new char[string_size + 1];
    
  for (int i = 0; i < num_strings; ++i)
  {
    for (int j = 0; j < string_size; ++j)
    {
      string_buffer[j] = text_buffer[i * string_size + j];
    }

    string_buffer[string_size + 1] = '\0';
    
    file_strings.push_back(string_buffer);
  }

  delete [] text_buffer;
  delete [] string_buffer;
  
  return file_strings;
}


/**********************************************************************
 * _extractStringVariableAtt() - Extract the given string attribute of
 *                               the given variable.
 *
 * Returns the extracted string on success, "" on failure.
 */

string FieldHandler::_extractStringVariableAtt(const int nc_file_id,
					       const string &var_name,
					       const string &att_name)
{
  // Get a pointer to the variable

  int var_id;
  
  if (nc_inq_varid(nc_file_id, var_name.c_str(), &var_id) != NC_NOERR)
  {
    cerr << "Failed to get ID for variable " << var_name << endl;
    return "";
  }

  // Get the attribute value.  I don't know how to get the string
  // length, so I'm going to assume it won't be longer than 1K

  char *att_value_string = new char[1024];
  memset(att_value_string, 0, 1024);
  
  if (nc_get_att_text(nc_file_id, var_id,
		      att_name.c_str(), att_value_string) != NC_NOERR)
  {
    cerr << "Failed to read attribute " << att_name 
	 << " from variable " << var_name << endl;
    delete [] att_value_string;
    return "";
  }

  string att_value = att_value_string;
  delete [] att_value_string;
  
  return att_value;
}

/**********************************************************************
 * extractField() - Extract the field from the given netCDF file.
 */

MdvxField *FieldHandler::extractField(const int nc_file_id)
{
  // Get the number of data times in the file.  Currently, we can
  // only handle files with a single data time.

  int num_times;
  
  if ((num_times = _extractDimensionValue(nc_file_id, "n_valtimes")) != 1)
  {
    cerr << "Multiple times in the file - skipping file." << endl;
    return 0;
  }

  // Get the dimension lengths.

  int nx  = _extractDimensionValue(nc_file_id, "x");
  int ny  = _extractDimensionValue(nc_file_id, "y");
  int nz  = _extractDimensionValue(nc_file_id, "levels_1");

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  

  // Extract the dx/dy values from the netCDF file.

  float dx, dy;
  float min_lat, min_lon;
  float max_lat, max_lon;
  float c_lat, c_lon;
  
   if (!_extractFloatGlobalAtt(nc_file_id, "centralLat", c_lat))
     return 0;
   if (!_extractFloatGlobalAtt(nc_file_id, "centralLon", c_lon))
     return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lat00", min_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lon00", min_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "latNxNy", max_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lonNxNy", max_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "dxKm", dx))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "dyKm", dy))
    return 0;

  
  if (_debug)
  {
    cerr << "dy: " << dy << endl;
    cerr << "dx : " << dx << endl;
    cerr << "Latitude range : " << min_lat << " to " << max_lat << endl;
    cerr << "Longitude range : " << min_lon << " to " << max_lon << endl;
  }

  
  MdvxPjg Projection;
  
  double x_distKm;
  double y_distKm;
  double theta;
  Projection.latlon2RTheta( c_lat, c_lon, c_lat, min_lon, x_distKm, theta );
  Projection.latlon2RTheta( c_lat, c_lon, min_lat, c_lon, y_distKm, theta );
  cerr << "x distKm = " << x_distKm << endl;
  cerr << "y distKm = " << y_distKm << endl;
  

  // OK - now ready to read the data itself.

  float missing_value;
  float *data = _extractFloatVariableArray(nc_file_id, _fieldName,
					   nx * ny * nz,
					   "_FillValue", missing_value,
					   "_scale");
  if (data == 0)
    return 0;
  
  //Get the data time.

  int *time_list = _extractIntVariableArray(nc_file_id, "valtime",
					    num_times);
  if (time_list == 0)
    return 0;
  
  time_t data_time = (time_t)time_list[0];
  
  // Print out some data statistics if we are in debug mode

  if (_debug)
  {
    // Print the min, max, perecnt good.

    int first=1;
    double min=0, max=0;
    int  numGood = 0;
    for (int i=0; i < nx*ny*nz; i++)
    {
      if (data[i] != missing_value)
      {
	numGood++;
	if (first)
	{
	  first=0;
	  min = data[i];
	  max=data[i];
	}
	else
	{
	  if (min > data[i]) min = data[i];
	  if (max < data[i]) max = data[i];
	}
      }
    }

    if (first)
    {
      cerr << "All data are missing." << endl;
    }
    else
    {
      cerr << "Data run from " << min << " to " << max << endl;
      int pg = int(100.0*double(numGood)/double(nx*ny*nz));
      cerr << numGood << " of " << nx*ny*nz;
      cerr << " were good (" << pg << "%)" << endl;
    }
  }

  // Create the field header for the field

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.forecast_time =  data_time; /* Not a forecast. */

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  fhdr.proj_param[0] = 15.0;
  fhdr.proj_param[1] = 45.0;

  fhdr.proj_origin_lat = c_lat;
  fhdr.proj_origin_lon = c_lon;

  fhdr.grid_dx =  dx;
  fhdr.grid_dy =  dy;
  fhdr.grid_dz =  0.0;

  fhdr.grid_minx =  -(x_distKm);
  fhdr.grid_miny =  -(y_distKm);
  fhdr.grid_minz =  0.0;

  fhdr.bad_data_value = missing_value;
  fhdr.missing_data_value = missing_value;

  string units_string = _extractStringVariableAtt(nc_file_id,
						  _fieldName,
						  "units");
  
  string fieldNameLong = _extractStringVariableAtt(nc_file_id,
						   _fieldName,
						   "long_name");
  
  sprintf(fhdr.field_name_long,"%s", fieldNameLong.c_str());
  sprintf(fhdr.field_name,"%s", _fieldName.c_str());
  sprintf(fhdr.units,"%s", units_string.c_str());

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
	
  // Create the vlevel header for the field

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);     

  vhdr.level[0] = 0.0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // Update the vlevel type in the field header

  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_UNKNOWN;
  
  // Create the field

  MdvxField *field = new MdvxField(fhdr, vhdr, data);
  
  // Reclaim memory

  free(data);

  return field;
}
