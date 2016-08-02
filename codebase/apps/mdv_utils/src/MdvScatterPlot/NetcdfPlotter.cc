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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: NetcdfPlotter.cc,v 1.3 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NetcdfPlotter: Class that creates scatter plots as netCDF files
 *                that can be read into other programs for creating
 *                the plots.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <Mdv/MdvxPjg.hh>
#include <toolsa/MemBuf.hh>

#include "NetcdfPlotter.hh"

using namespace std;


const string NetcdfPlotter::DIMENSION_NAME = "obs";

const string NetcdfPlotter::LAT_VAR_NAME = "lat";
const string NetcdfPlotter::LON_VAR_NAME = "lon";
const string NetcdfPlotter::TIME_VAR_NAME = "unix_time";

/*********************************************************************
 * Constructors
 */

NetcdfPlotter::NetcdfPlotter(const string &output_dir,
			     const string &output_ext,
			     const string &x_field_name,
			     const string &y_field_name,
			     const bool debug_flag) :
  Plotter(output_dir, output_ext, x_field_name, y_field_name, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

NetcdfPlotter::~NetcdfPlotter()
{
}


/*********************************************************************
 * createPlot() - Create the scatter plot for the given fields.
 *
 * Returns true on success, false on failure.
 */

bool NetcdfPlotter::createPlot(const DateTime &data_time,
			       const MdvxField &x_field,
			       const MdvxField &y_field)
{
  static const string method_name = "NetcdfPlotter::createPlot()";
  
  // Generate the output file path.  Note that _generateOutputPath
  // also creates the directories for us.

  Path output_path = _generateOutputPath(data_time);
  
  return createPlot(output_path.getPath(),
		    data_time, x_field, y_field);
}


bool NetcdfPlotter::createPlot(const string &output_path,
			       const DateTime &data_time,
			       const MdvxField &x_field,
			       const MdvxField &y_field)
{
  static const string method_name = "NetcdfPlotter::createPlot()";
  
  // Accumulate the new data for the file

  Mdvx::field_header_t x_field_hdr = x_field.getFieldHeader();
  Mdvx::field_header_t y_field_hdr = y_field.getFieldHeader();
  
  string x_field_name = _xFieldName;
  string y_field_name = _yFieldName;
  
  if (x_field_name == "")
    x_field_name = x_field_hdr.field_name;
  if (y_field_name == "")
    y_field_name = y_field_hdr.field_name;
  
  fl32 *x_field_data = (fl32 *)x_field.getVol();
  fl32 *y_field_data = (fl32 *)y_field.getVol();
  
  MdvxPjg proj(x_field_hdr);
  
  int data_size = x_field_hdr.nx * x_field_hdr.ny;
  
  float *lat_data = new float[data_size];
  float *lon_data = new float[data_size];
  int *time_data = new int[data_size];
  float *x_data = new float[data_size];
  float *y_data = new float[data_size];
  
  int var_index = 0;
  
  for (int x = 0; x < x_field_hdr.nx; ++x)
  {
    for (int y = 0; y < x_field_hdr.ny; ++y)
    {
      int field_index = y * x_field_hdr.nx + x;
      
      // Don't process missing data

      if (x_field_data[field_index] == x_field_hdr.missing_data_value ||
	  x_field_data[field_index] == x_field_hdr.bad_data_value)
	continue;
    
      if (y_field_data[field_index] == y_field_hdr.missing_data_value ||
	  y_field_data[field_index] == y_field_hdr.bad_data_value)
	continue;
    
      // Calculate the needed values

      double lat, lon;
      
      proj.xyIndex2latlon(x, y, lat, lon);
      
      // Save the data

      lat_data[var_index] = lat;
      lon_data[var_index] = lon;
      time_data[var_index] = data_time.utime();
      x_data[var_index] = x_field_data[field_index];
      y_data[var_index] = y_field_data[field_index];
      
      ++var_index;
      
    } /* endfor - y */
  } /* endfor - x */

  // Create an error object so that the netCDF library doesn't exit when an
  // error is encountered.  This object is not explicitly used in the below
  // code, but is used implicitly by the netCDF library.

  NcError nc_error(NcError::silent_nonfatal);

  // Get a pointer to the netCDF file.

  NcFile *nc_file;
  
  if ((nc_file = _getFile(output_path,
			  x_field_name,
			  y_field_name)) == 0)
    return false;
  
  if (!nc_file->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid netCDF file: " << output_path << endl;
    
    delete nc_file;
    
    return false;
  }
  
  // Get all of the pointers we need from the file

  NcDim *dimension = 0;
  NcVar *lat_var = 0;
  NcVar *lon_var = 0;
  NcVar *time_var = 0;
  NcVar *x_var = 0;
  NcVar *y_var = 0;
  
  if ((dimension = nc_file->get_dim(DIMENSION_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing dimension " << DIMENSION_NAME
	 << " in file " << output_path << endl;
   
    delete nc_file;
    
    return false;
  }
	
  if ((lat_var = nc_file->get_var(LAT_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing " << LAT_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return false;
  }

  if ((lon_var = nc_file->get_var(LON_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing " << LON_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return false;
  }

  if ((time_var = nc_file->get_var(TIME_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing " << TIME_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return false;
  }

  if ((x_var = nc_file->get_var(x_field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing " << x_field_name
	 << " variable in file " << output_path << endl;
      
    delete nc_file;
    
    return false;
  }

  if ((y_var = nc_file->get_var(y_field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing " << y_field_name
	 << " variable in file " << output_path << endl;
      
    delete nc_file;
    
    return false;
  }

  // Put the data in the appropriate fields

  int current_data_size = dimension->size();
  
  if (!_appendData(lat_var, LAT_VAR_NAME, current_data_size,
		   lat_data, var_index))
  {
    delete nc_file;
    
    return false;
  }
  
  if (!_appendData(lon_var, LON_VAR_NAME, current_data_size,
		   lon_data, var_index))
  {
    delete nc_file;
    
    return false;
  }
  
  if (!_appendData(time_var, TIME_VAR_NAME, current_data_size,
		   time_data, var_index))
  {
    delete nc_file;
    
    return false;
  }
  
  if (!_appendData(x_var, x_field_name, current_data_size,
		   x_data, var_index))
  {
    delete nc_file;
    
    return false;
  }
  
  if (!_appendData(y_var, y_field_name, current_data_size,
		   y_data, var_index))
  {
    delete nc_file;
    
    return false;
  }
  
  // Reclaim memory that's no longer needed

  delete [] lat_data;
  delete [] lon_data;
  delete [] time_data;
  delete [] x_data;
  delete [] y_data;
  
  // Write the netCDF file to disk.  This might be done automatically by the
  // destructor.

  if (!nc_file->sync())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file " << output_path << endl;
    
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _appendData() - Append the given data to the currently existing data
 *                 for the given variable.
 *
 * Returns true on success, false on failure.
 */

template< class T >
bool NetcdfPlotter::_appendData(NcVar *variable,
				const string &variable_name,
				const int current_data_size,
				const T *data,
				const int data_size) const
{
  static const string method_name = "NetcdfPlotter::_appendData()";
  
  MemBuf data_buffer;
  
  if (_debug)
  {
    cerr << "Appending data to " << variable_name << " variable:" << endl;
    cerr << "      Before add, have " << variable->num_vals() << " values" << endl;
    cerr << "      Adding " << data_size << " values" << endl;
  }
  
  // Put the new data into the variable

  if (!variable->set_cur(current_data_size))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error moving to end of data in " << variable_name
	 << " variable" << endl;
    
    return false;
  }
  
  if (!variable->put(data, data_size))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending data to " << variable_name << " variable" << endl;
    
    return false;
  }

  if (_debug)
  {
    cerr << "      After add, have " << variable->num_vals() << " values" << endl;
  }
  
  return true;
}


/*********************************************************************
 * _getExistingFile() - Open the indicated existing netCDF file and return
 *                      a pointer to it.
 *
 * Returns a pointer to the netCDF file on success, 0 on failure.
 *
 * The caller must delete the file pointer once they are finished with the
 * file.
 */

NcFile *NetcdfPlotter::_getExistingFile(const string &output_path,
					const string &x_field_name,
					const string &y_field_name) const
{
  static const string method_name = "NetcdfPlotter::_getExistingFile()";
  
  NcFile *nc_file = new NcFile(output_path.c_str(), NcFile::Write);
  
  return nc_file;
}


/*********************************************************************
 * _getFile() - Open the indicated netCDF file and return a pointer to it.
 *              If the file currently exists on disk, we will open that
 *              file, check for any inconsistencies compared to what we
 *              expect to find in the file, and return the pointer.  If
 *              the file doesn't currently exist, we will create the file
 *              and all of the needed dimensions and variables.
 *
 * Returns a pointer to the netCDF file on success, 0 on failure.
 *
 * The caller must delete the file pointer once they are finished with the
 * file.
 */

NcFile *NetcdfPlotter::_getFile(const string &output_path,
				const string &x_field_name,
				const string &y_field_name) const
{
  static const string method_name = "NetcdfPlotter::_getFile()";
  
  // See if the desired file currently exists.  If it does, return the
  // existing file.  If it doesn't, create a new file.

  struct stat file_stat;
  
  if (stat(output_path.c_str(), &file_stat) == 0)
    return _getExistingFile(output_path, x_field_name, y_field_name);

  return _getNewFile(output_path, x_field_name, y_field_name);
}


/*********************************************************************
 * _getNewFile() - Create the indicated netCDF file and return a pointer
 *                 to it.  Create the all of the needed dimensions and
 *                 variables.
 *
 * Returns a pointer to the netCDF file on success, 0 on failure.
 *
 * The caller must delete the file pointer once they are finished with the
 * file.
 */

NcFile *NetcdfPlotter::_getNewFile(const string &output_path,
				   const string &x_field_name,
				   const string &y_field_name) const
{
  static const string method_name = "NetcdfPlotter::_getNewFile()";
  
  // Open the netCDF file.  If a file already exists, we need to add
  // this data to the existing file because we're in accumulation mode.
  // If the file doesn't exist, we need to create a new one.

  NcFile *nc_file = new NcFile(output_path.c_str(), NcFile::Replace);
  
  // If this is a new file, add our dimension and variables.  If the file
  // currently exists, check for compatibility with our current fields and
  // get pointers to the dimension and variables.

  NcDim *dimension = 0;
  NcVar *variable = 0;
  
  if ((dimension = nc_file->add_dim(DIMENSION_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating dimension " << DIMENSION_NAME
	 << " in file " << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  if ((variable =
       nc_file->add_var(LAT_VAR_NAME.c_str(), ncFloat, dimension)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating " << LAT_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  if ((variable =
       nc_file->add_var(LON_VAR_NAME.c_str(), ncFloat, dimension)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating " << LON_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  if ((variable =
       nc_file->add_var(TIME_VAR_NAME.c_str(), ncInt, dimension)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating " << TIME_VAR_NAME << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  if ((variable =
       nc_file->add_var(x_field_name.c_str(), ncFloat, dimension)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating " << x_field_name << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  if ((variable =
       nc_file->add_var(y_field_name.c_str(), ncFloat, dimension)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating " << y_field_name << " variable in file "
	 << output_path << endl;
      
    delete nc_file;
    
    return 0;
  }
    
  return nc_file;
}
