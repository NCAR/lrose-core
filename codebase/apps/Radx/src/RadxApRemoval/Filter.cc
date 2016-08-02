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
 * @file Filter.cc
 *
 * @class Filter
 *
 * Filter handles filtering of radar data.
 *  
 * @date 9/18/2002
 *
 */

using namespace std;

#include <limits.h>

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>

#include "Filter.hh"
#include "InterestFunction.hh"
#include "ApRemoval.hh"

//
// Constants
//
const int Filter::RADAR_PARAMS_COUNT = 10;
const int Filter::RADAR_SUMMARY_COUNT = 30;

const string Filter::INTEREST_OUTPUT = "interest";
const string Filter::CONFIDENCE_OUTPUT = "confidence";


/**
 * Constructor
 */

Filter::Filter() :
  _nGates(0),
  _azRadius(0),
  _gateSpacing(0.0),
  _finalThreshold(0.0),
  _combineType(COMBINE_AND),
  _ignoreLowDbz(false),
  _lowDbzThreshold(0.0),
  _terrainOk(true),
  _terrainUseType(ALL),
  _applyConfidence(false),
  _interestQueue(0),
  _confidenceQueue(0)
{
}


/**
 * Destructor
 */

Filter::~Filter() 
{
  // Clear interest functions

  map< FilterBeamInfo::InterestType, InterestFunction* >::iterator int_iter;
  for (int_iter = _interestFunc.begin(); int_iter != _interestFunc.end();
       ++int_iter)
    delete (*int_iter).second;

  _interestFunc.erase(_interestFunc.begin(), _interestFunc.end());

  // Clear interest field params

  vector< DsFieldParams* > &field_params = _interestMsg.getFieldParams();
   
  vector< DsFieldParams* >::iterator field_iter;
  for (field_iter = field_params.begin(); field_iter != field_params.end();
       ++field_iter)
    delete *field_iter;

  field_params.erase(field_params.begin(), field_params.end() );

  // Clear confidence functions

  map< FilterBeamInfo::InterestType, InterestFunction* >::iterator conf_iter;
  for (conf_iter = _confidenceFunc.begin(); conf_iter != _confidenceFunc.end();
       ++conf_iter)
    delete (*conf_iter).second;

  _confidenceFunc.erase(_confidenceFunc.begin(), _confidenceFunc.end());

  // Clear out the interest data arrays

  _clear();
}


/**
 * _clear()
 */

void Filter::_clear() 
{
  // Clear the beam information

  vector< FilterBeamInfo* >::iterator beam;
  
  for (beam = _beamList.begin(); beam != _beamList.end(); ++beam)
    delete *beam;
  
  _beamList.erase(_beamList.begin(), _beamList.end());
}


/**
 * init()
 */

void Filter::init(const double gate_spacing, const double final_thresh,
		  const int az_radius, const TerrainType terrain_use_type,
		  DsRadarQueue* interest_queue, DsRadarQueue *confidence_queue,
		  const bool ignore_low_dbz, const double low_dbz_thresh) 
{
  // Parameters

  _gateSpacing = gate_spacing;
  _azRadius = az_radius;
  _interestQueue = interest_queue;
  _confidenceQueue = confidence_queue;
  _finalThreshold = final_thresh;
  _terrainUseType = terrain_use_type;
  _ignoreLowDbz = ignore_low_dbz;
  _lowDbzThreshold = low_dbz_thresh;
   
  // Interest field params

  DsFieldParams *fparams;
  vector< DsFieldParams* > &field_params = _interestMsg.getFieldParams();
   
  fparams = new DsFieldParams("TDBZ", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2,
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);
   
  fparams = new DsFieldParams("GDZ", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2, 
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("MVE", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("MSW", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("SDVE", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("SDSW", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("SC_SPIN", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("AP_SPIN", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("P_SPIN", "none",
			      InterestFunction::INTEREST_SCALE,  
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("SIGN", "none",
			      InterestFunction::INTEREST_SCALE,   
			      InterestFunction::INTEREST_BIAS, 2, 
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);
   
  fparams = new DsFieldParams("RGDZ", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2, 
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);
   
  fparams = new DsFieldParams("SRDZ", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2, 
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);
   
  fparams = new DsFieldParams("RSINZ", "none",
			      InterestFunction::INTEREST_SCALE, 
			      InterestFunction::INTEREST_BIAS, 2, 
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("FINAL", "none",
			      InterestFunction::INTEREST_SCALE,    
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  fparams = new DsFieldParams("FINALC", "none",
			      InterestFunction::INTEREST_SCALE,    
			      InterestFunction::INTEREST_BIAS, 2,  
			      InterestFunction::SCALED_MISSING_INTEREST);
  field_params.push_back(fparams);

  // Interest functions

  _interestFunc[FilterBeamInfo::TDBZ] = new InterestFunction();
  _interestFunc[FilterBeamInfo::GDZ] = new InterestFunction();
  _interestFunc[FilterBeamInfo::MVE] = new InterestFunction();
  _interestFunc[FilterBeamInfo::MSW] = new InterestFunction();
  _interestFunc[FilterBeamInfo::SDVE] = new InterestFunction();
  _interestFunc[FilterBeamInfo::SDSW] = new InterestFunction();
  _interestFunc[FilterBeamInfo::SC_SPIN] = new InterestFunction();
  _interestFunc[FilterBeamInfo::AP_SPIN] = new InterestFunction();
  _interestFunc[FilterBeamInfo::P_SPIN] = new InterestFunction();
  _interestFunc[FilterBeamInfo::SIGN] = new InterestFunction();
  _interestFunc[FilterBeamInfo::RGDZ] = new InterestFunction();
  _interestFunc[FilterBeamInfo::SRDZ] = new InterestFunction();
  _interestFunc[FilterBeamInfo::RSINZ] = new InterestFunction();

  // Confidence functions

  _confidenceFunc[FilterBeamInfo::TDBZ] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::GDZ] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::MVE] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::MSW] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::SDVE] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::SDSW] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::SC_SPIN] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::AP_SPIN] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::P_SPIN] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::SIGN] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::RGDZ] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::SRDZ] = new InterestFunction();
  _confidenceFunc[FilterBeamInfo::RSINZ] = new InterestFunction();
}


