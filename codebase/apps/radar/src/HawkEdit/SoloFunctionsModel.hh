#ifndef SOLOFUNCTIONSMODEL_H
#define SOLOFUNCTIONSMODEL_H

#include <stdio.h>

#include <vector>
#include <iostream>
#include <string>

#include "Radx/RadxVol.hh"
#include "Solo/SoloFunctionsApi.hh"
using namespace std;


class SoloFunctionsModel
{


public:
  SoloFunctionsModel();

  vector<double> RemoveAircraftMotion(vector<double>, RadxVol *vol);
  vector<double> RemoveAircraftMotion(string fieldName, RadxVol *vol);
  vector<double> RemoveAircraftMotion(string fieldName, RadxVol *vol,
				      int rayIdx, int sweepIdx);

  string RemoveAircraftMotion(string fieldName, RadxVol *vol,
			      int rayIdx, int sweepIdx,
			      float nyquist_velocity,
			      size_t clip_gate,
			      float bad_data_value,
			      string newFieldName);


  string ZeroMiddleThird(string fieldName,  RadxVol *vol,
			 int rayIdx, int sweepIdx,
			 string newFieldName);

  string ZeroInsideBoundary(string fieldName,  RadxVol *vol,
			 int rayIdx, int sweepIdx,
			 string newFieldName);

  string Despeckle(string fieldName,  RadxVol *vol,
		   int rayIdx, int sweepIdx,
		   size_t speckle_length,
		   size_t clip_gate,
		   float bad_data_value,
		   string newFieldName);

  string BBUnfoldFirstGoodGate(string fieldName, RadxVol *vol,
			       int rayIdx, int sweepIdx,
			       float nyquist_velocity,
			       int max_pos_folds,
			       int max_neg_folds,
			       size_t ngates_averaged,
			       size_t clip_gate,
			       float bad_data_value,
			       string newFieldName);


  // Bad flags operations ...

  string SetBadFlagsAbove(string fieldName,  RadxVol *vol,
			  int rayIdx, int sweepIdx,
			  float lower_threshold,
			  size_t clip_gate,
			  float bad_data_value,
			  string badFlagMaskFieldName);

  string SetBadFlagsBelow(string fieldName,  RadxVol *vol,
			  int rayIdx, int sweepIdx,
			  float lower_threshold,
			  size_t clip_gate,
			  float bad_data_value,
			  string badFlagMaskFieldName);

  string SetBadFlagsBetween(string fieldName,  RadxVol *vol,
			  int rayIdx, int sweepIdx,
			  float lower_threshold,
			  float upper_threshold,
			  size_t clip_gate,
			  float bad_data_value,
			  string badFlagMaskFieldName);

  string SetBadFlags(string fieldName,  RadxVol *vol,
		     int rayIdx, int sweepIdx,
                     string where,
		     float lower_threshold, float upper_threshold,
		     size_t clip_gate,
		     float bad_data_value,
		     string badFlagMaskFieldName);


  string AssertBadFlags(string fieldName,  RadxVol *vol,
			int rayIdx, int sweepIdx,
			size_t clip_gate,
			float bad_data_value,
			string badFlagMaskFieldName);

  string ClearBadFlags(string badFlagMaskFieldName,  RadxVol *vol,
		       int rayIdx, int sweepIdx);
		       //size_t clip_gate,
		       //float bad_data_value,
		       //string badFlagMaskFieldName);

  string ComplementBadFlags(string fieldName,  RadxVol *vol,
			    int rayIdx, int sweepIdx);

  string FlaggedAdd(string fieldName,  RadxVol *vol,
	     int rayIdx, int sweepIdx,
	     float constant,
	     size_t clip_gate,
	     float bad_data_value,
	     string flagFieldName);

  string FlaggedMultiply(string fieldName,  RadxVol *vol,
	     int rayIdx, int sweepIdx,
	     float constant,
	     size_t clip_gate,
	     float bad_data_value,
	     string flagFieldName);

