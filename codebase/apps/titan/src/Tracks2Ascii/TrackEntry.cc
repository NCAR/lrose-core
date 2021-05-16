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
// TrackEntry.cc
//
// TrackEntry object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#include "TrackEntry.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
#include <toolsa/pjg.h>
#include <rapmath/math_macros.h>
#include <Mdv/Mdvx.hh>
using namespace std;

//////////////
// Constructor

TrackEntry::TrackEntry(const string &prog_name, const Params &params) :
  Entity(prog_name, params)

{

  _labelsPrinted = false;

}

/////////////
// destructor

TrackEntry::~TrackEntry()

{


}

//////////////////////////////
// print the column labels

void TrackEntry::_printLabels(storm_file_handle_t *s_handle)
  
{

  if (_labelsPrinted) {
    // previously done
    return;
  }
  
  // print labels

  if (_params.refl_histogram_only) {

    fprintf(stdout,
	    "#labels: "
	    "NSimpleTracks,"
	    "ComplexNum,"
	    "SimpleNum,"
	    "Year,"
	    "Month,"
	    "Day,"
	    "Hour,"
	    "Min,"
	    "Sec,"
            "dBZThreshold,"
            "ReflCentroidLat(deg),"
	    "ReflCentroidLon(deg),");
    
    if (_params.refl_histogram_vol) {
      fprintf(stdout, "Volume(km3),");
    } else {
      fprintf(stdout, "PrecipArea(km2),");
    }
    
    fprintf(stdout,
	    "nBins,"
	    "DbzInBin1,"
	    "PercentInBin1,"
	    "DbzInBin2,"
	    "PercentInBin2,"
	    "......");

  } else {

    fprintf(stdout,
	    "#labels: "
	    "NSimpleTracks,"
	    "ComplexNum,"
	    "SimpleNum,"
	    "Year,"
	    "Month,"
	    "Day,"
	    "Hour,"
	    "Min,"
	    "Sec,"
	    "dBZThreshold,"
	    "RemainingDuration(hrs),"
	    "VolCentroidLat(deg),"
	    "VolCentroidLon(deg),"
	    "VolCentroidX(km),"
	    "VolCentroidY(km),"
	    "VolCentroidZ(km),"
	    "ReflCentroidLat(deg),"
	    "ReflCentroidLon(deg),"
	    "ReflCentroidX(km),"
	    "ReflCentroidY(km),"
	    "ReflCentroidZ(km),"
	    "PrecipCentroidLat(deg),"
	    "PrecipCentroidLon(deg),"
	    "PrecipCentroidX(km),"
	    "PrecipCentroidY(km),"
	    "PrecipArea(km2),"
	    "PrecipOrientation(degT),"
	    "PrecipMajorRadius(km),"
	    "PrecipMinorRadius(km),"
	    "PrecipFlux(m3/s),"
	    "PrecipRate(mm/hr),"
	    "EnvelopeCentroidLat(deg),"
	    "EnvelopeCentroidLon(deg),"
	    "EnvelopeCentroidX(km),"
	    "EnvelopeCentroidY(km),"
	    "EnvelopeArea(km2),"
	    "EnvelopeOrientation(degT),"
	    "EnvelopeMajorRadius(km),"
	    "EnvelopeMinorRadius(km),"
	    "Top(km),"
	    "Base(km),"
	    "Volume(km3),"
	    "MeanArea(km2),"
	    "Mass(ktons),"
	    "TiltAngle(deg),"
	    "TiltOrientation(degT),"
	    "MaxDBZ(dBZ),"
	    "MeanDbz(dBZ),"
	    "MaxDBZGradient(dBZ/km),"
	    "MeanDbzGradient(dBZ/km),"
	    "HtOfMaxDBZ(km),"
	    "Vorticity(/s),"
	    "VilFromMaxZ(kg/m2),"
	    "U(km/hr),"
	    "V(km/hr),"
	    "Dtop/Dt(km/hr),"
	    "Dvolume/Dt(km3/hr),"
	    "DprecipFlux/Dt(m3/s2),"
	    "Dmass/Dt(ktons/hr),"
	    "DdBzMax/Dt(dBZ/hr),"
	    "Speed(km/hr),"
	    "Dirn(DegT),"
	    "%%Vol>40dBZ,"
	    "%%Area>40dBZ,"
	    "%%Vol>50dBZ,"
	    "%%Area>50dBZ,"
	    "%%Vol>60dBZ,"
	    "%%Area>60dBZ,"
	    "%%Vol>70dBZ,"
	    "%%Area>70dBZ,"
            "HailFOKRCat0-4,"
            "HailWaldvogelProb,"
            "HailMassAloft(ktons),"
            "HailVihm(kg/m2),"
	    "ReflCentroidRange(km),"
	    "ReflCentroidAzimuth(deg),"
            "parents,"
            "children");

    if (_params.print_level_properties) {

      // set field list
      
      vector<string> flds;
      flds.push_back("VolCentroidX");
      flds.push_back("VolCentroidY");
      flds.push_back("ReflCentroidX");
      flds.push_back("ReflCentroidY");
      flds.push_back("Area");
      flds.push_back("MaxDBZ");
      flds.push_back("MeanDBZ");
      flds.push_back("Mass");
      flds.push_back("MeanVel");
      flds.push_back("SdevVel");
      flds.push_back("Vorticity");

      // loop through all grid heights
      
      titan_grid_t grid = s_handle->scan->grid;
      for (int iz = 0; iz < grid.nz; iz++) {
        double htKm = grid.minz + iz * grid.dz;
        char htKmLabel[128];
        snprintf(htKmLabel, 128, "Ht_%.2f_", htKm);
        for (size_t ii = 0; ii < flds.size(); ii++) {
          fprintf(stdout, ",%s%s", htKmLabel, flds[ii].c_str());
        }
      } // iz

    } // if (_params.print_level_properties) 

    if (_params.print_polygons) {
      fprintf(stdout, ",nPolySidesPolygonRays*%d", N_POLY_SIDES);
    }

  }
  
  fprintf(stdout, "\n");

  _labelsPrinted = true;
  
}

