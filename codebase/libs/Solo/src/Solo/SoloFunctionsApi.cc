#include <Solo/SoloFunctionsApi.hh>
#include <Solo/OneBoundary.hh>
#include <Solo/BoundaryPointMap.hh>
#include <Solo/PointInSpace.hh>
#include <Solo/SoloFunctions.hh>
#include <Solo/GeneralDefinitions.hh>

//void SoloFunctionsApi::CreateBoundary() {
//
//}

SoloFunctionsApi::SoloFunctionsApi() {}


// get the boundary mask for a given:
// - list of boundary vertices
// - ray geometry
//
//short *SoloFunctionsApi::GetBoundaryMask(OneBoundary *boundaryList,
//                                           PointInSpace *radar_origin,
//                                           PointInSpace *boundary_origin,
void SoloFunctionsApi::GetBoundaryMask(long *xpoints, long *ypoints, int npoints,
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
				       float rotation_angle,
				       bool *boundary_mask_out) {

/*
  // map flat data to internal data structures ...
  PointInSpace *radar_origin = new PointInSpace;
  PointInSpace *boundary_origin = new PointInSpace;

  // insert xy-points into OneBoundary structure
  OneBoundary *boundary = new OneBoundary();
  BoundaryPointMap map;

  // Fill the OneBoundary object with the boundary points
  for (int i=0; i<npoints; i++) {
    map.xse_add_bnd_pt(xpoints[i], ypoints[i], boundary);
  }

  //  radar_origin->x = radar_origin_x;
  //  radar_origin->y = radar_origin_y;
  //  radar_origin->z = radar_origin_z;
  radar_origin->latitude = radar_origin_latitude;
  radar_origin->longitude = radar_origin_longitude;
  radar_origin->altitude = radar_origin_altitude;

  //boundary_origin->x = boundary_origin_x;
  //boundary_origin->y = boundary_origin_y;
  //boundary_origin->z = boundary_origin_z;
  boundary_origin->latitude = boundary_origin_latitude;
  boundary_origin->longitude = boundary_origin_longitude;
  boundary_origin->altitude = boundary_origin_altitude;
  boundary_origin->tilt = boundary_origin_tilt;

  //  send the filled boundary object to the function that will
  // create the mask for the current ray geometry
  map.get_boundary_mask(  
  boundary,
    // bool new_sweep,  // assume new_sweep                                                                       
    //        bool operate_outside_bnd,                                                                           
    //bool shift_bnd,  // always shift                                                                            
    radar_origin,
    boundary_origin,
    nGates,
    gateSize,
    distanceToCellNInMeters,
    azimuth,    // TODO: are azimuth & rotation_angle the same thing? YES                                   
    radar_scan_mode,
    radar_type,
    tilt_angle,
  rotation_angle,
  boundary_mask_out);

  //
  // copy boundary mask to output array
  //for (int i=0; i<nGates; i++)
  //  boundary_mask_out[i] = boundary_mask[i];
  //

  delete boundary;
  delete radar_origin;
  delete boundary_origin;

  //return boundary_mask;
  */

  try {
    se_get_boundary_mask(xpoints, ypoints, npoints,
           //float radar_origin_x,
           //  float radar_origin_y,
           //  float radar_origin_z,
             radar_origin_latitude,
             radar_origin_longitude,
             radar_origin_altitude,
             boundary_origin_tilt,
             // float boundary_origin_x,
             // float boundary_origin_y,
             // float boundary_origin_z,
             boundary_origin_latitude,
             boundary_origin_longitude,
             boundary_origin_altitude,
             nGates,
             gateSize,
             distanceToCellNInMeters,
             azimuth,
             radar_scan_mode,
             radar_type,
             tilt_angle,
             rotation_angle,
             boundary_mask_out);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }


} 

