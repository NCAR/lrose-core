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
//   $Id: ConusFieldHandler.cc,v 1.10 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ConusFieldHandler: Class for handling fields from a netCDF file that is
 *                    in the format used for the NSSL mosaics produced over
 *                    the CONUS.
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

#include "ConusFieldHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

ConusFieldHandler::ConusFieldHandler(const string field_name,
				     const bool debug_flag,
				     const bool old_format) :
  FieldHandler(field_name, debug_flag, old_format)
{
}


/**********************************************************************
 * Destructor
 */

ConusFieldHandler::~ConusFieldHandler(void)
{
}
  

/**********************************************************************
 * extractField() - Extract the field from the given netCDF file.
 */

MdvxField *ConusFieldHandler::extractField(const int nc_file_id)
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
  int nz  = _extractDimensionValue(nc_file_id, _fieldName + "_levels");

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  
  // Extract the list of vertical levels from the file

  vector< level_info_t > levels = _extractLevels(nc_file_id,
						 _fieldName + "Levels", nz);
  if (levels.size() != (size_t)nz)
    return 0;
  
  // Extract the dx/dy values from the netCDF file.

  float dx, dy;
  float min_lat, min_lon;
  float max_lat, max_lon;
  
  if (!_extractFloatGlobalAtt(nc_file_id, "dx", dx))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "dy", dy))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "yMin", min_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "xMin", min_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "yMax", max_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "xMax", max_lon))
    return 0;
  
  if (_debug)
  {
    cerr << "Latitude step : " << dy << endl;
    cerr << "Longitude step : " << dx << endl;
    cerr << "Latitude range : " << min_lat << " to " << max_lat << endl;
    cerr << "Longitude range : " << min_lon << " to " << max_lon << endl;
  }

  // OK - now ready to read the data itself.

  float missing_value;
  float *data = _extractFloatVariableArray(nc_file_id, _fieldName,
					   nx * ny * nz,
					   "_FillValue", missing_value,
					   "_scale");
  if (data == 0)
    return 0;
  
  // Get the data time.

//  int *time_list = _extractIntVariableArray(nc_file_id, "valtime",
//					    num_times);
//  if (time_list == 0)
//    return 0;
//  
//  time_t data_time = (time_t)time_list[0];
  
  char datatime_string[64];

  if (nc_get_att_text(nc_file_id, NC_GLOBAL,
		      "Data_record_time", datatime_string) != NC_NOERR)
  {
    cerr << "Failed to get data time" << endl;
    delete [] data;
    return 0;
  }

  int len = strlen("20030519.2030");
  datatime_string[len]=char(0);
  if (_debug)
    cerr << "Data time is " << datatime_string << endl;

  int year, month, day, hour, min;
  sscanf(datatime_string,"%4d%2d%2d.%2d%2d",
	 &year, &month, &day, &hour, &min);
  
  DateTime data_time_obj(year, month, day, hour, min);
  time_t data_time = data_time_obj.utime();
  
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

  fhdr.proj_type = Mdvx::PROJ_LATLON;

  fhdr.proj_origin_lat = (min_lat + max_lat) / 2.0;
  fhdr.proj_origin_lon = (min_lon + max_lon) / 2.0;

  fhdr.grid_dx =  dx;
  fhdr.grid_dy =  dy;
  fhdr.grid_dz =  0.0;

  fhdr.grid_minx =  min_lon;
  fhdr.grid_miny =  min_lat;
  fhdr.grid_minz =  levels[0].level;

  fhdr.bad_data_value = missing_value;
  fhdr.missing_data_value = missing_value;

  string units_string = _extractStringVariableAtt(nc_file_id,
						  _fieldName,
						  "units");
  
  sprintf(fhdr.field_name_long,"%s", _fieldName.c_str());
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

  for (int i = 0; i < nz; ++i)
  {
    vhdr.level[i] = levels[i].level;
    vhdr.type[i] = levels[i].level_type;
  }

  // Update the vlevel type in the field header

  fhdr.vlevel_type = vhdr.type[0];
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_UNKNOWN;
  
  // Create the field

  MdvxField *field = new MdvxField(fhdr, vhdr, data);
  
  // Reclaim memory

  free(data);

  return field;
}
  


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _extractLevels() - Extract the vertical levels from the netCDF file.
 */

vector< ConusFieldHandler::level_info_t > ConusFieldHandler::_extractLevels(const int nc_file_id,
									    const string &level_field_name,
									    const int nz)
{
  vector< level_info_t > levels;

  // Extract the level strings from the netCDF file.

  vector< string > level_strings =
    _extractStringVariableArray(nc_file_id, level_field_name.c_str(),
				"charsPerLevel", nz);
  
  if ((int)(level_strings.size()) != nz)
    return levels;
  
  // Decode what we have.

  for (int i = 0; i < nz; ++i)
  {
    string level_string = level_strings[i];

    level_info_t level_info;
    double level;
    
    if (sscanf(level_string.c_str(), "FH %lf", &level) == 1)
    {
      level_info.level_type = Mdvx::VERT_TYPE_Z;
      level_info.level = level / 1000.0;
    }
    else if (STRequal(level_string.c_str(), "SFC"))
    {
      level_info.level_type = Mdvx::VERT_TYPE_SURFACE;
      level_info.level = 0.0;
    }
    else
    {
      cerr << "Failed to decode height string <" << level_string
	   << ">" << endl;
      return levels;
    }

    levels.push_back(level_info);
  }

  return levels;
}


