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
 * WrfVILFieldHandler: Class for handling fields from a netCDF file that is
 *                    in the format used for the WrfVIL data interpolated to
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
#include <toolsa/toolsa_macros.h>
#include <euclid/Pjg.hh>

#include "WrfVILFieldHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

WrfVILFieldHandler::WrfVILFieldHandler(bool user_defined_bad_missing,
                                       const float bad_missing_val,
				       bool redefine_bad_missing,
                                       const float new_bad_missing_val,
				       const bool scale_data,
				       const float multiplicative_factor,
                                       const string field_name,
				       const bool debug_flag) :
  FieldHandler(user_defined_bad_missing, bad_missing_val,
	       redefine_bad_missing, new_bad_missing_val,
	       scale_data,  multiplicative_factor,
	       field_name, debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

WrfVILFieldHandler::~WrfVILFieldHandler(void)
{
}

/**********************************************************************
 * extractData() - Extract the field from the given netCDF file.
 */

float *WrfVILFieldHandler::extractData(const int nc_file_id)
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

vector <time_t> WrfVILFieldHandler::extractTimes(const int nc_file_id)
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

MdvxField *WrfVILFieldHandler::extractField(const int nc_file_id, const time_t data_time, const int extract_time, const float *data)
{

  int nx  = _extractDimensionValue(nc_file_id,
				   "west_east");
  int ny  = _extractDimensionValue(nc_file_id,
				   "south_north");
  
  int nz = 1;

  if (nx < 0 || ny < 0 || nz < 0)
    return 0;
  
  float dx, dy;
  float origin_lat, origin_lon;
  float lat1, lat2;

  double minx, miny;
    
  string gen_time;

  if (!_extractStringGlobalAtt(nc_file_id, "START_DATE", gen_time))
    return 0;
  cout << "gen_time =" << gen_time << endl;

  if (!_extractFloatGlobalAtt(nc_file_id, "DX", dx))
    return 0;
  dx = dx / 1000.0;

  if (!_extractFloatGlobalAtt(nc_file_id, "DY", dy))
    return 0;
  dy = dy / 1000.0;
  if (!_extractFloatGlobalAtt(nc_file_id, "CEN_LAT", origin_lat))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "CEN_LON", origin_lon))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "TRUELAT1", lat1))
    return 0;
  if (!_extractFloatGlobalAtt(nc_file_id, "TRUELAT2", lat2))
    return 0;
  
  if (_debug)
  {
    cerr << "nx = " << nx << endl;
    cerr << "ny = " << ny <<endl;
    cerr << "Latitude step : " << dy << endl;
    cerr << "Longitude step : " << dx << endl;
    cerr << "origin_lat = " << origin_lat << endl;
    cerr << "origin_lon = " << origin_lon << endl;
    cerr << "lat1 = " << lat1 << endl;
    cerr << "lat2 = " << lat2 << endl;
  }
  
  minx = - ( nx * .5 * dx );
  miny = - ( ny * .5 * dx );
  
  float missing_value = -9999.0;
  
  // Create the field header for the field

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);


  int year, month, day, hour, min;
  year = atoi(gen_time.substr(0,4).c_str());
  month = atoi(gen_time.substr(5,2).c_str());
  day = atoi(gen_time.substr(8,2).c_str());
  hour = atoi(gen_time.substr(11,2).c_str());
  min = atoi(gen_time.substr(14,2).c_str());
    
  DateTime init_time_obj(year, month, day, hour, min);
    
  time_t init_data_time = init_time_obj.utime();

  fhdr.forecast_delta = data_time - init_data_time;
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

  if(_user_defined_bad_missing)
   {
      fhdr.bad_data_value = _bad_missing_val;
      fhdr.missing_data_value = _bad_missing_val;
   }
  else
   {
      fhdr.bad_data_value = missing_value;
      fhdr.missing_data_value = missing_value;
   }

   

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
    if (_scale_data && this_time_data[x] != fhdr.missing_data_value)
      this_time_data[x] =  this_time_data[x] * _multiplicative_factor;
    if (this_time_data[x] == fhdr.missing_data_value && _redefine_bad_missing)
      this_time_data[x] = _new_bad_missing_val;
    x++;
  }
  
  //
  // reset field header bad/missing if necessary
  //
  if (_redefine_bad_missing)
    {
      fhdr.bad_data_value = _new_bad_missing_val;
      fhdr.missing_data_value = _new_bad_missing_val;
    }

  // Create the field

  MdvxField *field = new MdvxField(fhdr, vhdr, this_time_data);
  delete this_time_data;

  return field;
}

