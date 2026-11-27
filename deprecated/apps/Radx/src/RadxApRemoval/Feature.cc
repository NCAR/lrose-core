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
/**
 *
 * @file Feature.cc
 *
 * @class Feature
 *
 * Feature stores the values of the feature fields.
 *  
 * @date 9/18/2002
 *
 */

using namespace std;

#include <limits.h>
#include <float.h>

#include "Feature.hh"
#include "RadxApRemoval.hh"
#include "FilterBeamInfo.hh"

//
// Constants
//
const int Feature::RADAR_PARAMS_COUNT  = 10;
const int Feature::RADAR_SUMMARY_COUNT = 30;

// Don't include FINAL and FINALC interest fields, which are not feature
// fields.
const int Feature::N_FEATURE_FIELDS = FilterBeamInfo::N_INTEREST_FIELDS - 2;

// Assume data range of 0 - 250
const float Feature::TDBZ_FIELD_SCALE = 0.004;
const float Feature::TDBZ_FIELD_BIAS = 0.0;

// Assume data range of -100 - 100
const float Feature::GDZ_FIELD_SCALE = 0.0035;
const float Feature::GDZ_FIELD_BIAS = -100.0;

// Assume data range of -100 - 100
const float Feature::MVE_FIELD_SCALE = 0.0035;
const float Feature::MVE_FIELD_BIAS = -100.0;

// Assume data range of 0 - 50
const float Feature::MSW_FIELD_SCALE = 0.0008;
const float Feature::MSW_FIELD_BIAS = 0.0;

// Assume data range of 0 - 20
const float Feature::SDVE_FIELD_SCALE = 0.00035;
const float Feature::SDVE_FIELD_BIAS = 0.0;

// Assume data range of 0 - 20
const float Feature::SDSW_FIELD_SCALE = 0.00035;
const float Feature::SDSW_FIELD_BIAS = 0.0;

// Assume data range of 0 - 100
const float Feature::SPIN_FIELD_SCALE = 0.0016;
const float Feature::SPIN_FIELD_BIAS = 0.0;

// Assume data range of -1 - 1
const float Feature::SIGN_FIELD_SCALE = 0.000035;
const float Feature::SIGN_FIELD_BIAS = -1.0;

// Assume data range of -100 - 100
const float Feature::RGDZ_FIELD_SCALE = GDZ_FIELD_SCALE;
const float Feature::RGDZ_FIELD_BIAS = GDZ_FIELD_BIAS;

// Assume data range of -100 - 100
const float Feature::SRDZ_FIELD_SCALE = GDZ_FIELD_SCALE;
const float Feature::SRDZ_FIELD_BIAS = GDZ_FIELD_BIAS;

// Assume data range of -100 - 100
const float Feature::RSINZ_FIELD_SCALE = 0.0035;
const float Feature::RSINZ_FIELD_BIAS = -100.0;

const float Feature::MISSING_VALUE = FLT_MAX;

const int Feature::SCALED_MISSING_VALUE = USHRT_MAX;
const int Feature::SCALED_MIN_VALUE = 0;
const int Feature::SCALED_MAX_VALUE = USHRT_MAX - 1;


/**
 * Constructor
 */

Feature::Feature() 
{
  _nGates = 0;
  _gateSpacing = 0.0;
  _featureQueue = NULL;
}


/**
 * Destructor
 */

Feature::~Feature() 
{
  // Clear out feature field params

  vector< DsFieldParams* > &field_params = _featureMsg.getFieldParams();
  vector< DsFieldParams* >::iterator field;

  for (field = field_params.begin(); field != field_params.end(); ++field)
    delete *field;

  field_params.erase(field_params.begin(), field_params.end());
  
  _featureFieldMap.erase(_featureFieldMap.begin(), _featureFieldMap.end());

  // Clear out feature data

  _clear();
}


/**
 * _clear()
 */

void Feature::_clear() 
{
  for (size_t i = 0; i < _featurePtrs.size(); ++i)
    delete [] _featurePtrs[i];

  _featurePtrs.erase(_featurePtrs.begin(), _featurePtrs.end());
  _azimuths.erase(_azimuths.begin(), _azimuths.end());
}


