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
//   $Date: 2016/03/04 02:22:15 $
//   $Id: CiwsFieldHandler.cc,v 1.5 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CiwsFieldHandler: Base class for classes that handle fields in the NSSL
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
#include <toolsa/mem.h>
#include "CiwsFieldHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

CiwsFieldHandler::CiwsFieldHandler(const string field_name,
				   const bool debug_flag) :
  FieldHandler(field_name, debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

CiwsFieldHandler::~CiwsFieldHandler(void)
{
}
  

/**********************************************************************
 * extractField() - Extract the field from the given netCDF file.
 */

MdvxField *CiwsFieldHandler::extractField(const int nc_file_id)
{
  // Get the number of data times in the file.  Currently, we can
  // only handle files with a single data time.

  if (_extractDimensionValue(nc_file_id, "n_valtimes") != 1)
  {
    cerr << "Multiple times in the file - skipping file." << endl;
    return 0;
  }

  // Get the dimension lengths.

  int nx  = _extractDimensionValue(nc_file_id, "x");
  int ny  = _extractDimensionValue(nc_file_id, "y");
  int nz  = _extractDimensionValue(nc_file_id, "levels");

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  
  // Extract the list of vertical levels from the file

  vector< double > levels = _extractLevels(nc_file_id, nz);
  if (levels.size() != (size_t)nz)
    return 0;
  
  // Extract the dx/dy values from the netCDF file.

  double dx, dy;
  double min_lat, min_lon;
  double max_lat, max_lon;
  
  if (!_extractDxDy(nc_file_id, nx, ny, dx, dy,
		    min_lat, min_lon, max_lat, max_lon))
    return 0;
  
  if (_debug)
  {
    cerr << "Latitude step : " << dy << endl;
    cerr << "Longitude step : " << dx << endl;
    cerr << "Latitude range : " << min_lat << " to " << max_lat << endl;
    cerr << "Longitude range : " << min_lon << " to " << max_lon << endl;
  }

  // OK - now ready to read the data itself.

  float missingVal;
  float *rr = _extractFloatVariableArray(nc_file_id, "rr", nx * ny * nz,
					 "_FillValue", missingVal,
					 "Record_scale");
  if (rr == 0)
    return 0;
  
  // Get the data time.

  char datatime_string[64];

  if (nc_get_att_text(nc_file_id, NC_GLOBAL,
		      "Data_record_time", datatime_string) != NC_NOERR)
  {
    cerr << "Failed to get data time" << endl;
    free(rr);
    return 0;
  }

  int len = strlen("20030519.2030");
  datatime_string[len]=char(0);
  if (_debug)
    cerr << "Data time is " << datatime_string << endl;

  int year, month, day, hour, min;
  sscanf(datatime_string,"%4d%2d%2d.%2d%2d",
	 &year, &month, &day, &hour, &min);
  
  DateTime data_time(year, month, day, hour, min);

  // Print out some data statistics if we are in debug mode

  if (_debug)
  {
    // Print the min, max, perecnt good.

    int first=1;
    double min=0, max=0;
    int  numGood = 0;
    for (int i=0; i < nx*ny*nz; i++)
    {
      if (rr[i] != missingVal)
      {
	numGood++;
	if (first)
	{
	  first=0;
	  min = rr[i];
	  max=rr[i];
	}
	else
	{
	  if (min > rr[i]) min = rr[i];
	  if (max < rr[i]) max = rr[i];
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

  fhdr.forecast_time =  data_time.utime(); /* Not a forecast. */

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.proj_type = Mdvx::PROJ_LATLON;

  fhdr.proj_origin_lat = (min_lat + max_lat) / 2.0;
  fhdr.proj_origin_lon = (min_lon + max_lon) / 2.0;

  fhdr.grid_dx =  dx;
  fhdr.grid_dy =  dy;
  fhdr.grid_dz =  0.0;

  fhdr.grid_minx =  min_lon;
  fhdr.grid_miny =  min_lat;
  fhdr.grid_minz =  levels[0];

  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;

  sprintf(fhdr.field_name_long,"%s", "DBZ");
  sprintf(fhdr.field_name,"%s", "DBZ");
  sprintf(fhdr.units,"%s", "dbz");

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
	
  // Create the vlevel header for the field

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);     

  for (int i = 0; i < nz; ++i)
  {
    vhdr.level[i] = levels[i];
    vhdr.type[i] = Mdvx::VERT_TYPE_Z;
  }

  // Create the field

  MdvxField *field = new MdvxField(fhdr, vhdr, rr);
  
  // Reclaim memory

  free(rr);

  return field;
}

/**********************************************************************
 * extractFieldWDSS2() - Extract the field from the given netCDF file.
 */

MdvxField *CiwsFieldHandler::extractFieldWDSS2(const int nc_file_id)
{
    cout << "Does nothing at this time" << endl;
    return 0;
    
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _getMinMax() - Get the minimum and maximum values from the given
 *                data array.
 */

void CiwsFieldHandler::_getMinMax(const float *data, const int num_values,
				  double &min_value, double &max_value)
{
  min_value = data[0];
  max_value = data[0];

  for (int i = 1; i < num_values; ++i)
  {
    if (data[i] > max_value) max_value = data[i];
    if (data[i] < min_value) min_value = data[i];
  }
}


/**********************************************************************
 * _extractLevels() - Extract the vertical levels from the netCDF file.
 */

vector< double > CiwsFieldHandler::_extractLevels(const int nc_file_id,
						  const int nz) const
{
  vector< double > levels;

  // Extract the level strings from the netCDF file.

  vector< string > level_strings =
    _extractStringVariableArray(nc_file_id, "rrLevels", "charsPerLevel", nz);
  
  if ((int)(level_strings.size()) != nz)
    return levels;
  
  // Decode what we have.

  for (int i = 0; i < nz; ++i)
  {
    string level_string = level_strings[i];

    double level;
    
    if (1 != sscanf(level_string.c_str(), "FH %lf", &level))
    {
      cerr << "Failed to decode height string " << level_string << endl;
      return levels;
    }

    levels.push_back(level / 1000.0);
  }

  return levels;
}


/**********************************************************************
 * _extractDxDy() - Extract the dx/dy values from the netCDF file.  Also
 *                  return the min/max lat/lon values since these are
 *                  needed, too.
 */

bool CiwsFieldHandler::_extractDxDy(const int nc_file_id,
				    const int nx, const int ny,
				    double &dx, double &dy,
				    double &min_lat, double &min_lon,
				    double &max_lat, double &max_lon) const
{
  // Get x,y ranges. This is done in a somewhat cumbersome way - the
  // rrLat and rrLon variables contain all of the lat/lon values for
  // the data points.  We assume that these are evenly spaced in the
  // file so we find the min and max lat/lon values in the file and
  // then compute dx/dy from these values.

  // Read all of the latitude values in the file

  float *lat_values = _extractFloatVariableArray(nc_file_id,
						 "rrLat", nx * ny);
  if (lat_values == 0)
    return false;
  
  _getMinMax(lat_values, nx * ny,
	     min_lat, max_lat);

  delete [] lat_values;
  
  // Read all of the longitude values in the file

  float *lon_values = _extractFloatVariableArray(nc_file_id,
						 "rrLon", nx * ny);
  if (lon_values == 0)
    return false;
  
  _getMinMax(lon_values, nx * ny,
	     min_lon, max_lon);

  delete [] lon_values;

  // Calculate dx/dy

  dy = (max_lat - min_lat) / double(ny);
  dx = (max_lon - min_lon) / double(nx);

  return true;
}