  string AndBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string AndBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string AndBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
			 string newFieldName);

  string OrBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string OrBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string OrBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
			 string newFieldName);

  string XorBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string XorBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constant, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  string XorBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			 float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
			 string newFieldName);
  
  string CopyBadFlags(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		      size_t clip_gate, float bad_data_value);

  // string flagFieldName
  string FlaggedAssign(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		       float constant,
		       size_t clip_gate, string maskFieldName);
  // Not implemented
  string FlaggedCopy(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		       size_t clip_gate, string maskFieldName);

  string FlagFreckles(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		      float freckle_threshold, size_t freckle_avg_count,
		      size_t clip_gate, float bad_data_value);

  string FlagGlitches(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		      float deglitch_threshold, int deglitch_radius,
		      int deglitch_min_gates,
		      size_t clip_gate, float bad_data_value);

  string ThresholdFieldAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			     float scaled_thr,
			     int first_good_gate, string threshold_field,
			     float threshold_bad_data_value,
			     size_t clip_gate, float bad_data_value,
			     string bad_flag_mask_field_name);

  string ThresholdFieldBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			     float scaled_thr,
			     int first_good_gate, string threshold_field,
			     float threshold_bad_data_value,
			     size_t clip_gate, float bad_data_value,
			     string bad_flag_mask_field_name);

  string ThresholdFieldBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			       float lower_threshold, float upper_threshold,
			       int first_good_gate, string threshold_field,
			       float threshold_bad_data_value,
			       size_t clip_gate, float bad_data_value,
			       string bad_flag_mask_field_name);

  string ForceUnfolding(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		 float nyquist_velocity, 
		 float center,
		 float bad_data_value, size_t dgi_clip_gate);
  
/*
  string BBUnfoldAircraftWind(string fieldName, RadxVol *vol,
			      int rayIdx, int sweepIdx,
			      float nyquist_velocity,
			      int max_pos_folds,
			      int max_neg_folds,
			      size_t ngates_averaged,
			      size_t clip_gate,
			      float bad_data_value,
			      string newFieldName);

  */
  /*
const float *data, float *newData, size_t nGates,
                            float nyquist_velocity, float dds_radd_eff_unamb_vel,
                            float azimuth_angle_degrees, float elevation_angle_degrees,
// these come from the platform (asib) information
                            float ew_horiz_wind,
                            float ns_horiz_wind,
                            float vert_wind,

                            int max_pos_folds, int max_neg_folds,
                            size_t ngates_averaged,
                            float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask);
  */
  /*
  string BBUnfoldLocalWind(string fieldName, RadxVol *vol,
			   int rayIdx, int sweepIdx,
			   float nyquist_velocity,
			   float ew_wind, float ns_wind, float ud_wind,
			   int max_pos_folds,
			   int max_neg_folds,
			   size_t ngates_averaged,
			   size_t clip_gate,
			   float bad_data_value,
			   string newFieldName);
  */
  /*
const float *data, float *newData, size_t nGates,
			 float nyquist_velocity, float dds_radd_eff_unamb_vel,
			 float azimuth_angle_degrees, float elevation_angle_degrees,
// these come from the editor (from the script)
			 float ew_wind, float ns_wind, float ud_wind,

			 int max_pos_folds, int max_neg_folds,
			 size_t ngates_averaged,
			 float bad_data_value, size_t dgi_clip_gate, bool *boundary_mask);
  */


  void SetBoundaryMask(RadxVol *vol, int rayIdx, int sweepIdx, bool useBoundaryMask);
  void SetBoundaryMaskOriginal(RadxVol *vol, int rayIdx, int sweepIdx);
  void CheckForDefaultMask(RadxVol *vol, int rayIdx, int sweepIdx, bool determineMask);
  void SetDefaultMask(RadxVol *vol, int rayIdx, int sweepIdx);
  const vector<bool> *GetBoundaryMask();

  void DetermineBoundaryMask(RadxVol *vol, int rayIdx, int sweepIdx); 
  void printBoundaryMask();
  RadxField *fetchDataField(RadxRay *ray, string &fieldName);
  const vector<float> *GetData(string fieldName,  RadxVol *vol,
              int rayIdx, int sweepIdx);
  void SetData(string &fieldName,  RadxVol *vol,
            int rayIdx, int sweepIdx, vector<float> *data);


private:

  //  SpreadSheetModel *dataModel;
  bool *_boundaryMask;
  size_t _boundaryMaskLength;
  bool _boundaryMaskSet;

  string _flaggedAddMultiply(string fieldName,  RadxVol *vol,
	     int rayIdx, int sweepIdx,
             bool multiply,
	     float constant,
	     size_t clip_gate,
	     float bad_data_value,
	     string flagFieldName);

  string _generalLogicalFx(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			   float constant,
			   size_t clip_gate, float bad_data_value,
			   string maskFieldName,
			   void (SoloFunctionsApi::*function) (float, const float *, size_t,
			       float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api);

  string _generalLogicalFx2(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
			    float constantLower, float constantUpper,
			   size_t clip_gate, float bad_data_value,
			   string maskFieldName,
			    void (SoloFunctionsApi::*function) (float, float, const float *, size_t,
			       float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api);

  string _generalThresholdFx(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		      float threshold,
		      int first_good_gate, string threshold_field,
		      float threshold_bad_data_value,
		      size_t clip_gate, float bad_data_value,
		      string maskFieldName,
		      void (SoloFunctionsApi::*function) (float, int,  const float*, 
							  const float *, size_t, float *, float, float, size_t,
							  bool *, const bool *), SoloFunctionsApi& api);

};


#endif