/**
 * init()
 */

void Feature::init(const double gate_spacing, DsRadarQueue *output_queue)
{
  _gateSpacing  = gate_spacing;
  _featureQueue = output_queue;
      
  // Feature field params

  DsFieldParams *fparams;
  vector< DsFieldParams* > &field_params = _featureMsg.getFieldParams();
  
  fparams = new DsFieldParams("TDBZ", "dBZ^2", 
			      TDBZ_FIELD_SCALE, TDBZ_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::TDBZ] = fparams;

  fparams = new DsFieldParams("GDZ", "dBZ", 
			      GDZ_FIELD_SCALE, GDZ_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::GDZ] = fparams;

  fparams = new DsFieldParams("MVE", "m/s", 
			      MVE_FIELD_SCALE, MVE_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::MVE] = fparams;

  fparams = new DsFieldParams("MSW", "m/s", 
			      MSW_FIELD_SCALE, MSW_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::MSW] = fparams;

  fparams = new DsFieldParams("SDVE", "m/s", 
			      SDVE_FIELD_SCALE, SDVE_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::SDVE] = fparams;

  fparams = new DsFieldParams("SDSW", "m/s", 
			      SDSW_FIELD_SCALE, SDSW_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::SDSW] = fparams;

  fparams = new DsFieldParams("SC_SPIN", "%", 
			      SPIN_FIELD_SCALE, SPIN_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::SC_SPIN] = fparams;

  fparams = new DsFieldParams("AP_SPIN", "%", 
			      SPIN_FIELD_SCALE, SPIN_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::AP_SPIN] = fparams;

  fparams = new DsFieldParams("P_SPIN", "%", 
			      SPIN_FIELD_SCALE, SPIN_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::P_SPIN] = fparams;

  fparams = new DsFieldParams("SIGN", "none", 
			      SIGN_FIELD_SCALE, SIGN_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::SIGN] = fparams;

  fparams = new DsFieldParams("RGDZ", "dBZ", 
			      RGDZ_FIELD_SCALE, RGDZ_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::RGDZ] = fparams;

  fparams = new DsFieldParams("SRDZ", "dBZ", 
			      SRDZ_FIELD_SCALE, SRDZ_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::SRDZ] = fparams;

  fparams = new DsFieldParams("RSINZ", "dBZ/km", 
			      RSINZ_FIELD_SCALE, RSINZ_FIELD_BIAS, 2,
			      SCALED_MISSING_VALUE);
  field_params.push_back(fparams);
  _featureFieldMap[FilterBeamInfo::RSINZ] = fparams;
}


/**
 * setTilt()
 */

void Feature::setTilt(const int ngts)
{
  _nGates = ngts;
  _clear();
}


/**
 * setBeam()
 */

void Feature::setBeam(const double azimuth)
{
  int total_gates = N_FEATURE_FIELDS * _nGates;
  
  ui16 *new_beam = new ui16[total_gates];
  for (int i = 0; i < total_gates; ++i)
    new_beam[i] = SCALED_MISSING_VALUE;
   
  _featurePtrs.push_back(new_beam);
  _azimuths.push_back(azimuth);
}


/**
 * setData
 */

void Feature::setData(const FilterBeamInfo::InterestType int_type,
		      const double field_val, 
		      const int ibeam, const int igate)
{
  int gate_index = igate * N_FEATURE_FIELDS + int_type;
  
  if (field_val == MISSING_VALUE)
  {
    _featurePtrs[ibeam][gate_index] = SCALED_MISSING_VALUE;
  }
  else
  {
    int scaled_value =
      (int)((field_val - _featureFieldMap[int_type]->bias) /
	    _featureFieldMap[int_type]->scale + 0.5);

    if (scaled_value < SCALED_MIN_VALUE)
      _featurePtrs[ibeam][gate_index] = SCALED_MIN_VALUE;
    else if (scaled_value > SCALED_MAX_VALUE)
      _featurePtrs[ibeam][gate_index] = SCALED_MAX_VALUE;
    else
      _featurePtrs[ibeam][gate_index] = scaled_value;
  }
}


