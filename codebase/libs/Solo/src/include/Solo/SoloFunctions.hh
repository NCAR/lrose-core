// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2020                                         
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

#ifndef SOLOFUNCTIONS_H
#define SOLOFUNCTIONS_H

#include <stdio.h>
#include <Solo/GeneralDefinitions.hh>

void se_despeckle(const float *data, float *newData, size_t nGates, float bad, int a_speckle,
                  size_t dgi_clip_gate, bool *boundary_mask);


void se_remove_ac_motion(float vert_velocity, float ew_velocity, float ns_velocity,
                         float ew_gndspd_corr, float tilt, float elevation,
                         const float *data, float *newData, size_t nGates,
                         float bad, size_t dgi_clip_gate,
                         float dds_radd_eff_unamb_vel,
                         float seds_nyquist_velocity, bool *bnd);

// float running_average(std::vector<float> const& v);
bool is_data_bad(float data, float bad_data_value);
// {
//  return abs(newData[ssIdx] - bad) < 0.0001;
//}

void se_BB_unfold_ac_wind(const float *data, float *newData, size_t nGates,
                          float nyquist_velocity, float dds_radd_eff_unamb_vel,
                          float azimuth_angle_degrees, float elevation_angle_degrees,
                          float ew_horiz_wind,
                          float ns_horiz_wind,
                          float vert_wind,
                          int max_pos_folds, int max_neg_folds,
                          size_t ngates_averaged,
                          float bad_data_value, size_t dgi_clip_gate, bool *bnd);

void se_BB_unfold_local_wind(const float *data, float *newData, size_t nGates,
                             float nyquist_velocity, float dds_radd_eff_unamb_vel,
                             float azimuth_angle_degrees, float elevation_angle_degrees,
                             float ew_wind, float ns_wind, float ud_wind,
                             int max_pos_folds, int max_neg_folds,
                             size_t ngates_averaged,
                             float bad_data_value, size_t dgi_clip_gate, bool *bnd);

void se_BB_unfold_first_good_gate(const float *data, float *newData, size_t nGates,
                                  float nyquist_velocity, float dds_radd_eff_unamb_vel,
				  //                                  float azimuth_angle_degrees, float elevation_angle_degrees,
                                  int max_pos_folds, int max_neg_folds,
                                  size_t ngates_averaged,
                                  float *last_good_v0,
                                  float bad_data_value, size_t dgi_clip_gate, bool *bnd);




void se_BB_generic_unfold(const float *data, float *newData, size_t nGates,
                          float *v0, size_t ngates_averaged,
                          float nyquist_velocity,
                          int BB_max_pos_folds, int BB_max_neg_folds,
                          float bad_data_value, size_t dgi_clip_gate, bool *bnd);

// Bad flag operations

void se_do_clear_bad_flags_array(bool *bad_flag_mask, size_t nn);

void se_set_bad_flags(Where where, float scaled_thr1, float scaled_thr2, const float *data, 
		      size_t nGates,
                      float bad, size_t dgi_clip_gate, bool *boundary_mask, bool *bad_flag_mask);

void se_assert_bad_flags(const float *data, float *newData, size_t nGates,
                         float bad, size_t dgi_clip_gate,
                         bool *boundary_mask, const bool *bad_flag_mask);

//void se_flagged_add(float f_const, bool multiply, const float *data, float *newData, size_t nGates,
//		    float bad, size_t dgi_clip_gate,
//		    bool *boundary_mask, const bool *bad_flag_mask,
//		    bool *updated_bad_flag_mask);

void se_flagged_add(float f_const, bool multiply, const float *data, float *newData, size_t nGates,
                    float bad, size_t dgi_clip_gate,
                    bool *boundary_mask, const bool *flag);

void se_bad_flags_logic(float scaled_thr1, float scaled_thr2, enum Where where,
                        enum Logical logical_operator, const float *data, size_t nGates,
			float bad, size_t dgi_clip_gate,
			bool *boundary_mask, const bool *bad_flag_mask,
			bool *updated_bad_flag_mask);

void se_clear_bad_flags(bool complement,
                        const bool *bad_flag_mask,
                        bool *complement_mask, size_t nGates);

void se_copy_bad_flags(const float *data, size_t nGates,
		       float bad, size_t dgi_clip_gate,
		       bool *boundary_mask, bool *bad_flag_mask);

void se_flag_glitches(float deglitch_threshold, int deglitch_radius,
                      int deglitch_min_bins,  // aka deglitch_min_gates                              
                      const float *data, size_t nGates,
                      float bad, size_t dgi_clip_gate,
                      bool *boundary_mask, bool *bad_flag_mask);

void se_flag_freckles(float freckle_threshold, size_t freckle_avg_count,
                      const float *data, size_t nGates,
                      float bad, size_t dgi_clip_gate,
                      bool *boundary_mask, bool *bad_flag_mask);

// flagged-assign
void se_flagged_assign_value(float constant, const float *data, float *newData, size_t nGates,
                    size_t dgi_clip_gate,
                    bool *boundary_mask, const bool *bad_flag_mask);

void se_assign_value(const float *data, float *newData, size_t nGates,
         float value, size_t dgi_clip_gate, bool *boundary_mask);

void se_threshold_field(Where where, float scaled_thr1, float scaled_thr2,
                        int first_good_gate,
                        const float *data, const float *thr_data, size_t nGates,
                        float *newData,
                        float bad, float thr_bad, size_t dgi_clip_gate,
                        bool *boundary_mask, const bool *bad_flag_mask);


