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
////////////////////////////////////////////////////////////////////////
// TitanSpdb.cc
//
// TitanSpdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// Routines for handling TITAN spdb data
//
////////////////////////////////////////////////////////////////

#include <titan/TitanSpdb.hh>
using namespace std;

/////////////////////////////////////////////////////////////////////////
// load header

void TitanSpdb::loadHeader(tstorm_spdb_header_t &header,
			   const storm_file_params_t &sparams,
			   const titan_grid_t &grid,
			   time_t dtime,
			   int n_entries)
     
{

  header.time = dtime;
  header.grid = grid;
  header.low_dbz_threshold = sparams.low_dbz_threshold;
  header.n_entries = n_entries;
  header.n_poly_sides = sparams.n_poly_sides;
  if (header.grid.proj_type == TITAN_PROJ_FLAT) {
    header.poly_start_az =
      sparams.poly_start_az + grid.proj_params.flat.rotation;
  } else {
    header.poly_start_az = sparams.poly_start_az;
  }
  header.poly_delta_az = sparams.poly_delta_az;

}  

/////////////////////////////////////////////////////////////////////////
// load entry

void TitanSpdb::loadEntry(const tstorm_spdb_header_t &header,
			  const track_file_entry_t &file_entry,
			  const storm_file_global_props_t &gprops,
			  const track_file_forecast_props_t &fprops,
			  const titan_grid_comps_t &comps,
			  tstorm_spdb_entry_t &entry)
  
{
  
  // lat / lon
  
  double longitude, latitude;
  TITAN_xy2latlon(&comps,
		  gprops.proj_area_centroid_x,
		  gprops.proj_area_centroid_y,
		  &latitude, &longitude);
  
  entry.longitude = longitude;
  entry.latitude = latitude;
  
  // speed
  
  entry.speed = fprops.smoothed_speed;
  
  // direction
  
  double direction = fmod(fprops.smoothed_direction + comps.rotation, 360.0);
  if (direction < 0.0) {
    direction += 360.0;
  }
  entry.direction = direction;

  // track numbers
  
  entry.simple_track_num = file_entry.simple_track_num;
  entry.complex_track_num = file_entry.complex_track_num;

  // area

  entry.area = gprops.proj_area;
  entry.darea_dt = fprops.proj_area;

  // top
  
  entry.top = gprops.top;
  
  // ellipse
  
  double orientation;
  if (comps.proj_type == TITAN_PROJ_FLAT) {
    orientation = (fmod((double) gprops.proj_area_orientation, 360.0) +
		   comps.rotation);
  } else {
    orientation = fmod((double) gprops.proj_area_orientation, 360.0);
  }
  if (orientation < 0.0) {
    orientation += 360.0;
  }
  entry.ellipse_orientation = orientation;

  entry.ellipse_major_radius = gprops.proj_area_major_radius;
  entry.ellipse_minor_radius = gprops.proj_area_minor_radius;

  // polygon
  
  const fl32 *radial = gprops.proj_area_polygon;
  double max_length = 0.0;
  for (int i = 0; i < header.n_poly_sides; i++, radial++) {
    max_length = MAX(max_length, *radial);
  }
  double polygon_scale = max_length / 255.0;
  entry.polygon_scale = polygon_scale;

  radial = gprops.proj_area_polygon;
  ui08 *rp = entry.polygon_radials;
  for (int i = 0; i < header.n_poly_sides; i++, rp++, radial++) {
    *rp = (ui08) (*radial / polygon_scale + 0.5);
  }

  // valid forecast

  entry.forecast_valid = file_entry.forecast_valid;

  // dbz_max

  entry.dbz_max = (si08) (gprops.dbz_max + 0.5);

  // set intensity trend

  double mass = gprops.mass;
  double dmass_dt = fprops.mass;
  double norm_dmass_dt = dmass_dt / mass;
  
  if (norm_dmass_dt < -0.2) {
    entry.intensity_trend = -1;
  } else if (norm_dmass_dt > 0.5) {
    entry.intensity_trend = 1;
  } else {
    entry.intensity_trend = 0;
  }

  // set size trend

  double area = gprops.proj_area;
  double darea_dt = fprops.proj_area;
  double norm_darea_dt = darea_dt / area;
  
  if (norm_darea_dt < -0.2) {
    entry.size_trend = -1;
  } else if (norm_darea_dt > 0.5) {
    entry.size_trend = 1;
  } else {
    entry.size_trend = 0;
  }

}