void SoloFunctionsApi::Despeckle(const float *data, float *newData, size_t nGates, float bad, int a_speckle,
				 size_t dgi_clip_gate, bool *boundary_mask) {
  try {
    se_despeckle(data, newData, nGates, bad, a_speckle, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

  // data is in/out parameter
void SoloFunctionsApi::RemoveAircraftMotion(float vert_velocity, float ew_velocity, float ns_velocity,
					    float ew_gndspd_corr, float tilt, float elevation,
					    const float *data, float *newData, size_t nGates,
					    float bad, size_t dgi_clip_gate,
					    float dds_radd_eff_unamb_vel, float seds_nyquist_velocity,
					    bool *boundary_mask) {

  // TODO: remove parameter_scale and parameter_bias?

  try {

    /// All data are coming in scaled and biased                                              
    se_remove_ac_motion(vert_velocity, ew_velocity, ns_velocity,
			     ew_gndspd_corr, tilt, elevation,
			     data, newData, nGates,
			     bad, dgi_clip_gate,
			     dds_radd_eff_unamb_vel,
			     seds_nyquist_velocity, boundary_mask);


  } catch(...) {
    throw "Something bad happened during script evaluation";
  }

}

void SoloFunctionsApi::BBUnfoldFirstGoodGate(const float *data, float *newData, size_t nGates,
			   float nyquist_velocity, float dds_radd_eff_unamb_vel,
			   int max_pos_folds, int max_neg_folds,
			   size_t ngates_averaged,
			   float *last_good_v0,
			   float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask) {
  try {

    /// All data are coming in scaled and biased
    // last_good_v0 is in/out parameter                                              
    se_BB_unfold_first_good_gate(data, newData, nGates,
				 nyquist_velocity, dds_radd_eff_unamb_vel,
				 max_pos_folds, max_neg_folds,
				 ngates_averaged,
				 last_good_v0,
				 bad_data_value, dgi_clip_gate, boundary_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }

}



void SoloFunctionsApi::BBUnfoldAircraftWind(const float *data, float *newData, size_t nGates,
			  float nyquist_velocity, float dds_radd_eff_unamb_vel,
			  float azimuth_angle_degrees, float elevation_angle_degrees,
			  float ew_horiz_wind,
			  float ns_horiz_wind,
			  float vert_wind,
			  int max_pos_folds, int max_neg_folds,
			  size_t ngates_averaged,
			  float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask) {
  try {

    /// All data are coming in scaled and biased                                              

    se_BB_unfold_ac_wind(data, newData, nGates,
			    nyquist_velocity, dds_radd_eff_unamb_vel,
			    azimuth_angle_degrees, elevation_angle_degrees,
			    ew_horiz_wind, ns_horiz_wind, vert_wind,
			    max_pos_folds, max_neg_folds,
			    ngates_averaged,
			    bad_data_value, dgi_clip_gate, boundary_mask);


  } catch(...) {
    throw "Something bad happened during script evaluation";
  }

}


void SoloFunctionsApi::BBUnfoldLocalWind(const float *data, float *newData, size_t nGates,
		       float nyquist_velocity, float dds_radd_eff_unamb_vel,
		       float azimuth_angle_degrees, float elevation_angle_degrees,
		       float ew_wind, float ns_wind, float ud_wind,
		       int max_pos_folds, int max_neg_folds,
		       size_t ngates_averaged,
		       float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask) {
  try {

    /// All data are coming in scaled and biased                                              

    se_BB_unfold_local_wind(data, newData, nGates,
			    nyquist_velocity, dds_radd_eff_unamb_vel,
			    azimuth_angle_degrees, elevation_angle_degrees,
			    ew_wind, ns_wind, ud_wind,
			    max_pos_folds, max_neg_folds,
			    ngates_averaged,
			    bad_data_value, dgi_clip_gate, boundary_mask);


  } catch(...) {
    throw "Something bad happened during script evaluation";
  }

}



void SoloFunctionsApi::ZeroInsideBoundary(const float *data, bool *boundaryMask,
					  float *newData, size_t nGates) {


  if (boundaryMask != NULL) {
    for (size_t i=0; i<nGates; i++) {                                                                  
      if (!boundaryMask[i])                                                                                      
        newData[i] = data[i];                                                                                    
      else                                                                                                       
        newData[i] = 0.0;                                                                                        
    }             
  } else {
    for (size_t i=0; i<nGates; i++) {
      newData[i] = 0.0;
    } 
  }
}

//
// Bad flag operations
//

// The API offers specific versions of logical operators.
// The API converts the specific logical function to an internal
// enumeration for (AND, OR, XOR) and (ABOVE, BELOW, BETWEEN)
//
// parameters:
// in    data
// in/out bad_flag_mask
// new variable is bad_flag_mask
void SoloFunctionsApi::SetBadFlagsAbove(float scaled_thr1, 
				   const float *data, size_t nGates,
				   float bad, size_t dgi_clip_gate,
				   bool *boundary_mask, bool *bad_flag_mask) {
  try {
    float scaled_thr2 = 0.0;
    enum Where where = ABOVE; 
    se_set_bad_flags(where, scaled_thr1, scaled_thr2,
		     data, nGates, 
		     bad, dgi_clip_gate,
		     boundary_mask, bad_flag_mask);
 
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

//
// parameters:
// in    data
// in/out bad_flag_mask
// new variable is bad_flag_mask
void SoloFunctionsApi::SetBadFlagsBelow(float scaled_thr1, 
				   const float *data, size_t nGates,
				   float bad, size_t dgi_clip_gate,
				   bool *boundary_mask, bool *bad_flag_mask) {
  try {
    float scaled_thr2 = 0.0;
    se_set_bad_flags(BELOW, scaled_thr1, scaled_thr2,
		     data, nGates, 
		     bad, dgi_clip_gate,
		     boundary_mask, bad_flag_mask);
 
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

//
// parameters:
// in    data
// in/out bad_flag_mask
// new variable is bad_flag_mask
void SoloFunctionsApi::SetBadFlagsBetween(float scaled_thr1, float scaled_thr2,
				   const float *data, size_t nGates,
				   float bad, size_t dgi_clip_gate,
				   bool *boundary_mask, bool *bad_flag_mask) {
  try {
    se_set_bad_flags(BETWEEN, scaled_thr1, scaled_thr2,
		     data, nGates, 
		     bad, dgi_clip_gate,
		     boundary_mask, bad_flag_mask);
 
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

//
// parameters:
// in   data
// in   bad_flag_mask
// in/out newData 
void SoloFunctionsApi::AssertBadFlags(const float *data, float *newData, size_t nGates,
				      float bad, size_t dgi_clip_gate,
				      bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    se_assert_bad_flags(data, newData, nGates,
			bad, dgi_clip_gate,
			boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// ---  Bad Flag Mask UPDATE operations 
//
// parameters:
// in/out   bad_flag_mask
void SoloFunctionsApi::ClearBadFlags(bool *bad_flag_mask, size_t nGates) {
  try {
    bool complement = false;
    se_clear_bad_flags(complement, NULL, bad_flag_mask, nGates);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

//
// parameters:
// in/out   bad_flag_mask
void SoloFunctionsApi::ComplementBadFlags(const bool *bad_flag_mask, 
					  bool *complement_mask, size_t nGates) {
  try {
    bool complement = true;
    se_clear_bad_flags(complement, bad_flag_mask, complement_mask, nGates);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


//
// Add or multiply a constant to every data value marked bad;
// return result in newData array.
// if multiply is true, multiply the constant
// if multiply is false, add the constant

// parameters:
// in      data
// in/out  newData
// in      bad_flag_mask
void SoloFunctionsApi::FlaggedAdd(float f_const, bool multiply, 
				  const float *data, float *newData, size_t nGates,
				  float bad, size_t dgi_clip_gate,
				  bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    se_flagged_add(f_const, multiply,
		   data, newData, nGates,
		   bad, dgi_clip_gate,
		   boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

//
// Apply logical expression to every data value marked bad that also
// fits the selection criteria (ABOVE, BELOW, BETWEEN)
// return result in bool mask array.

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::XorBadFlagsBetween(float scaled_thr1, float scaled_thr2, 
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BETWEEN;
    enum Logical logical_operator = XOR;
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::OrBadFlagsBetween(float scaled_thr1, float scaled_thr2, 
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BETWEEN;
    enum Logical logical_operator = OR;
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::AndBadFlagsBetween(float scaled_thr1, float scaled_thr2, 
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BETWEEN;
    enum Logical logical_operator = AND;
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// ----

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::XorBadFlagsAbove(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = ABOVE;
    enum Logical logical_operator = XOR;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::XorBadFlagsBelow(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BELOW;
    enum Logical logical_operator = XOR;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::AndBadFlagsAbove(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = ABOVE;
    enum Logical logical_operator = AND;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::AndBadFlagsBelow(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BELOW;
    enum Logical logical_operator = AND;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::OrBadFlagsAbove(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = ABOVE;
    enum Logical logical_operator = OR;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

// parameters:
// in      data
// in/out  updated_bad_flag_mask
// in      bad_flag_mask

void SoloFunctionsApi::OrBadFlagsBelow(float scaled_thr1,  
					  const float *data, size_t nGates,
					  float bad, size_t dgi_clip_gate,
					  bool *boundary_mask, const bool *bad_flag_mask,
					  bool *updated_bad_flag_mask) {

  try {
    enum Where where = BELOW;
    enum Logical logical_operator = OR;
    float scaled_thr2 = 0.0; 
    se_bad_flags_logic(scaled_thr1, scaled_thr2, where,
		       logical_operator, data, nGates,
		       bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask,
		       updated_bad_flag_mask);

  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}




/*
void SoloFunctionsApi::se_clear_bad_flags(bool complement, size_t nGates,
					  bool *bad_flag_mask) {
  try {
    se_despeckle(data, newData, nGates, bad, a_speckle, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}
*/

void SoloFunctionsApi::CopyBadFlags(const float *data, size_t nGates,
				    float bad, size_t dgi_clip_gate,
				    bool *boundary_mask, bool *bad_flag_mask) {
  try {
    se_copy_bad_flags(data, nGates, bad, dgi_clip_gate, boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::FlaggedAssign(float constant, const float *data, float *newData, size_t nGates,
		   size_t dgi_clip_gate,
		   bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    se_assign_value(constant, data, newData, nGates, dgi_clip_gate, boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::FlaggedCopy(const float *data, float *newData, size_t nGates,
		 size_t dgi_clip_gate,
		 bool *boundary_mask, const bool *bad_flag_mask) {
  //  try {
  //??     se_copy_bad_flags(data, nGates, bad, dgi_clip_gate, boundary_mask, bad_flag_mask);
  //  } catch(...) {
  throw "Not implemented"; // "Something bad happened during script evaluation";
    //  }
}


void SoloFunctionsApi::FlagFreckles(float freckle_threshold, size_t freckle_avg_count,
		  const float *data, size_t nGates,
		  float bad, size_t dgi_clip_gate,
		  bool *boundary_mask, bool *bad_flag_mask) {
  try {
    se_flag_freckles(freckle_threshold, freckle_avg_count,
		     data, nGates,
		     bad, dgi_clip_gate,
		     boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::FlagGlitches(float deglitch_threshold, int deglitch_radius,
		  int deglitch_min_bins,  // aka deglitch_min_gates                                
		  const float *data, size_t nGates,
		  float bad, size_t dgi_clip_gate,
		  bool *boundary_mask, bool *bad_flag_mask){
  try {
    se_flag_glitches(deglitch_threshold, deglitch_radius,
		     deglitch_min_bins, data, nGates,
		     bad, dgi_clip_gate,
		     boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::ThresholdFieldAbove(float scaled_thr,
			 int first_good_gate,
			 const float *data, const float *thr_data, size_t nGates,
			 float *newData,
			 float bad, float thr_bad, size_t dgi_clip_gate,
			 bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    Where where = ABOVE;
    float threshold2 = 0.0;
    se_threshold_field(where, scaled_thr, threshold2,
		       first_good_gate,
		       data, thr_data, nGates,
		       newData,
		       bad, thr_bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::ThresholdFieldBelow(float scaled_thr,
			 int first_good_gate,
			 const float *data, const float *thr_data, size_t nGates,
			 float *newData,
			 float bad, float thr_bad, size_t dgi_clip_gate,
			 bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    Where where = BELOW;
    float threshold2 = 0.0;
    se_threshold_field(where, scaled_thr, threshold2,
		       first_good_gate,
		       data, thr_data, nGates,
		       newData,
		       bad, thr_bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::ThresholdFieldBetween(float scaled_thr1, float scaled_thr2,
			   int first_good_gate,
			   const float *data, const float *thr_data, size_t nGates,
			   float *newData,
			   float bad, float thr_bad, size_t dgi_clip_gate,
			   bool *boundary_mask, const bool *bad_flag_mask) {
  try {
    Where where = BETWEEN;
    se_threshold_field(where, scaled_thr1, scaled_thr2,
		       first_good_gate,
		       data, thr_data, nGates,
		       newData,
		       bad, thr_bad, dgi_clip_gate,
		       boundary_mask, bad_flag_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


void SoloFunctionsApi::ForceUnfolding(const float *data, float *newData, size_t nGates,
		    float nyquist_velocity, float dds_radd_eff_unamb_vel,
		    float center,
		    float bad_data_value, size_t dgi_clip_gate,
		    bool *boundary_mask) {
  try {
    se_funfold(data, newData, nGates,
	       nyquist_velocity, dds_radd_eff_unamb_vel,
	       center,
	       bad_data_value, dgi_clip_gate,
	       boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


void SoloFunctionsApi::MergeFields(const float *data1, const float *data2,
		 float *newData, size_t nGates,
		 float bad, size_t dgi_clip_gate,
		 bool *boundary_mask) {
  try {
    se_merge_fields(data1, data2,
		    newData, nGates,
		    bad, dgi_clip_gate,
		    boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::RemoveRing(size_t from_km, size_t to_km,
		const float *data, float *newData, size_t nGates,
		float bad, size_t dgi_clip_gate, bool *boundary_mask) {
  try {
    se_ring_zap(from_km, to_km,
		data, newData, nGates,
		bad, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::RainRate(float d_const, const float *data, float *newData, size_t nGates,
	      float bad, size_t dgi_clip_gate,
	      bool *boundary_mask) {
  try {
    se_rain_rate(d_const, data, newData, nGates,
		 bad, dgi_clip_gate,
		 boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::Abs(const float *data, float *newData, size_t nGates,
	 float bad, size_t dgi_clip_gate, bool *boundary_mask) {
  try {
    se_absolute_value(data, newData, nGates,
		      bad, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::Exp(float f_const, const float *data, float *newData, size_t nGates,
	 float bad, size_t dgi_clip_gate,
	 bool *boundary_mask) {
  try {
    se_mult_const(f_const, data, newData, nGates,
		  bad, dgi_clip_gate,
		  boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


void SoloFunctionsApi::RadialShear(size_t seds_gate_diff_interval, const float *data, float *newData,
		 size_t nGates,
		 float bad_data_value, size_t dgi_clip_gate,
		 bool *boundary_mask) {
  try {
    se_radial_shear(seds_gate_diff_interval, data, newData,
		    nGates,
		    bad_data_value, dgi_clip_gate,
		    boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

void SoloFunctionsApi::RemoveStormMotion(float wind, float speed, float dgi_dd_rotation_angle,
		       float dgi_dd_elevation_angle,
		       const float *data, float *new_data, size_t nGates,
		       float bad, size_t dgi_clip_gate, bool *boundary_mask) {
  try {
    se_remove_storm_motion(wind, speed, dgi_dd_rotation_angle,
			   dgi_dd_elevation_angle,
			   data, new_data, nGates,
			   bad, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}

/*
void SoloFunctionsApi::se_flag_glitches(float deglitch_threshold, int deglitch_radius,
                      int deglitch_min_bins,  // aka deglitch_min_gates                                  
                      const float *data, size_t nGates,
                      float bad, size_t dgi_clip_gate,
					bool *boundary_mask, bool *bad_flag_mask) {
  try {
    se_despeckle(data, newData, nGates, bad, a_speckle, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}


void SoloFunctionsApi::Despeckle(const float *data, float *newData, size_t nGates, float bad, int a_speckle,
				 size_t dgi_clip_gate, bool *boundary_mask) {
  try {
    se_despeckle(data, newData, nGates, bad, a_speckle, dgi_clip_gate, boundary_mask);
  } catch(...) {
    throw "Something bad happened during script evaluation";
  }
}
*/