/**********************************************************************
 * ExtractFieldWDSS2() - Extract the field from the given netCDF file.
 */

MdvxField *ConusFieldHandler::extractFieldWDSS2(const int nc_file_id)
{

  int nx  = _extractDimensionValue(nc_file_id, "Lon");
  int ny  = _extractDimensionValue(nc_file_id, "Lat");
  int nz  = _extractDimensionValue(nc_file_id, "Ht", false);

  if( nz == -1 )
      nz = 1;

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;

  float dx, dy;
  float min_lat, min_lon;
  float max_lat, max_lon;
  
  if (!_extractFloatGlobalAtt(nc_file_id, "LonGridSpacing", dx))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "LatGridSpacing", dy))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "Longitude", min_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "Latitude", max_lat))
    return 0;
  min_lat = max_lat - (ny * dy);
  max_lon = min_lon + (nx * dx);
  
  if (_debug)
  {
    cerr << "Latitude step : " << dy << endl;
    cerr << "Longitude step : " << dx << endl;
    cerr << "Latitude range : " << min_lat << " to " << max_lat << endl;
    cerr << "Longitude range : " << min_lon << " to " << max_lon << endl;
  }

  // OK - now ready to read the data itself.

  float missing_value;

  PMU_auto_register("Retrieving data");

  float *data = _extractFloatVariableArray(nc_file_id, _fieldName,
					   nx * ny * nz,
					   "MissingData", missing_value,
					   "Scale");

  float *ht_data = _extractFloatVariableArray(nc_file_id, "Height",
					      nz, false);
  if (data == 0)
    return 0;


  // Extract the list of vertical levels from the file

//  vector< level_info_t > levels = _extractLevels2(nc_file_id,
//						 "Height", nz);

  vector< level_info_t > levels;
  
  for (int i = 0; i < nz; ++i)
  {

    level_info_t level_info;
    
    level_info.level_type = Mdvx::VERT_TYPE_Z;

    if (ht_data)
	level_info.level = ht_data[i] / 1000.0;
    else
	level_info.level = 1.0;
    
    levels.push_back(level_info);
  }


  // Get the data time.

//  char datatime_string[64];
  long time;
  
  if (int status = nc_get_att_long(nc_file_id, NC_GLOBAL,
		      "Time", &time) != NC_NOERR)
  {
    cerr << "status = " << status << endl;
    cerr << "Failed to get data time" << endl;
    delete [] data;
    return 0;
  }

  if (_debug)
    cerr << "Data time is " << time << endl;

  time_t data_time =  static_cast<time_t>(time);
    
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


// reorder data to start at SW corner
// WDSS-II format starts at NW corner

  float *_data = new float[nx*ny];
  PMU_force_register("Rearranging data");

  for (int z = 0; z < nz; z++)
  {
      int it=0;

      if (_debug)
	  cerr << "Rearranging z level: " << z << endl;

      for(int  y = 1; y <= ny; y++) 
      {
	  int start_grid_pt = (z * nx * ny) +( nx * (ny - y));
	  for (int i=0; i < nx; i++)
	  {
	      _data[it] = data[start_grid_pt + i];
	      it++;
	  }
      }
      
      for(int n = 0; n < ( nx * ny); n++)
      {
	  data[n + (z * nx * ny)] = _data[n];
      }
  }
  delete [] _data;
      
	  
  // Create the field header for the field

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.forecast_time =  data_time; /* Not a forecast. */

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
  fhdr.grid_minz =  levels[0].level;

  fhdr.bad_data_value = missing_value;
  fhdr.missing_data_value = missing_value;

  string units_string = _extractStringVariableAtt(nc_file_id,
						  _fieldName,
						  "Units");
  
  sprintf(fhdr.field_name_long,"%s", _fieldName.c_str());
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

  for (int i = 0; i < nz; ++i)
  {
    vhdr.level[i] = levels[i].level;
    vhdr.type[i] = levels[i].level_type;
  }

  // Update the vlevel type in the field header

  fhdr.vlevel_type = vhdr.type[0];
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_UNKNOWN;
  
  // Create the field

  if (_debug)
      cerr << "Create mdv field\n";

  PMU_force_register("create mdv field");
  MdvxField *field = new MdvxField(fhdr, vhdr, data);
  
  // Reclaim memory
  free(data);
  free(ht_data);

  return field;
}
