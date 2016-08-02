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
/////////////////////////////////////////////////////////////
// TrForecast.hh
//
// TrForecast class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef TrForecast_HH
#define TrForecast_HH

#include "Worker.hh"
#include "TrStorm.hh"
#include <vector>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

////////////////////////////////
// TrForecast

class TrForecast : public Worker {
  
public:

  // typedefs

  typedef struct {
    double x, y;
  } xypair_t;

  // constructor

  TrForecast(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~TrForecast();

  // compute forecast
  // Returns 0 on success, -1 on failure

  int compute(TitanStormFile &sfile,
	      TrStorm &storm, int storm_num);
  
  // compute speed and dirn for all storms

  void compute_speed_and_dirn(TitanStormFile &sfile,
			      vector<TrStorm*> &storms);

protected:
  
private:

  bool _projIsLatLon;
  bool _useFieldTracker;

  void _smooth_spatial_forecasts(TitanStormFile &sfile,
				 vector<TrStorm*> &storms);

  int _get_vol_monotonicity(TrTrack &track);

  double _get_refl_max_ht(const storm_file_layer_props_t *layer,
			  int n_layers,
			  double min_z,
			  double delta_z,
			  double refl);

  void _get_trend(TrTrack &track,
		  double &val,
		  double forecast_scale,
		  int zero_growth,
		  int zero_decay);

  void _set_area_change_ratio(TrTrack &track);

  double _get_vol_percentile(const storm_file_dbz_hist_t *hist,
			     int n_dbz_intervals,
			     double low_dbz_threshold,
			     double dbz_hist_interval,
			     double percentile);

  int _regression_forecast(TitanStormFile &sfile,
			   int storm_num,
			   TrStorm &storm,
			   double forecast_scale);
  
  void _trend_forecast(TrStorm &storm,
		       int three_d_analysis,
		       double forecast_scale);

  void _copy_props(TrTrack &track);

  void _get_extreme_coords(vector<TrStorm*> &storms,
			   xypair_t *max_coord,
			   xypair_t *min_coord);
  
  void _load_distance_array(const TitanStormFile &sfile,
			    vector<TrStorm*> &storms,
			    fl32 **distance_array);
  
  void _smooth_motion(vector<TrStorm*> &storms,
		      size_t istorm,
		      TrTrack &this_track,
		      fl32 **distance_array,
		      bool ignore_this_storm,
		      bool history_override,
		      bool erratic_only);
  
  void _limit_rel_speed_change(const TitanStormFile &sfile,
			       TrTrack &this_track);
  
  void _load_speed_and_dirn(TitanStormFile &sfile,
                            vector<TrStorm*> &storms,
                            bool allow_field_tracker);
  
  void _compute_motion(double storm_x,
                       double storm_y,
                       double dx_dt,
                       double dy_dt,
                       double &speed,
                       double &dirn);
 
  int _load_field_tracker_data(const TitanStormFile &sfile,
                               DsMdvx &fieldTrackerData,
                               MdvxField* &uField,
                               MdvxField* &vField,
                               MdvxProj &fieldProj);

  void _override_motion_using_centroid(TitanStormFile &sfile,
                                       const MdvxField *uField,
                                       const MdvxField *vField,
                                       titan_grid_comps_t &titanProj,
                                       const MdvxProj &fieldProj,
                                       const storm_file_global_props_t &gprops,
                                       TrTrack &track);
  
  void _override_motion_using_perimeter(TitanStormFile &sfile,
                                        const MdvxField *uField,
                                        const MdvxField *vField,
                                        titan_grid_comps_t &titanProj,
                                        const MdvxProj &fieldProj,
                                        const storm_file_global_props_t &gprops,
                                        TrTrack &track);
  
  
};

#endif