/**
 * write()
 */

int Feature::write(const time_t start_time, const time_t end_time, 
		   const DsRadarParams &sample_params, const double elev_angle,
		   const int vol_num, const int tilt_num)
{
  if (!_featureQueue)
    return 0;
   
  int n_beams = (int) _featurePtrs.size();

  // Set up basic information about the tilt

  float target_elev = elev_angle;
  double delta_time = (double)(end_time - start_time) / (double)n_beams;

  // Set feature radar parameters

  DsRadarParams &radar_params = _featureMsg.getRadarParams();
   
  radar_params.copy(sample_params);
  radar_params.numFields = N_FEATURE_FIELDS;   
  radar_params.numGates = _nGates;
  radar_params.gateSpacing = _gateSpacing;
   
  // Put out a start of tilt flag

  POSTMSG(DEBUG, "Writing start of tilt flag to feature queue");
  _featureQueue->putStartOfTilt(tilt_num, start_time);
   
  // Construct message and write it out

  int count = 0;
  int summary_count     = 0;

  for (int iaz = 0; iaz < n_beams; ++iaz)
  {
    // Set message content

    int out_msg_content = 0;

    if (count == 0)
    {
      out_msg_content  = 0;
      out_msg_content |= DsRadarMsg::RADAR_PARAMS;
      out_msg_content |= DsRadarMsg::FIELD_PARAMS;
      out_msg_content |= DsRadarMsg::RADAR_BEAM;
    }
    else
    {
      out_msg_content = DsRadarMsg::RADAR_BEAM;
    }
      
    // Set beam header information

    DsRadarBeam &radar_beam = _featureMsg.getRadarBeam();

    radar_beam.dataTime = (time_t)(start_time + iaz * delta_time);

    radar_beam.referenceTime = radar_beam.dataTime;
    radar_beam.volumeNum = vol_num;
    radar_beam.tiltNum = tilt_num;
    radar_beam.azimuth = _azimuths[iaz];
    radar_beam.elevation = target_elev;
    radar_beam.targetElev = target_elev;

    // Load up data array

    radar_beam.loadData(_featurePtrs[iaz],
			_nGates * N_FEATURE_FIELDS * 2, 2);

    // Tell the user what we are doing

    if (summary_count > RADAR_SUMMARY_COUNT)
    {
      POSTMSG(DEBUG, "Feature: Vol Tilt El_tgt El_act     Az");
      POSTMSG(DEBUG, "         %4ld %4ld %6.2f %6.2f %6.2f",
	      (long) radar_beam.volumeNum,
	      (long) radar_beam.tiltNum,
	      (double) radar_beam.targetElev,
	      (double) radar_beam.elevation,
	      (double) radar_beam.azimuth);

      summary_count = 0;
    }
    summary_count++;

    // Write message to output queue

    if (_featureQueue->putDsMsg(_featureMsg, out_msg_content))
    {
      POSTMSG(ERROR, "Could not write to fmq" );
      return -1;
    }

    // Reset count for message content control

    count++;
    if (count >= RADAR_PARAMS_COUNT)
      count = 0;
  }
   
  // Write end of tilt flag

  POSTMSG(DEBUG, "Writing end of tilt flag to feature queue");
  _featureQueue->putEndOfTilt(tilt_num, end_time);

  // Clean up

  POSTMSG(DEBUG, "Wrote feature data for plane at %f degrees", elev_angle );
   
  return 0;
}


/**
 * putStartOfVolume()
 */

void Feature::putStartOfVolume(const time_t start_time, const int vol_num)
{
  if (_featureQueue)
  {
    POSTMSG(DEBUG, "Writing start of volume flag to feature queue");
    _featureQueue->putStartOfVolume(vol_num, start_time);
  }
}


/**
 * putEndOfVolume()
 */

void Feature::putEndOfVolume(const time_t end_time, const int vol_num)
{
  if (_featureQueue)
  {
    POSTMSG(DEBUG, "Writing end of volume flag to feature queue");
    _featureQueue->putEndOfVolume(vol_num, end_time);
  }
}