/**
 * setTilt()
 */

void Filter::setTilt(const int num_gates)
{
  // Initialize the tilt information

  _nGates = num_gates;
  _terrainOk = true;

  // Clear the beam information

  _clear();
}


/**
 * setBeam()
 */

void Filter::setBeam(const double azimuth)
{
  FilterBeamInfo *beam_info = new FilterBeamInfo();
  
  beam_info->setBeam(azimuth, _nGates);
  
  _beamList.push_back(beam_info);
}


/**
 * setInterestFunc()
 */

int Filter::setInterestFunc(const FilterBeamInfo::InterestType it,
			    const double x1, const double y1,
			    const double x2, const double y2,
			    const double x3, const double y3,
			    const double x4, const double y4,
			    const double x5, const double y5,
			    const double x6, const double y6,
			    const double weight)
{
  return _interestFunc[it]->setFunction(x1, y1, x2, y2, x3, y3, 
					x4, y4, x5, y5, x6, y6,
					weight);
}


/**
 * setConfidenceFunc()
 */

int Filter::setConfidenceFunc(const FilterBeamInfo::InterestType it,
			      const double x1, const double y1,
			      const double x2, const double y2,
			      const double x3, const double y3,
			      const double x4, const double y4,
			      const double x5, const double y5,
			      const double x6, const double y6,
			      const double weight)
{
  _applyConfidence = true;
   
  return _confidenceFunc[it]->setFunction(x1, y1, x2, y2, x3, y3, 
					  x4, y4, x5, y5, x6, y6,
					  weight );
}


/**
 * setTerrain()
 */

void Filter::setTerrain(const float terrain_val)
{
  _terrainOk = true;
   
  if (_terrainUseType == LAND &&
      (terrain_val == TerrainMask::WATER_VALUE ||
       terrain_val == TerrainMask::MISSING_VALUE))
    _terrainOk = false;
  else if (_terrainUseType == WATER &&
	   (terrain_val == TerrainMask::LAND_VALUE ||
	    terrain_val == TerrainMask::MISSING_VALUE))
    _terrainOk = false;
}


/**
 * calcInterest()
 */

void Filter::calcInterest(const FilterBeamInfo::InterestType int_type,
			  const double field_val,
			  const int ibeam, const int igate)
{
  // Don't calculate interest in places with the wrong terrain type

  if (!_terrainOk)
    return;
   
  // Calculate and set the interest and confidence values

  double interest_val = _interestFunc[int_type]->apply(field_val);
  double confidence_val = _confidenceFunc[int_type]->apply(field_val);

  _beamList[ibeam]->calcInterest(int_type, igate,
				 interest_val,
				 _interestFunc[int_type]->getWeight(),
				 confidence_val,
				 _confidenceFunc[int_type]->getWeight());
}


/**
 * calcFinal()
 */

void Filter::calcFinal(const int ibeam, const int igate, const float dbz_val)
{
  // Set the final interest to 0 if the DBZ value is too low

  if (_ignoreLowDbz && dbz_val < _lowDbzThreshold)
    _beamList[ibeam]->setFinal(igate, 0.0);
  else
    _beamList[ibeam]->calcFinal(igate, _applyConfidence);
}


/**
 * writeInterest()
 */

int Filter::writeInterest(const time_t start_time, const time_t end_time, 
			  const DsRadarParams& sample_params,
			  const double elev_angle, 
			  const int vol_num, const int tilt_num) 
{
  // Write the interest fields

  int interest_status = _writeInterest(_interestQueue, INTEREST_OUTPUT,
				       start_time, end_time, sample_params,
				       elev_angle, vol_num, tilt_num);
  
  // Write the confidence fields

  int confidence_status = _writeInterest(_confidenceQueue, CONFIDENCE_OUTPUT,
					 start_time, end_time, sample_params,
					 elev_angle, vol_num, tilt_num);
  
  if (interest_status == 0 && confidence_status == 0)
    return 0;
  
  return -1;
}


