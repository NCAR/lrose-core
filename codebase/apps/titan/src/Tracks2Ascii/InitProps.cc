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
///////////////////////////////////////////////////////////////
// InitProps.cc
//
// InitProps object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#include "InitProps.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
#include <rapmath/math_macros.h>
using namespace std;

//////////////
// Constructor

InitProps::InitProps(const string &prog_name, const Params &params) :
  Entity(prog_name, params)
  
{

  // print labels
  
  fprintf(stdout, "#labels: %s",
	 "N_simple_tracks,"
	 "Complex num,Simple num,"
	 "Year,Month,Day,Hour,Min,Sec,"
	 "Range(km),"
	 "X(km),Y(km),"
	 "Area(km2),Vol(km3),Mass(ktons),Precip_flux(m3/s)"
	 "dA/dt(km2/hr)"
	 "dV/dt(km3/hr)"
	 "dM/dt(ktons/hr)"
	 "dP/dt(m3/s/hr)");

  fprintf(stdout, "\n");

  _nScansAlloc = _params.initial_props_nscans;
  _iprops = (init_props_t *)
    umalloc(_nScansAlloc * sizeof(init_props_t));
  
}

/////////////
// destructor

InitProps::~InitProps()

{

  ufree(_iprops);

}

////////////////
// comps_init()
//

