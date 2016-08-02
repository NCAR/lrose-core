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

/*********************************************************************
 * FieldHandler: Base class for classes that handle fields in the stage4
 *               precip file.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2008
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <netcdf.h>

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
					 const string &dim_name)
{
  int dim_id;

  int status = nc_inq_dimid(nc_file_id, dim_name.c_str(), &dim_id);
  if (status != NC_NOERR)
  {
    cerr << "Failed to get dimension ID for " << dim_name << endl;
    return -1;
  }
 
  size_t dim_value;
 
  status = nc_inq_dimlen(nc_file_id, dim_id, &dim_value);
  if (status != NC_NOERR)
  {
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
 * _extractIntGlobalAtt() - Extract the given float global attribute
 *                            from the given netCDF file.
 *
 * Returns true on success, false on failure.
 */

bool FieldHandler::_extractIntGlobalAtt(const int nc_file_id,
					const string &att_name,
					int &att_value)
{
  if (nc_get_att_int(nc_file_id, NC_GLOBAL,
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
						const int array_size) const
{
  float missing_value;
  
  return _extractFloatVariableArray(nc_file_id, var_name, array_size,
				    "", missing_value, "");
}


float *FieldHandler::_extractFloatVariableArray(const int nc_file_id,
						const string &var_name,
						const int array_size,
						const string &missing_value_att_name,
						float &missing_value,
						const string scale_att_name) const
{
  // Get access to the given variable

  int var_id;

  if (nc_inq_varid(nc_file_id, var_name.c_str(), &var_id) != NC_NOERR)
  {
    cerr << "Failed to get ID for variable " << var_name << endl;
    return 0;
  }

  if(_debug)
      cout << "var_name = " << var_name.c_str() << endl;
  
  // Read the data

  float *data = new float[array_size];

  if (nc_get_var_float(nc_file_id, var_id, data) != NC_NOERR)
  {
    cerr << "Failed to read variable " << var_name << endl;
    delete [] data;
    return 0;
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

  char *string_buffer = new char[string_size];
    
  for (int i = 0; i < num_strings; ++i)
  {
    for (int j = 0; j < string_size; ++j)
    {
      string_buffer[j] = text_buffer[i * string_size + j];
    }

    string_buffer[string_size] = '\0';
    
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