void se_funfold(const float *data, float *newData, size_t nGates,
		float nyquist_velocity, float dds_radd_eff_unamb_vel,
		float center,
		float bad_data_value, size_t dgi_clip_gate,
		bool *boundary_mask);

void se_merge_fields(const float *data1, const float *data2,
                     float *newData, size_t nGates,
                     float bad, size_t dgi_clip_gate,
                     bool *boundary_mask);

// RemoveRing.cc
void se_ring_zap(size_t from_km, size_t to_km,
                 const float *data, float *newData, size_t nGates,
                 float bad, size_t dgi_clip_gate, bool *boundary_mask);

void se_rain_rate(float d_const, const float *data, float *newData, size_t nGates,
                  float bad, size_t dgi_clip_gate,
                  bool *boundary_mask);

void se_absolute_value(const float *data, float *newData, size_t nGates,
                       float bad, size_t dgi_clip_gate, bool *boundary_mask);

// Exp.cc 
void se_mult_const(float f_const, const float *data, float *newData, size_t nGates,
                   float bad, size_t dgi_clip_gate,
                   bool *boundary_mask);

void se_radial_shear(size_t seds_gate_diff_interval, const float *data, float *newData, 
		     size_t nGates,
                     float bad_data_value, size_t dgi_clip_gate,
                     bool *boundary_mask);

void se_remove_storm_motion(float wind, float speed, float dgi_dd_rotation_angle,
			    float dgi_dd_elevation_angle,
			    const float *data, float *new_data, size_t nGates,
			    float bad, size_t dgi_clip_gate, bool *boundary_mask);

void se_ac_surface_tweak(enum Surface_Type which_removal,  // internal value based on function call
     float optimal_beamwidth,      // script parameter; origin seds->optimal_beamwidth
     int seds_surface_gate_shift,       // script parameter; origin seds->surface_gate_shift
     float vert_beam_width,        // from radar angles???; origin dgi->dds->radd->vert_beam_width
     float asib_altitude_agl,      // altitude angle ???
     float dds_ra_elevation,       // radar angles!! requires cfac values and calculation
                           // origin dds->ra->elevation, ra = radar_angles
                           // get this from RadxRay::_elev if RadxRay::_georefApplied == true
     bool getenv_ALTERNATE_GECHO,  // script parameter
     double d, // used for min_grad, if getenv_ALTERNATE_GECHO is true
               // d = ALTERNATE_GECHO environment variable
     double dds_asib_rotation_angle,  // origin dds->asib->rotation_angle;  asib is struct platform_i
     double dds_asib_roll,            // origin dds->asib->roll
     double dds_cfac_rot_angle_corr,  // origin dds->cfac->rot_angle_corr; cfac is struct correction_d
     float radar_latitude,  // radar->latitude 
     const float *data,     // internal value
     float *new_data,       // internal value
     size_t nGates,         // internal value
     float gate_size,
     float distance_to_first_gate,
     double max_range,      // internal value; origin dds->celvc_dist_cells[dgi_clip_gate];
     float bad_data_value,  // default value
     size_t dgi_clip_gate,  // default value
     bool *boundary_mask);

void se_unconditional_delete(const float *data, float *newData, size_t nGates,
         float bad, size_t dgi_clip_gate, bool *boundary_mask);

 void se_hard_zap(
      const float *data, size_t nGates,
      float *newData,
      float bad, size_t dgi_clip_gate,
      bool *boundary_mask);

 void dd_radar_angles( 
     float asib_roll,
     float asib_pitch,
     float asib_heading,
     float asib_drift_angle,
     float asib_rotation_angle,
     float asib_tilt,


     float cfac_pitch_corr,
     float cfac_heading_corr,
     float cfac_drift_corr,
     float cfac_roll_corr,
     float cfac_elevation_corr,
     float cfac_azimuth_corr,
     float cfac_rot_angle_corr,
     float cfac_tilt_corr,
     int radar_type,  // from dgi->dds->radd->radar_type
     bool use_Wen_Chaus_algorithm,
    float dgi_dds_ryib_azimuth,
    float dgi_dds_ryib_elevation,
     float *ra_x,
     float *ra_y,
     float *ra_z,
     float *ra_rotation_angle,
     float *ra_tilt,
     float *ra_azimuth,
     float *ra_elevation,
     float *ra_psi
);


// given a list of x, y points, an other geo data,
// return a mask of booleans where 
//   true is inside the boundary and 
//   false is outside the boundary
void se_get_boundary_mask(long *xpoints, long *ypoints, int npoints,
             //float radar_origin_x,                                                      
             //  float radar_origin_y,                                                    
             //  float radar_origin_z,                                                    
             float radar_origin_latitude,
             float radar_origin_longitude,
             float radar_origin_altitude,
             float boundary_origin_tilt,
             // float boundary_origin_x,                                                
             // float boundary_origin_y,                                                
             // float boundary_origin_z,                                                
             float boundary_origin_latitude,
             float boundary_origin_longitude,
             float boundary_origin_altitude,
             int nGates,
             float gateSize,
             float distanceToCellNInMeters,
             float azimuth,
             int radar_scan_mode,
             int radar_type,
             float tilt_angle,
             bool *boundary_mask);

#endif
