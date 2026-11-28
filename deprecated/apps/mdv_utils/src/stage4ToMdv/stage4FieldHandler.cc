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
 
/*********************************************************************
 * stage4FieldHandler: Class for handling fields from a netCDF file that is
 *                    in the format used for the stage4 data interpolated to
 *                    the model grid.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2008
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <netcdf.h>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <euclid/Pjg.hh>

#include "stage4FieldHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

stage4FieldHandler::stage4FieldHandler(const string field_name,
				       const bool debug_flag) :
  FieldHandler(field_name, debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

stage4FieldHandler::~stage4FieldHandler(void)
{
}

/**********************************************************************
 * extractData() - Extract the field from the given netCDF file.
 */

float *stage4FieldHandler::extractData(const int nc_file_id)
{
  int number_of_times = _extractDimensionValue(nc_file_id,
					       "Time");
  int nx  = _extractDimensionValue(nc_file_id,
				   "west_east");
  int ny  = _extractDimensionValue(nc_file_id,
				   "south_north");
  int nz = 1;

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  
  float missing_value;
  float *data = _extractFloatVariableArray(nc_file_id, _fieldName,
					   nx * ny * number_of_times,
					   "_FillValue", missing_value,
					   "_scale");

  return data;
}


/**********************************************************************
 * extractTimes() - Extract the field from the given netCDF file.
 */

vector <time_t> stage4FieldHandler::extractTimes(const int nc_file_id)
{
  int number_of_times = _extractDimensionValue(nc_file_id,
					       "Time");
  
  vector< string > Times = 
    _extractStringVariableArray(nc_file_id, 
				"Times",
				"DateStrLen",
				number_of_times);
  vector< time_t > time;
  
  for(int time_list = 0; time_list < (int)Times.size(); time_list++)
  {
    int year, month, day, hour, min;
    year = atoi(Times[time_list].substr(0,4).c_str());
    month = atoi(Times[time_list].substr(5,2).c_str());
    day = atoi(Times[time_list].substr(8,2).c_str());
    hour = atoi(Times[time_list].substr(11,2).c_str());
    min = atoi(Times[time_list].substr(14,2).c_str());
    
    DateTime data_time_obj(year, month, day, hour, min);
    
    time_t data_time = data_time_obj.utime();
    time.push_back(data_time);
  }
  

  return time;
}


/**********************************************************************
 * extractField() - Extract the field from the given netCDF file.
 */

MdvxField *stage4FieldHandler::extractField(const int nc_file_id, const time_t data_time, const int extract_time, const float *data)
{

  int nx  = _extractDimensionValue(nc_file_id,
				   "west_east");
  int ny  = _extractDimensionValue(nc_file_id,
				   "south_north");
  
  int nz = 1;

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  
  float dx, dy;
  float min_lat, min_lon;
  float max_lat, max_lon;
  float origin_lat, origin_lon;
  float lat1, lat2;

  double minx, miny;

  if (!_extractFloatGlobalAtt(nc_file_id, "grid_spacing", dx))
    return 0;
  dx = dx / 1000.0;

  if (!_extractFloatGlobalAtt(nc_file_id, "grid_spacing", dy))
    return 0;
  dy = dy / 1000.0;

  if (!_extractFloatGlobalAtt(nc_file_id, "lat_SW_corner", min_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lon_SW_corner", min_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lat_NE_corner", max_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "lon_NE_corner", max_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "central_latitude", origin_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "central_longitude", origin_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "truelat1", lat1))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "truelat2", lat2))
    return 0;
  
  if (_debug)
  {
    cerr << "nx = " << nx << endl;
    cerr << "ny = " << ny <<endl;
    cerr << "Latitude step : " << dy << endl;
    cerr << "Longitude step : " << dx << endl;
    cerr << "Latitude range : " << min_lat << " to " << max_lat << endl;
    cerr << "Longitude range : " << min_lon << " to " << max_lon << endl;
    cerr << "origin_lat = " << origin_lat << endl;
    cerr << "origin_lon = " << origin_lon << endl;
    cerr << "lat1 = " << lat1 << endl;
    cerr << "lat2 = " << lat2 << endl;
  }

  if(_debug)
  {
    cerr << "origin_lat = " << origin_lat << endl;
    cerr << "origin_lon = " << origin_lon << endl;
  }
 
  Pjg *MP = new Pjg();
  
  MP->initLc2(origin_lat, origin_lon,
	      lat1, lat2);
  
  MP->latlon2xy(min_lat, min_lon, minx, miny);
  delete MP;
  
  float missing_value;
  
// Create the field header for the field

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.forecast_time =  data_time; /* Not a forecast. */

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;

  fhdr.proj_origin_lat = origin_lat;
  fhdr.proj_origin_lon = origin_lon;
 
  fhdr.proj_param[0] = lat1;
  fhdr.proj_param[1] = lat2;

  fhdr.grid_dx =  dx;
  fhdr.grid_dy =  dy;
  fhdr.grid_dz =  0.0;

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;

  if(_debug)
  {
    cerr << "minx = " << fhdr.grid_minx << endl;
    cerr << "miny = " << fhdr.grid_miny << endl;
  }

  fhdr.grid_minz =  1.0;

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
    vhdr.level[i] = i;
    vhdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
  }

  float *this_time_data = new float[nx * ny];
  int x = 0;

  for(int i = nx * ny * extract_time; i < nx * ny * ( extract_time + 1); i++)
  {
    this_time_data[x] = data[i];
    x++;
  }
  
  // Create the field

  MdvxField *field = new MdvxField(fhdr, vhdr, this_time_data);
  
  return field;
}