////////////////
// comps_init()
//

int TrackEntry::comps_init(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
  
{
  // suppress compiler warnings
  s_handle = NULL;
  t_handle = NULL;
  return (0);
}

////////////
// compute()
//

int TrackEntry::compute(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle)

{

  storm_file_global_props_t *gprops =
    s_handle->gprops + t_handle->entry->storm_num;
  track_file_entry_t *entry = t_handle->entry;

  // read in the storm props

  if (RfReadStormProps(s_handle, entry->storm_num,
		       "TrackEntry::compute") != R_SUCCESS) {
    return -1;
  }

  // print labels if not already done
  
  _printLabels(s_handle);

  // Decide if this entry is within a 'sampling region in time'. 
  // A 'sampling region' starts at regular sampling intervals, and
  // lasts for the duration of a scan interval.

  date_time_t entry_time;
  entry_time.unix_time = entry->time;
  uconvert_from_utime(&entry_time);
  
  bool accept_entry;

  if (_params.sample_interval > 0) {
    
    // test if time coincides with sampling time
    
    int start_sampling_region =
      ((entry_time.unix_time / _params.sample_interval) *
       _params.sample_interval);
    
    int time_diff = entry_time.unix_time - start_sampling_region;
    
    if (time_diff > _params.scan_interval * 1.1) {
      accept_entry = false;
    } else {
      accept_entry = true;
    }

  } else {

    // accept all entries
    accept_entry = true;

  }

  // Decide if this entry is within a 'sampling region in space'. 
  // The 'sampling region' is a box defined by the box limits

  if (_params.use_box_limits) {
    
    if (gprops->refl_centroid_x < _params.box.min_x ||
	gprops->refl_centroid_x > _params.box.max_x ||
	gprops->refl_centroid_y < _params.box.min_y ||
	gprops->refl_centroid_y > _params.box.max_y) {
      
      accept_entry = false;
      
    }
    
  } // if (_params.use_box_limits)

  if (!accept_entry) {
    return 0;
  }

  if (_params.refl_histogram_only) {
    _computeHist(s_handle, t_handle, entry_time);
  } else {
    _computeAll(s_handle, t_handle, entry_time);
  }

  return 0;

}

/////////////////////
// compute all props
//

void TrackEntry::_computeAll(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     date_time_t &entry_time)

{

  storm_file_params_t *sparams = &s_handle->header->params;
  storm_file_global_props_t *gprops =
    s_handle->gprops + t_handle->entry->storm_num;
  track_file_entry_t *entry = t_handle->entry;
  track_file_forecast_props_t *fprops = &entry->dval_dt;
  simple_track_params_t *stparams = t_handle->simple_params;
  storm_file_scan_header_t *scan = s_handle->scan;
  titan_grid_t grid = scan->grid;
  double isLatLon = false;
  if (grid.proj_type == TITAN_PROJ_LATLON) {
    isLatLon = true;
  }

  // compute life expectancy
  
  date_time_t last_time;
  last_time.unix_time =
    t_handle->simple_params->last_descendant_end_time;
  uconvert_from_utime(&last_time);
  int this_scan = entry->scan_num;
  int last_scan = t_handle->simple_params->last_descendant_end_scan;

  double rem_duration;
  if (this_scan == last_scan) {
    rem_duration = 0.0;
  } else {
    // adjust for the fact that the times are taken at mid-scan,
    // by adding the duration of one-half of a scan
    rem_duration = ((((double) last_time.unix_time -
		      (double) entry_time.unix_time) +
		     _params.scan_interval / 2) / 3600.0);
  }

  // create a converted gprops, for computing ellipses and
  // lat/lon as required

  storm_file_global_props_t gpropsConvert = *gprops;
  RfStormGpropsEllipses2Km(s_handle->scan, &gpropsConvert);
  RfStormGpropsXY2LatLon(s_handle->scan, &gpropsConvert);

  // load props for printing
    
  double vol_centroid_lon = gpropsConvert.vol_centroid_x;
  double vol_centroid_lat = gpropsConvert.vol_centroid_y;
  double vol_centroid_x = gprops->vol_centroid_x;
  double vol_centroid_y = gprops->vol_centroid_y;
  double vol_centroid_z = gprops->vol_centroid_z;
    
  double refl_centroid_lon = gpropsConvert.refl_centroid_x;
  double refl_centroid_lat = gpropsConvert.refl_centroid_y;
  double refl_centroid_x = gprops->refl_centroid_x;
  double refl_centroid_y = gprops->refl_centroid_y;
  double refl_centroid_z = gprops->refl_centroid_z;

  double precip_area_centroid_lon = gpropsConvert.precip_area_centroid_x;
  double precip_area_centroid_lat = gpropsConvert.precip_area_centroid_y;
  double precip_area_centroid_x = gprops->precip_area_centroid_x;
  double precip_area_centroid_y = gprops->precip_area_centroid_y;
  double precip_area = gprops->precip_area;
  double precip_area_orientation = gpropsConvert.precip_area_orientation;
  double precip_area_major_radius = gpropsConvert.precip_area_major_radius;
  double precip_area_minor_radius = gpropsConvert.precip_area_minor_radius;
    
  double proj_area_centroid_lon = gpropsConvert.proj_area_centroid_x;
  double proj_area_centroid_lat = gpropsConvert.proj_area_centroid_y;
  double proj_area_centroid_x = gprops->proj_area_centroid_x;
  double proj_area_centroid_y = gprops->proj_area_centroid_y;
  double proj_area = gprops->proj_area;
  double proj_area_orientation = gpropsConvert.proj_area_orientation;
  double proj_area_major_radius = gpropsConvert.proj_area_major_radius;
  double proj_area_minor_radius = gpropsConvert.proj_area_minor_radius;

  double reflCentroidRangeKm, reflCentroidAzimuthDeg;
  PJGLatLon2RTheta(grid.proj_origin_lat, grid.proj_origin_lon,
                   refl_centroid_lat, refl_centroid_lon,
                   &reflCentroidRangeKm, &reflCentroidAzimuthDeg);
    
  double top = gprops->top;
  double base = gprops->base;
  double volume = gprops->volume;
  double area_mean = gprops->area_mean;
  double precip_flux = gprops->precip_flux;
  double precip_rate;
  if (precip_area == 0.0) {
    precip_rate = 0.0;
  } else {
    precip_rate = (precip_flux / precip_area) * 3.6;
  }

  double mass = gprops->mass;

  double tilt_angle = gprops->tilt_angle;
  double tilt_dirn = gprops->tilt_dirn;

  double dbz_max = gprops->dbz_max;
  double dbz_mean = gprops->dbz_mean;
  double dbz_max_gradient = gprops->dbz_max_gradient;
  double dbz_mean_gradient = gprops->dbz_mean_gradient;
  double ht_of_dbz_max = gprops->ht_of_dbz_max;

  double vorticity = gprops->vorticity;
  double vil_from_maxz = gprops->vil_from_maxz;

  double dx_dt = fprops->proj_area_centroid_x;
  double dy_dt = fprops->proj_area_centroid_y;
  double dtop_dt = fprops->top;
  double dvolume_dt = fprops->volume;
  double dprecip_flux_dt = fprops->precip_flux;
  double dmass_dt = fprops->mass;
  double ddbz_max_dt = fprops->dbz_max;

  double speed, dirn;
  double uu, vv;

  _compute_speed_and_dirn(isLatLon,
                          proj_area_centroid_x, proj_area_centroid_y,
                          dx_dt, dy_dt,
                          speed, dirn, uu, vv);
  
  double percent_vol_gt_40, percent_area_gt_40;
  double percent_vol_gt_50, percent_area_gt_50;
  double percent_vol_gt_60, percent_area_gt_60;
  double percent_vol_gt_70, percent_area_gt_70;
    
  _compute_percent_gt_refl(s_handle, gprops,
			   40.0, percent_vol_gt_40, percent_area_gt_40);
  _compute_percent_gt_refl(s_handle, gprops,
			   50.0, percent_vol_gt_50, percent_area_gt_50);
  _compute_percent_gt_refl(s_handle, gprops,
			   60.0, percent_vol_gt_60, percent_area_gt_60);
  _compute_percent_gt_refl(s_handle, gprops,
			   70.0, percent_vol_gt_70, percent_area_gt_70);

  fprintf(stdout, "%d %d %d ",
	  (int) t_handle->complex_params->n_simple_tracks,
	  (int) t_handle->complex_params->complex_track_num,
	  (int) t_handle->simple_params->simple_track_num);

  fprintf(stdout,
	  "%.4d %.2d %.2d %.2d %.2d %.2d",
	  entry_time.year,
	  entry_time.month,
	  entry_time.day,
	  entry_time.hour,
	  entry_time.min,
	  entry_time.sec);

  fprintf(stdout, " %g", sparams->low_dbz_threshold);
  fprintf(stdout, " %g", rem_duration);

  fprintf(stdout, " %g", vol_centroid_lat);
  fprintf(stdout, " %g", vol_centroid_lon);
  fprintf(stdout, " %g", vol_centroid_x);
  fprintf(stdout, " %g", vol_centroid_y);
  fprintf(stdout, " %g", vol_centroid_z);

  fprintf(stdout, " %g", refl_centroid_lat);
  fprintf(stdout, " %g", refl_centroid_lon);
  fprintf(stdout, " %g", refl_centroid_x);
  fprintf(stdout, " %g", refl_centroid_y);
  fprintf(stdout, " %g", refl_centroid_z);

  fprintf(stdout, " %g", precip_area_centroid_lat);
  fprintf(stdout, " %g", precip_area_centroid_lon);
  fprintf(stdout, " %g", precip_area_centroid_x);
  fprintf(stdout, " %g", precip_area_centroid_y);
  fprintf(stdout, " %g", precip_area);
  fprintf(stdout, " %g", precip_area_orientation);
  fprintf(stdout, " %g", precip_area_major_radius);
  fprintf(stdout, " %g", precip_area_minor_radius);
  fprintf(stdout, " %g", precip_flux);
  fprintf(stdout, " %g", precip_rate);
    
  fprintf(stdout, " %g", proj_area_centroid_lat);
  fprintf(stdout, " %g", proj_area_centroid_lon);
  fprintf(stdout, " %g", proj_area_centroid_x);
  fprintf(stdout, " %g", proj_area_centroid_y);
  fprintf(stdout, " %g", proj_area);
  fprintf(stdout, " %g", proj_area_orientation);
  fprintf(stdout, " %g", proj_area_major_radius);
  fprintf(stdout, " %g", proj_area_minor_radius);
    
  fprintf(stdout, " %g", top);
  fprintf(stdout, " %g", base);
  fprintf(stdout, " %g", volume);
  fprintf(stdout, " %g", area_mean);
  fprintf(stdout, " %g", mass);

  fprintf(stdout, " %g", tilt_angle);
  fprintf(stdout, " %g", tilt_dirn);

  fprintf(stdout, " %g", dbz_max);
  fprintf(stdout, " %g", dbz_mean);
  fprintf(stdout, " %g", dbz_max_gradient);
  fprintf(stdout, " %g", dbz_mean_gradient);
  fprintf(stdout, " %g", ht_of_dbz_max);

  fprintf(stdout, " %g", vorticity);
  fprintf(stdout, " %g", vil_from_maxz);

  fprintf(stdout, " %g", uu);
  fprintf(stdout, " %g", vv);
  fprintf(stdout, " %g", dtop_dt);
  fprintf(stdout, " %g", dvolume_dt);
  fprintf(stdout, " %g", dprecip_flux_dt);
  fprintf(stdout, " %g", dmass_dt);
  fprintf(stdout, " %g", ddbz_max_dt);

  fprintf(stdout, " %g", speed);
  fprintf(stdout, " %g", dirn);

  fprintf(stdout, " %g", percent_vol_gt_40);
  fprintf(stdout, " %g", percent_area_gt_40);
  fprintf(stdout, " %g", percent_vol_gt_50);
  fprintf(stdout, " %g", percent_area_gt_50);
  fprintf(stdout, " %g", percent_vol_gt_60);
  fprintf(stdout, " %g", percent_area_gt_60);
  fprintf(stdout, " %g", percent_vol_gt_70);
  fprintf(stdout, " %g", percent_area_gt_70);
  fprintf(stdout, " %d", gprops->add_on.hail_metrics.FOKRcategory);
  fprintf(stdout, " %g", gprops->add_on.hail_metrics.waldvogelProbability);
  fprintf(stdout, " %g", gprops->add_on.hail_metrics.hailMassAloft);
  fprintf(stdout, " %g", gprops->add_on.hail_metrics.vihm);

  fprintf(stdout, " %g", reflCentroidRangeKm);
  fprintf(stdout, " %g", reflCentroidAzimuthDeg);

  // parent simple track numbers

  fprintf(stdout, " ");
  if (stparams->nparents > 0) {
    // comma delimited  list of parents
    for (int i = 0; i < stparams->nparents; i++) {
      fprintf(stdout, "%d", stparams->parent[i]);
      if (i < stparams->nparents - 1) {
        fprintf(stdout, ",");
      }
    }
  } else {
    // no parents
    fprintf(stdout, "-");
  }
    
  // child simple track numbers

  fprintf(stdout, " ");
  if (stparams->nchildren > 0) {
    // comma delimited  list of children
    for (int i = 0; i < stparams->nchildren; i++) {
      fprintf(stdout, "%d", stparams->child[i]);
      if (i < stparams->nchildren - 1) {
        fprintf(stdout, ",");
      }
    }
  } else {
    // no children
    fprintf(stdout, "-");
  }
    
  if (_params.print_level_properties) {
    int baseLayer = gprops->base_layer;
    int topLayer = baseLayer + gprops->n_layers - 1;
    // loop through all grid heights
    for (int iz = 0; iz < grid.nz; iz++) {
      if (iz >= baseLayer && iz <= topLayer) {
        storm_file_layer_props_t &layer = s_handle->layer[iz - baseLayer];
        fprintf(stdout, " %g", layer.vol_centroid_x);
        fprintf(stdout, " %g", layer.vol_centroid_y);
        fprintf(stdout, " %g", layer.refl_centroid_x);
        fprintf(stdout, " %g", layer.refl_centroid_y);
        fprintf(stdout, " %g", layer.area);
        fprintf(stdout, " %g", layer.dbz_max);
        fprintf(stdout, " %g", layer.dbz_mean);
        fprintf(stdout, " %g", layer.mass);
        fprintf(stdout, " %g", layer.rad_vel_mean);
        fprintf(stdout, " %g", layer.rad_vel_sd);
        fprintf(stdout, " %g", layer.vorticity);
      } else {
        // layer has no data
        for (int jj = 0; jj < 11; jj++) {
          fprintf(stdout, " -9999.0");
        }
      }
    } // iz
  } // if (_params.print_level_properties) 

  if (_params.print_polygons) {
    fprintf(stdout, " %d", (int) sparams->n_poly_sides);
    for (int i = 0; i < sparams->n_poly_sides; i++) {
      fprintf(stdout, " %3.1f", gprops->proj_area_polygon[i]);
    }
  }
  
  fprintf(stdout, "\n");

}

/////////////////////////////////////////
// compute speed and dirn for all tracks

void TrackEntry::_compute_speed_and_dirn(bool isLatLon,
					 double storm_x,
					 double storm_y,
					 double dx_dt,
					 double dy_dt,
					 double &speed,
					 double &dirn,
                                         double &uu,
                                         double &vv)
  
{
  
  double dx_km = 0.0, dy_km = 0.0;
  
  // compute movement over 1 hour
  
  if (isLatLon) {
    
    double start_lon = storm_x;
    double start_lat = storm_y;
    
    double end_lon = start_lon + dx_dt;
    double end_lat = start_lat + dy_dt;

    PJGLatLon2DxDy(start_lat, start_lon, end_lat, end_lon,
		   &dx_km, &dy_km);

  } else {
    
    dx_km = dx_dt;
    dy_km = dy_dt;
    
  } /* if (latlon) */
  
  speed = sqrt(dx_km * dx_km + dy_km * dy_km);
  
  dirn = 0.0;
  if (dx_km != 0.0 || dy_km != 0.0) {
    dirn = atan2(dx_km, dy_km) * RAD_TO_DEG;
  }
  
  if (dirn < 0.0) {
    dirn += 360.0;
  }

  uu = dx_km;
  vv = dy_km;
    
}

/////////////////////
// compute histograms
//

void TrackEntry::_computeHist(storm_file_handle_t *s_handle,
			      track_file_handle_t *t_handle,
			      date_time_t &entry_time)

{

  storm_file_params_t *sparams = &s_handle->header->params;
  storm_file_global_props_t *gprops =
    s_handle->gprops + t_handle->entry->storm_num;

  // create a converted gprops, for computing ellipses and
  // lat/lon as required
  
  storm_file_global_props_t gpropsConvert = *gprops;
  RfStormGpropsEllipses2Km(s_handle->scan, &gpropsConvert);
  RfStormGpropsXY2LatLon(s_handle->scan, &gpropsConvert);
  
  // load props for printing
    
  double refl_centroid_lon = gpropsConvert.refl_centroid_x;
  double refl_centroid_lat = gpropsConvert.refl_centroid_y;

  fprintf(stdout, "%d %d %d ",
	  (int) t_handle->complex_params->n_simple_tracks,
	  (int) t_handle->complex_params->complex_track_num,
	  (int) t_handle->simple_params->simple_track_num);

  fprintf(stdout,
	  "%.4d %.2d %.2d %.2d %.2d %.2d",
	  entry_time.year,
	  entry_time.month,
	  entry_time.day,
	  entry_time.hour,
	  entry_time.min,
	  entry_time.sec);
  
  fprintf(stdout, " %g", sparams->low_dbz_threshold);
  fprintf(stdout, " %g", refl_centroid_lat);
  fprintf(stdout, " %g", refl_centroid_lon);

  if (_params.refl_histogram_vol) {
    fprintf(stdout, " %g", gprops->volume);
  } else {
    fprintf(stdout, " %g", gprops->precip_area);
  }
    
  fprintf(stdout, " %d", gprops->n_dbz_intervals);

  for (int interval = 0;
       interval < gprops->n_dbz_intervals; interval++) {
      
    storm_file_dbz_hist_t *hist = s_handle->hist + interval;
    double dbz = (sparams->low_dbz_threshold +
		  (double) interval * sparams->dbz_hist_interval);
      
    double percent_vol = hist->percent_volume;
    double percent_area = hist->percent_area;

    fprintf(stdout, " %g", dbz);
    if (_params.refl_histogram_vol) {
      fprintf(stdout, " %g", percent_vol);
    } else {
      fprintf(stdout, " %g", percent_area);
    }
    
  }

  fprintf(stdout, "\n");

}

////////////////
// comps_done()
//

int TrackEntry::comps_done(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
  
{
  // suppress compiler warnings
  s_handle = NULL;
  t_handle = NULL;
  return (0);
}

void TrackEntry::_compute_percent_gt_refl(storm_file_handle_t *s_handle,
					  storm_file_global_props_t *gprops,
					  double dbz_threshold,
					  double &percent_vol_gt,
					  double &percent_area_gt)

{
  
  percent_vol_gt = 0.0;
  percent_area_gt = 0.0;
  
  storm_file_params_t *sparams = &s_handle->header->params;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int interval = 0;
	 interval < gprops->n_dbz_intervals; interval++) {
      
      storm_file_dbz_hist_t *hist = s_handle->hist + interval;
      double dbz = (sparams->low_dbz_threshold +
		    (double) interval * sparams->dbz_hist_interval);
      
      double percent_vol = hist->percent_volume;
      double percent_area = hist->percent_area;
      
      cerr << "dbz, %vol, %area: "
	   << dbz << ", "
	   << percent_vol << ", "
	   << percent_area << endl;
    }
  } // if (_params.debug >= Params::DEBUG_VERBOSE)

  for (int interval = 0;
       interval < gprops->n_dbz_intervals; interval++) {
    
    storm_file_dbz_hist_t *hist = s_handle->hist + interval;
    double dbz = (sparams->low_dbz_threshold +
		  (double) interval * sparams->dbz_hist_interval);
    
    double percent_vol = hist->percent_volume;
    double percent_area = hist->percent_area;
    
    if (dbz >= dbz_threshold) {
      percent_vol_gt += percent_vol;
      percent_area_gt += percent_area;
    } else if ((dbz + sparams->dbz_hist_interval) >= dbz_threshold) {
      double fraction = (dbz_threshold - dbz) / sparams->dbz_hist_interval;
      percent_vol_gt += percent_vol * fraction;
      percent_area_gt += percent_area * fraction;
    }
    
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "percent_vol_gt: " << percent_vol_gt << endl;
    cerr << "percent_area_gt: " << percent_area_gt << endl;
  }

}