int InitProps::comps_init(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
  
{

  // clear array

  _nScansFound = 0;
  memset(_iprops, 0, _nScansAlloc * sizeof(init_props_t));
  
  // suppress compiler warnings
  s_handle = NULL;
  t_handle = NULL;
  return (0);

}

////////////
// compute()
//

int InitProps::compute(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle)

{

  // check if we have gone beyond the initial period

  complex_track_params_t *ct_params = t_handle->complex_params;
  track_file_entry_t *entry = t_handle->entry;
  int scanNum =  entry->scan_num - ct_params->start_scan;
  if (scanNum > _nScansAlloc - 1) {
    return (-1);
  }
  _nScansFound = MAX(_nScansFound, scanNum + 1);

  storm_file_global_props_t *gprops =
    s_handle->gprops + t_handle->entry->storm_num;
  
  // Decide if this entry is inside box if applicable

  if (_params.use_box_limits) {
    if (gprops->proj_area_centroid_x < _params.box.min_x ||
	gprops->proj_area_centroid_x > _params.box.max_x ||
	gprops->proj_area_centroid_y < _params.box.min_y ||
	gprops->proj_area_centroid_y > _params.box.max_y) {
      return (0);
    }
  } // if (_params.use_box_limits)

  // get storm props

  double radarx = s_handle->scan->grid.sensor_x;
  double radary = s_handle->scan->grid.sensor_y;
  double stormx = gprops->proj_area_centroid_x;
  double stormy = gprops->proj_area_centroid_y;
  double dx = stormx - radarx;
  double dy = stormy - radary;
  double range = sqrt(dx * dx + dy * dy);

  // load up initial props array
  
  init_props_t *prop = _iprops + scanNum;
  
  prop->time = entry->time;
  prop->scan_num = scanNum;
  prop->nparts++;
  prop->x += stormx;
  prop->y += stormy;
  prop->range += range;
  prop->area += gprops->proj_area;
  prop->volume += gprops->volume;
  prop->mass += gprops->mass;
  prop->precip_flux += gprops->precip_flux;
  
  return (0);

}

////////////////
// comps_done()
//

int InitProps::comps_done(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
  
{

  // check that we have data at all scans

  if (_nScansFound < _nScansAlloc) {
    return (-1);
  }
  for (int i = 0; i < _nScansFound; i++) {
    if (_iprops[i].nparts < 1) {
      return (-1);
    }
  }

  // compute mean props at each time

  init_props_t *prop = _iprops;
  for (int i = 0; i < _nScansFound; i++, prop++) {
    if (prop->nparts > 1) {
      double nn = (double) prop->nparts;
      prop->x /= nn;
      prop->y /= nn;
      prop->range /= nn;
      prop->area /= nn;
      prop->volume /= nn;
      prop->mass /= nn;
      prop->precip_flux /= nn;
    }
  } // i

  // compute temporal means

  date_time_t start_time;
  start_time.unix_time = _iprops[0].time;
  uconvert_from_utime(&start_time);
  
  double range, xx, yy, area, volume, mass, precip_flux;

  _computeMean(&_iprops->range, &range);
  _computeMean(&_iprops->x, &xx);
  _computeMean(&_iprops->y, &yy);
  _computeMean(&_iprops->area, &area);
  _computeMean(&_iprops->volume, &volume);
  _computeMean(&_iprops->mass, &mass);
  _computeMean(&_iprops->precip_flux, &precip_flux);

  // compute temporal trends

  double da_dt, dv_dt, dm_dt, dp_dt;

  _computeTrend(&_iprops->time, &_iprops->area, &da_dt);
  _computeTrend(&_iprops->time, &_iprops->volume, &dv_dt);
  _computeTrend(&_iprops->time, &_iprops->mass, &dm_dt);
  _computeTrend(&_iprops->time, &_iprops->precip_flux, &dp_dt);

  // printout

  fprintf(stdout, "%3d %4d %4d ",
	  (int) t_handle->complex_params->n_simple_tracks,
	  (int) t_handle->complex_params->complex_track_num,
	  (int) t_handle->simple_params->simple_track_num);
  
  fprintf(stdout,
	  "%.4d %.2d %.2d %.2d %.2d %.2d",
	  start_time.year,
	  start_time.month,
	  start_time.day,
	  start_time.hour,
	  start_time.min,
	  start_time.sec);

  fprintf(stdout, " %10g", range);
  fprintf(stdout, " %10g", _iprops[0].x);
  fprintf(stdout, " %10g", _iprops[0].y);
  fprintf(stdout, " %10g", area);
  fprintf(stdout, " %10g", volume);
  fprintf(stdout, " %10g", mass);
  fprintf(stdout, " %10g", precip_flux);
  fprintf(stdout, " %10g", da_dt);
  fprintf(stdout, " %10g", dv_dt);
  fprintf(stdout, " %10g", dm_dt);
  fprintf(stdout, " %10g", dp_dt);

  fprintf(stdout, "\n");

  // suppress compiler warnings
  s_handle = NULL;

  return (0);

}



///////////////////////////////////////
// _computeTrend()
//
// computes the trend for a variable
//

void InitProps::_computeTrend(time_t *time0,
			      double *variable0,
			      double *trend_p)

{

  time_t *ttime = time0;
  double *variable = variable0;
  double trend;
  
  double x, y;
  double n = 0.0;
  double sum_x = 0.0, sum_y = 0.0;
  double sum_xx = 0.0, sum_xy = 0.0;
  double start_time;

  start_time = (double) *time0;

  for (int iscan = 0; iscan < _nScansFound; iscan++) {
    
    y = *variable;
    x = (double) *ttime - start_time;

    n += 1.0;
    sum_x += x;
    sum_y += y;
    sum_xx += x * x;
    sum_xy += x * y;
    
    ttime = (time_t *)
      ((char *) ttime + sizeof(init_props_t));

    variable = (double *)
      ((char *) variable + sizeof(init_props_t));

    
  } // iscan //

  // rate of change units per hour
  
  trend = ((n * sum_xy - sum_x * sum_y) /
	   (n * sum_xx - sum_x * sum_x)) * 3600.0;

  *trend_p = trend;

  return;

}

///////////////////////////////////////
// _computeMean()
//
// computes the mean for a variable
//

void InitProps::_computeMean(double *variable0,
			     double *mean_p)

{
  
  double *variable = variable0;
  double mean;
  
  double x;
  double n = 0.0;
  double sum_x = 0.0;

  for (int iscan = 0; iscan < _nScansFound; iscan++) {
    
    x = *variable;
    n += 1.0;
    sum_x += x;
    
    variable = (double *)
      ((char *) variable + sizeof(init_props_t));
    
  } // iscan //
  
  // mean
  
  if (n > 0.0) {
    mean = sum_x / n;
  } else {
    mean = 0.0;
  }

  *mean_p = mean;

  return;

}