/**
 * _writeInterest()
 */

int Filter::_writeInterest(DsRadarQueue *queue, const string &output_type,
			   const time_t start_time, const time_t end_time, 
			   const DsRadarParams& sample_params,
			   const double elev_angle, 
			   const int vol_num, const int tilt_num) 
{
  // If the interest queue pointer is set to null, don't do anything

  if (!queue)
    return 0;
   
  // Set up basic information about the tilt

  double delta_time =
    (double)(end_time - start_time) / (double)_beamList.size();

  // Set interest radar parameters

  DsRadarParams &radar_params = _interestMsg.getRadarParams();
   
  radar_params.copy(sample_params);
  radar_params.numFields = FilterBeamInfo::N_INTEREST_FIELDS;
  radar_params.numGates = _nGates;
  radar_params.gateSpacing = _gateSpacing;

  // Put out a start of tilt flag

  POSTMSG(DEBUG, "Writing start of tilt flag to %s queue",
	  output_type.c_str());
  queue->putStartOfTilt(tilt_num, start_time);
   
  // Construct message and write it out

  int count = 0;
  int summary_count = 0;

  for (size_t iaz = 0; iaz < _beamList.size(); ++iaz)
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

    DsRadarBeam &radar_beam = _interestMsg.getRadarBeam();
      
    radar_beam.dataTime = (time_t)(start_time + iaz * delta_time);
    radar_beam.referenceTime = radar_beam.dataTime;
    radar_beam.volumeNum = vol_num;
    radar_beam.tiltNum = tilt_num;
    radar_beam.azimuth = _beamList[iaz]->getAzimuth();
    radar_beam.elevation = elev_angle;
    radar_beam.targetElev = elev_angle;

    // Load up data array

    ui16 *scaled_data;
      
    if (output_type == INTEREST_OUTPUT)
      scaled_data = _beamList[iaz]->getScaledInterest();
    else if (output_type == CONFIDENCE_OUTPUT)
      scaled_data = _beamList[iaz]->getScaledConfidence();

    radar_beam.loadData(scaled_data,
			_nGates * FilterBeamInfo::N_INTEREST_FIELDS * 2,
			2 );

    // Clean up
    
    delete [] scaled_data;

    // Tell the user what we are doing

    if (summary_count > RADAR_SUMMARY_COUNT)
    {
      POSTMSG(DEBUG, "%s: Vol Tilt El_tgt El_act     Az",
	      output_type.c_str());
      POSTMSG(DEBUG, "         %4ld %4ld %6.2f %6.2f %6.2f",
	      (long)radar_beam.volumeNum,
	      (long)radar_beam.tiltNum,
	      (double)radar_beam.targetElev,
	      (double)radar_beam.elevation,
	      (double)radar_beam.azimuth);

      summary_count = 0;
    }

    summary_count++;

    if (queue->putDsMsg(_interestMsg, out_msg_content))
    {
      POSTMSG(ERROR, "Could not write to fmq");
      return -1;
    }

    // Reset count for message content control

    ++count;
    if (count >= RADAR_PARAMS_COUNT)
    {
      count = 0;
    }
  }

  // Put out the end of tilt flag

  POSTMSG(DEBUG, "Writing end of tilt flag to %s queue",
	  output_type.c_str());
  queue->putEndOfTilt(tilt_num, end_time);
  
  POSTMSG(DEBUG, "Wrote %s data for plane at %f degrees", 
	  output_type.c_str(), elev_angle);
   
  return 0;
}


/**
 * putStartOfVolume()
 */

void Filter::putStartOfVolume(const time_t start_time, const int vol_num)
{
  // Write the end of volume to the interest queue

  if (_interestQueue)
  {
    POSTMSG(DEBUG, "Writing start of volume flag to interest queue");
    _interestQueue->putStartOfVolume(vol_num, start_time);
  }
   
  // Write the end of volume to the confidence queue

  if (_confidenceQueue)
  {
    POSTMSG(DEBUG, "Writing start of volume flag to confidence queue");
    _confidenceQueue->putStartOfVolume(vol_num, start_time);
  }
}


/**
 * putEndOfVolume()
 */

void Filter::putEndOfVolume(const time_t end_time, const int vol_num)
{
  // Write the end of volume to the interest queue

  if (_interestQueue)
  {
    POSTMSG(DEBUG, "Writing end of volume flag to interest queue");
    _interestQueue->putEndOfVolume(vol_num, end_time);
  }

  // Write the end of volume to the confidence queue

  if (_confidenceQueue)
  {
    POSTMSG(DEBUG, "Writing end of volume flag to confidence queue");
    _confidenceQueue->putEndOfVolume(vol_num, end_time);
  }
}
