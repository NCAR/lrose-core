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

#ifndef SOLOFUNCTIONSAPI_H
#define SOLOFUNCTIONSAPI_H

#include <stdio.h>

// #include <Solo/SoloFunctionsApiDefinitions.hh>

using namespace std;

// This is the external interface to the Solo Functions Library

class SoloFunctionsApi
{


public:
  SoloFunctionsApi();

  //  void CreateBoundary(short *xpoints, short *ypoints, int npoints, char *name);


  // 
  // call BoundaryPointMap::get_boundary_mask to do the work
  // 
  //  short *GetBoundaryMask(OneBoundary *boundaryList,
  void GetBoundaryMask(long *xpoints, long *ypoints, int npoints,
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


  // TODO: use Radx::SI16, etc? to standardize the numberic sizes?  Ask Mike about this.
  // Q: send a pointer to the boundary mask? or the name of the boundary mask?
  // name of the boundary mask, 1st.  Then, translate the name to the list of
  // boolean mask; The API performs the translation.

  void Despeckle(const float *data, float *newData, size_t nGates, float bad, int a_speckle,
				   size_t dgi_clip_gate, bool *boundary_mask);

  void RemoveAircraftMotion(float vert_velocity, float ew_velocity, float ns_velocity,
			    float ew_gndspd_corr, float tilt, float elevation,
			    const float *data, float *newData, size_t nGates,
			    float bad, size_t dgi_clip_gate,
			    float dds_radd_eff_unamb_vel, float seds_nyquist_velocity,
			    bool *boundary_mask);

  void BBUnfoldFirstGoodGate(const float *data, float *newData, size_t nGates,
				    float nyquist_velocity, float dds_radd_eff_unamb_vel,
				    int max_pos_folds, int max_neg_folds,
				    size_t ngates_averaged,
				    float *last_good_v0,
				    float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask);

  void BBUnfoldAircraftWind(const float *data, float *newData, size_t nGates,
			    float nyquist_velocity, float dds_radd_eff_unamb_vel,
			    float azimuth_angle_degrees, float elevation_angle_degrees,
			    float ew_horiz_wind,
			    float ns_horiz_wind,
			    float vert_wind,
			    int max_pos_folds, int max_neg_folds,
			    size_t ngates_averaged,
			    float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask);

  void BBUnfoldLocalWind(const float *data, float *newData, size_t nGates,
			       float nyquist_velocity, float dds_radd_eff_unamb_vel,
			       float azimuth_angle_degrees, float elevation_angle_degrees,
			       float ew_wind, float ns_wind, float ud_wind,
			       int max_pos_folds, int max_neg_folds,
			       size_t ngates_averaged,
			       float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask);

  void ZeroInsideBoundary(const float *data, bool *boundaryMask,
			  float *newData, size_t nGates);

  void SetBadFlagsAbove(float scaled_thr1,
		   const float *data, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask, bool *bad_flag_mask);

  void SetBadFlagsBelow(float scaled_thr1,
		   const float *data, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask, bool *bad_flag_mask);

  void SetBadFlagsBetween(float scaled_thr1, float scaled_thr2,
		   const float *data, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask, bool *bad_flag_mask);

  void AssertBadFlags(const float *data, float *newData, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask, const bool *bad_flag_mask);

  // This just creates a new mask with all false
  void ClearBadFlags(bool *bad_flag_mask, size_t nGates);

  // bad_flag_mask:   in parameter
  // complement_mask: out parameter
  void ComplementBadFlags(const bool *bad_flag_mask, bool *complement_mask, size_t nGates);

  void FlaggedAdd(float f_const, bool multiply,
	     const float *data, float *newData, size_t nGates,
	     float bad, size_t dgi_clip_gate,
	     bool *boundary_mask, const bool *bad_flag_mask);

  void XorBadFlagsBetween(float scaled_thr1, float scaled_thr2,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void OrBadFlagsBetween(float scaled_thr1, float scaled_thr2,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void AndBadFlagsBetween(float scaled_thr1, float scaled_thr2,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void XorBadFlagsAbove(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void XorBadFlagsBelow(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void AndBadFlagsAbove(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void AndBadFlagsBelow(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void OrBadFlagsAbove(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  void OrBadFlagsBelow(float scaled_thr1,
		     const float *data, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask,
		     bool *updated_bad_flag_mask);

  // Sets bad flag mask to true if data == bad; otherwise, bad flag mask = false
  void CopyBadFlags(const float *data, size_t nGates,
		    float bad, size_t dgi_clip_gate,
		    bool *boundary_mask, bool *bad_flag_mask);

  // Assign all flagged gates to a constant value
  void FlaggedAssign(float constant, const float *data, float *newData, size_t nGates,
		     size_t dgi_clip_gate,
		     bool *boundary_mask, const bool *bad_flag_mask);
  // not implemented
  void FlaggedCopy(const float *data, float *newData, size_t nGates,
		   size_t dgi_clip_gate,
		   bool *boundary_mask, const bool *bad_flag_mask);
  
  void FlagFreckles(float freckle_threshold, size_t freckle_avg_count,
		    const float *data, size_t nGates,
		    float bad, size_t dgi_clip_gate,
		    bool *boundary_mask, bool *bad_flag_mask);
 
  void FlagGlitches(float deglitch_threshold, int deglitch_radius,
		    int deglitch_min_bins,  // aka deglitch_min_gates
		    const float *data, size_t nGates,
		    float bad, size_t dgi_clip_gate,
		    bool *boundary_mask, bool *bad_flag_mask);

  // 
  void ThresholdFieldAbove(float scaled_thr,
			   int first_good_gate,
			   const float *data, const float *thr_data, size_t nGates,
			   float *newData,
			   float bad, float thr_bad, size_t dgi_clip_gate,
			   bool *boundary_mask, const bool *bad_flag_mask);

  void ThresholdFieldBelow(float scaled_thr,
			   int first_good_gate,
			   const float *data, const float *thr_data, size_t nGates,
			   float *newData,
			   float bad, float thr_bad, size_t dgi_clip_gate,
			   bool *boundary_mask, const bool *bad_flag_mask);

  void ThresholdFieldBetween(float scaled_thr1, float scaled_thr2,
			     int first_good_gate,
			     const float *data, const float *thr_data, size_t nGates,
			     float *newData,
			     float bad, float thr_bad, size_t dgi_clip_gate,
			     bool *boundary_mask, const bool *bad_flag_mask);

  void ForceUnfolding(const float *data, float *newData, size_t nGates,
		      float nyquist_velocity, float dds_radd_eff_unamb_vel,
		      float center,
		      float bad_data_value, size_t dgi_clip_gate,
		      bool *boundary_mask);

  void MergeFields(const float *data1, const float *data2,
		   float *newData, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask);

  void RemoveRing(size_t from_km, size_t to_km,
		  const float *data, float *newData, size_t nGates,
		  float bad, size_t dgi_clip_gate, bool *boundary_mask);

  void RainRate(float d_const, const float *data, float *newData, size_t nGates,
		float bad, size_t dgi_clip_gate,
		bool *boundary_mask);

  void Abs(const float *data, float *newData, size_t nGates,
	   float bad, size_t dgi_clip_gate, bool *boundary_mask);

  void Exp(float f_const, const float *data, float *newData, size_t nGates,
	   float bad, size_t dgi_clip_gate,
	   bool *boundary_mask);


  void RadialShear(size_t seds_gate_diff_interval, const float *data, float *newData,
		   size_t nGates,
		   float bad_data_value, size_t dgi_clip_gate,
		   bool *boundary_mask);

  void RemoveStormMotion(float wind, float speed, float dgi_dd_rotation_angle,
			 float dgi_dd_elevation_angle,
			 const float *data, float *new_data, size_t nGates,
			 float bad, size_t dgi_clip_gate, bool *boundary_mask);

  void RemoveOnlySurface(
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

  void UnconditionalDelete(const float *data, float *newData, size_t nGates,
		  float bad, size_t dgi_clip_gate, bool *boundary_mask);

private:

};


#endif
