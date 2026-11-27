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
 * @file RadarTilt.cc
 *
 * @class RadarTilt
 *
 * RadarTilt manages the processing of a single radar tilt.
 *  
 * @date 4/5/2002
 *
 */

using namespace std;

#include <string>
#include <limits.h>
#include <float.h>
#include <rapmath/math_macros.h>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>

#include "RadarTilt.hh"
#include "Feature.hh"
#include "InterestFunction.hh"
#include "RadarMsg.hh"
#include "TerrainMask.hh"
#include "ApRemoval.hh"

//
// Constants
//
const int RadarTilt::AZ_INDEX_LEN = 4000;
const int RadarTilt::AZ_TOL = 5;
const int RadarTilt::RADAR_SUMMARY_COUNT = 30;
const float RadarTilt::MIN_GOOD_FRACTION = 0.25;
const float RadarTilt::GATE_SPACING_TOL = 0.0001;
const double RadarTilt::MISSING_MED_VALUE = DBL_MAX;


/**
 * Constructor
 */

RadarTilt::RadarTilt(DsRadarQueue& output_queue,
		     const int dbz_az_radius, const int vel_az_radius,
		     const int dbz_gate_radius, const int vel_gate_radius,
		     const double gate_spacing, const int max_gates,
		     const char* dbz_field_name, const char* vel_field_name, 
		     const char* sw_field_name, const double sc_spin_thresh, 
		     const double ap_spin_thresh, const double p_spin_thresh,
		     const double delta_az, const double slant_range_dist,
		     TerrainMask& terrain_mask, 
		     const FilterList &filter_list,
		     Feature& feature_fields, 
		     InterestFunction& range_wt_func,
		     vector< string* >& filter_field_names) :
  _firstBeamInTilt(true),
  _empty(true),
  _filtered(false),
  _initFailed(false),
  _startOfVolume(false),
  _endOfVolume(false),
  _dataSet(false),
  _dbzFieldIndex(-1),
  _velFieldIndex(-1),
  _swFieldIndex(-1),
  _dbzFieldName(dbz_field_name),
  _velFieldName(vel_field_name),
  _swFieldName(sw_field_name),
  _upperDbzExists(false),
  _startTime(0),
  _endTime(0),
  _nFields(0),
  _maxGates(max_gates),
  _byteWidth(0),
  _nBeams(0),
  _gateSpacing(gate_spacing),
  _deltaAzimuth(delta_az),
  _startRange(0.0),
  _elevAngle(-1.0),
  _noiseFloor(0.0),
  _radarConstant(0.0),
  _tiltNum(-1),
  _volNum(-1),
  _sampleParams(0),
  _terrainNGates(terrain_mask.getNumGates()),
  _terrainVals(terrain_mask.getMask()),
  _filterFieldNames(filter_field_names),
  _beamIndex(0),
  _maxAzRadius(MAX(dbz_az_radius, vel_az_radius)),
  _dbzAzRadius(dbz_az_radius),
  _velAzRadius(vel_az_radius),
  _dbzGateRadius(dbz_gate_radius),
  _velGateRadius(vel_gate_radius),
  _slantRangeNGates((int)(slant_range_dist / _gateSpacing + 0.5)),
  _scSpinThreshold(sc_spin_thresh),
  _apSpinThreshold(ap_spin_thresh),
  _pSpinThreshold(p_spin_thresh),
  _azimuthDiffTol(1.5 * delta_az),
  _azIndices(new int[AZ_INDEX_LEN]),
  _filterList(filter_list),
  _rangeWtFunc(range_wt_func),
  _featureFields(feature_fields),
  _summaryCount(RADAR_SUMMARY_COUNT + 1),
  _outputQueue(output_queue)
{
   for (int i = 0; i < AZ_INDEX_LEN; ++i)
     _azIndices[i] = -1;
}


/**
 * Destructor
 */

RadarTilt::~RadarTilt() 
{
   clear();

   delete[] _azIndices;
}


/**
 * clear()
 */

void RadarTilt::clear() 
{
  // Clear message list

  vector< RadarMsg* >::iterator msg_it;
   
  for (msg_it = _radarMsgs.begin(); msg_it != _radarMsgs.end(); ++msg_it)
    delete *msg_it;

  _radarMsgs.erase(_radarMsgs.begin(), _radarMsgs.end());

  // Clear input field vector

  vector< DsFieldParams* >::iterator field_it;

  for (field_it = _inputFieldParams.begin(); 
       field_it != _inputFieldParams.end(); ++field_it)
    delete *field_it;

  _inputFieldParams.erase(_inputFieldParams.begin(), _inputFieldParams.end());

  // Clear out the data arrays

  _dataPtrs.clear();

  for (size_t i = 0; i < _medianVel.size(); ++i)
    delete [] _medianVel[i];

  for (size_t i = 0; i < _medianSw.size(); ++i)
    delete [] _medianSw[i];

  _medianVel.erase(_medianVel.begin(), _medianVel.end());
  _medianSw.erase(_medianSw.begin(), _medianSw.end());

  _beamIndex = 0;

  // Clear the elevation angles and azimuths

  _elevAngles.erase(_elevAngles.begin(), _elevAngles.end());
  _azimuths.erase(_azimuths.begin(), _azimuths.end());
   
  // Reset the azimuth indeces

  for (int i = 0; i < AZ_INDEX_LEN; ++i)
    _azIndices[i] = -1;

  // Reset the number of beams

  _nBeams = 0;

  // Reset the indices

  _dbzFieldIndex = -1;
  _velFieldIndex = -1;
  _swFieldIndex  = -1;

  // Reset the elevation angle and tilt number

  _tiltNum = -1;
  _elevAngle = -1.0;

  // Set flags 

  _firstBeamInTilt = true;
  _empty = true;
  _filtered = false;
  _initFailed = false;
  _startOfVolume = false;
  _endOfVolume = false;
  _dataSet = false;
}


/**
 * _resetDataArrays()
 */

void RadarTilt::_resetDataArrays() 
{
  // Set up the first _maxAzRadius beams for the data arrays,
  // so that things are in the right order

  for (int i = 0; i < _maxAzRadius; ++i)
  {
    DsBeamData newBeam(_byteWidth, _nFields, _maxGates, _dataParams);
    newBeam.fill_with_missing();
    _dataPtrs.push_back(newBeam);

    double *newMedVel = new double[_maxGates];
    double *newMedSw = new double[_maxGates];

    for (int igate = 0; igate < _maxGates; ++igate)
    {
      newMedVel[igate] = MISSING_MED_VALUE;
      newMedSw[igate] = MISSING_MED_VALUE;
    }
 
   _medianVel.push_back(newMedVel);
    _medianSw.push_back(newMedSw);

    _elevAngles.push_back(0.0);
    _azimuths.push_back(0.0);

    _beamIndex++;
  } /* endfor - i */

}


/**
 * _initMsgs()
 */

void RadarTilt::_initMsgs(RadarMsg& radar_msg)
{
  // The beam will not be passed to this function until all
  // the parameters have been set, so we are assured that we
  // have radar and field parameters at this point.  Note that
  // we are assuming that the parameters set below do not 
  // change over the tilt, including the scale and bias for
  // each of the fields.  

  // Set general radar parameters

  DsRadarParams &radar_params = radar_msg.getRadarParams();

  // Set number of fields

  _nFields = radar_params.numFields;

  // Set the elevation angle and the volume number

  DsRadarBeam &radar_beam = radar_msg.getRadarBeam();

  _elevAngle = radar_beam.elevation;
  _tiltNum = radar_beam.tiltNum;
  _volNum = radar_beam.volumeNum;

  PMU_auto_register("Set data for this tilt");

  // Set range to the first gate
  
  _startRange = radar_params.startRange;

  // Set the sample params for later use

  _sampleParams = &radar_params;
   
  // Set up the list of field parameters and build Beam parms.
  // Check for consistent bytewidth as we go.

  _dataParams.clear();

  vector< DsFieldParams* > input_fields = radar_msg.getFieldParams();
  DsFieldParams *new_field;
      
  _byteWidth = 0;
  for (int i = 0; i < _nFields; ++i)
  {
    new_field = new DsFieldParams(*(input_fields[i]));
    _inputFieldParams.push_back(new_field);
    if (i == 0)
    {
      _byteWidth = new_field->byteWidth;
    }
    else
    {
      if (new_field->byteWidth != _byteWidth)
      {
	POSTMSG(ERROR, "Unequal byte widths is not implemented");
	_initFailed = true;
	return;
      }
    }
	  
    // create and store new parms. Use FLT_MAX as the float missing val.

    DsBeamDataFieldParms bp(new_field->scale, new_field->bias,
			    DsBeamData::is_floating_data(_byteWidth),
			    new_field->missingDataValue, FLT_MAX);
    _dataParams.push_back(bp);

    // Set field indices

    if (new_field->name == _dbzFieldName)
      _dbzFieldIndex = i;
    if (new_field->name == _velFieldName)
      _velFieldIndex = i;
    if (new_field->name == _swFieldName)
      _swFieldIndex = i;
  } /* endfor - i */

  // Check to make sure we have at least one data field

  if (_dbzFieldIndex < 0 && _velFieldIndex < 0 && _swFieldIndex < 0)
  {
    POSTMSG(ERROR, "All input fields missing");
    _initFailed = true;
    return;
  }

  if (_byteWidth != 1 && _byteWidth != 2 && _byteWidth != 4)
  {
    POSTMSG(ERROR, "Bytewidth not 1,2, or 4 not implmented");
    _initFailed = true;
    return;
  }

  // Set stuff to calculate default values later

  _noiseFloor = radar_params.receiverMds;
  _radarConstant = radar_params.radarConstant;

  // Check terrain data

  if (_terrainVals != 0)
  {
    if (_maxGates > _terrainNGates)
    {
      POSTMSG(ERROR, "Terrain data will not work for this tilt");
      _initFailed = true;
      return;
    }
  }

  // Get the start time for the tilt

  _startTime = radar_beam.dataTime;

  // Check gate spacing

  if (fabs(radar_params.gateSpacing - _gateSpacing) > GATE_SPACING_TOL)
  {
    POSTMSG(ERROR, "Gate spacing from message, %f, doesn't match "
	    "expected gate spacing, %f", radar_params.gateSpacing,
	    _gateSpacing);
    POSTMSG(ERROR, "Diff = %f, tolerance = %f",
	    fabs(radar_params.gateSpacing - _gateSpacing),
	    GATE_SPACING_TOL);
    _initFailed = true;
    return;
  }

  // Reset the data

  PMU_auto_register("Reset data arrays");
  _resetDataArrays();
}


/**
 * addMsg()
 */

int RadarTilt::addMsg(DsRadarMsg& radar_msg, const int content)
{
  // Get the number of gates for this beam message.  The number of gates can
  // change for each beam.

  int num_gates  = radar_msg.getRadarParams().numGates;

  // Create new radar message

  RadarMsg *new_msg = new RadarMsg(radar_msg, content);

  PMU_auto_register("Adding message");
   
  // Tell the user what we are doing

  if (content & DsRadarMsg::RADAR_BEAM)
  {
    // Initialize the parameters

    if (_firstBeamInTilt)
    {
      _initMsgs(*new_msg);
      _firstBeamInTilt = false;
    }

    if (_initFailed)
    {
      delete new_msg;
      return -1;
    }

    DsRadarBeam &radar_beam = new_msg->getRadarBeam();

    // Tell the user what we are doing

    if (_summaryCount > RADAR_SUMMARY_COUNT)
    {
      POSTMSG(INFO, " Adding:  Vol Tilt El_tgt El_act     Az");
      POSTMSG(INFO, "          %4ld %4ld %6.2f %6.2f %6.2f",
	      (long) radar_beam.volumeNum,
	      (long) radar_beam.tiltNum,
	      (double) radar_beam.targetElev,
	      (double) radar_beam.elevation,
	      (double) radar_beam.azimuth );

      _summaryCount = 0;
    }
    _summaryCount++;
      
    // Check data length

    if (radar_beam.byteWidth != _byteWidth)
    {
      POSTMSG(ERROR, "Input data must all have byte width %d",_byteWidth);
      POSTMSG(ERROR, "Your new data has a byte width of %d",
	      radar_beam.byteWidth);
      return -1;
    }
      
    // Check the maximum number of gates

    if (num_gates > _maxGates)
    {
      POSTMSG(ERROR, "Beam has %d gates, but max_num_gates was set to %d",
	      num_gates, _maxGates);
      _initFailed = true;
      return -1;
    }

    if (radar_beam.dataLen() != num_gates * _nFields * _byteWidth)
    {
      POSTMSG(ERROR, "Different number of gates or fields than expected");
      POSTMSG(ERROR, "num_gates = %d, nFields = %d, dataLen = %d",
	      num_gates, _nFields, radar_beam.dataLen());
      return -1;
    }

    // We get some bad azimuth values, so check azimuth value
    // before processing.

    if (radar_beam.azimuth < 0.0 || radar_beam.azimuth > 360.0)
    {
      POSTMSG(WARNING, "Bad azimuth value (%d) value for beam -- skipping",
	      radar_beam.azimuth);
      return -1;
    }
      
    // Copy the data over into the vector 
    
    DsBeamData new_beam(_byteWidth, _nFields, _maxGates, _dataParams);
    new_beam.copy(radar_beam);
    _dataPtrs.push_back(new_beam);

    // Keep track of the azimuth

    int az_index = AZ_TOL + (int)(radar_beam.azimuth * 10.0 + 0.5);
      
    _azIndices[az_index] = _beamIndex;
    _beamIndex++;

    // Keep track of the elevation angle at this beam

    _elevAngles.push_back(radar_beam.elevation);

    // Keep track of the azimuth of this beam

    _azimuths.push_back(radar_beam.azimuth);

    _endTime = radar_beam.dataTime;
    _empty = false;
    
  }

  // Tell the user about any flags we found.  Note that the
  // message can contain both flags and beam data, so we cannot
  // use an else if here

  if (content & DsRadarMsg::RADAR_FLAGS)
  {
    DsRadarFlags &radarFlags = radar_msg.getRadarFlags();
      
    if (radarFlags.endOfVolume)
    {
      POSTMSG(INFO, "Got end of volume flag");
      _endOfVolume = true;
    }

    if (radarFlags.startOfVolume)
    {
      POSTMSG(INFO, "Got start of volume flag");
      _startOfVolume = true;
    }

    if (radarFlags.endOfTilt)
      POSTMSG(INFO, "Got end of tilt flag");

    if (radarFlags.startOfTilt)
      POSTMSG(INFO, "Got start of tilt flag");

  }

  // Have to make sure both of these are not true, because
  // a single message can be just a radar beam, just flags
  // or both.  We only want to print this message if for
  // some reason we are getting a message that is neither
  // of those things.

  if (!(content & DsRadarMsg::RADAR_BEAM) && 
      !(content & DsRadarMsg::RADAR_FLAGS))
    POSTMSG(DEBUG, "No beam data - adding message");

  // Add the message to the list

  _radarMsgs.push_back( new_msg );

  return 0;
}


/**
 * setData()
 */

void RadarTilt::setData() 
{ 
  // If there is no data or this has already been done, we're done

  if (_empty || _dataSet)
    return;

  // Tell the user what we are doing

  POSTMSG(DEBUG, "Setting data for elevation = %f\n", _elevAngle);

  // Set the number of beams

  _nBeams = (int)_dataPtrs.size() - _maxAzRadius;

  // Handle data wrapping -
  //   Copy the data into these places in the array, that way we
  //   won't use filtered data in the sums and we don't have to
  //   worry about the 0/360 line.

  for (int i = 0; i < _maxAzRadius; ++i)
  {
    _dataPtrs[i].copy_contents(_dataPtrs[_nBeams+i]);
    DsBeamData new_beam(_byteWidth, _nFields, _maxGates, _dataParams);
    new_beam.copy_contents(_dataPtrs[i+_maxAzRadius]);
    _dataPtrs.push_back(new_beam);

    _elevAngles[i] = _elevAngles[_nBeams+i];
    _elevAngles.push_back(_elevAngles[i+_maxAzRadius]);

    _azimuths[i] = _azimuths[_nBeams+i];
    _azimuths.push_back(_azimuths[i+_maxAzRadius]);
  } /* endfor - i */

  // Calculate median data

  PMU_auto_register("Get median vel data");
  if (_velFieldIndex >= 0)
    _medianFilter(_velFieldIndex, _medianVel);

  PMU_auto_register("Get median sw data");
  if (_swFieldIndex >= 0)
    _medianFilter(_swFieldIndex, _medianSw);

  // Set default values for these

  _upperDbzExists = false;

  // Make sure we don't do this again for the same tilt

  _dataSet = true;
}


/**
 * _medianFilter()
 */

void RadarTilt::_medianFilter(const size_t field_index,
			      vector< double* > &median_vector) 
{
  // We're done if we don't have any velocity or sw data

  if (_velFieldIndex < 0 && _swFieldIndex < 0)
    return;
  
  // Set up arrays to use for median calculation

  double *median_array = new double[(2*_velAzRadius+1) * (2*_velGateRadius+1)];

  // Find median fields

  for (int ibeam = 0; ibeam < _nBeams; ++ibeam)
  {
    double *median_vals = new double[_maxGates];

    int d_beam_index = ibeam + _maxAzRadius;

    for (int igate = 0; igate < _maxGates; ++igate)
    {
      // Initialize the counts

      int count = 0;
         
      // Set up arrays for median calculations

      for (int i = d_beam_index - _velAzRadius; 
	   i <= d_beam_index + _velAzRadius; ++i)
      {
	// Initialize values

	double value = FLT_MAX;

	// Make sure we are not going out of bounds in the gate
	// dimension.  We took care of data wrapping for the
	// azimuth dimension only.  We don't want to wrap data
	// in the gate dimension.

	int start_gate = MAX(0, igate - _velGateRadius);
	int end_gate = MIN(igate + _velGateRadius, _maxGates - 1);

	// Do the calculations over the gates for the current
	// beam

	for (int j = start_gate; j <= end_gate; ++j)
	{
	  // Get values

	  value =
	    (double)_dataPtrs[i].get_scaled_value(field_index, j);
               
	  if (value != FLT_MAX)
	    median_array[count++] = value;
	} /* endfor - j */
	
      } /* endfor - i */

      if (count > 0)
	median_vals[igate] = _calcMedian(median_array, count);
      else
	median_vals[igate] = MISSING_MED_VALUE;
    } /* endfor - igate */

    median_vector.push_back(median_vals);
  } /* endfor - ibeam */

  delete [] median_array;

  // Handle data wrapping

  for (int i = 0; i < _maxAzRadius; ++i)
  {
    memcpy((void *)median_vector[i], (void *)median_vector[_nBeams+i],
	   sizeof(double) * _maxGates);

    double *new_beam = new double[_maxGates];
    memcpy((void *)new_beam, (void *)median_vector[i+_maxAzRadius], 
	   sizeof(double) * _maxGates);
    median_vector.push_back(new_beam);
  } /* endfor - i */
}


/**
 * _calcMedian()
 */

double RadarTilt::_calcMedian( double* val_array, const int count) const
{
  qsort((void *)val_array, count, sizeof(double), &_doubleCompare);
   
  if (count % 2 == 0)
    return (val_array[count/2 - 1] + val_array[count/2]) / 2.0;

  return val_array[(count-1) / 2];
}


/**
 * getAzimuthIdex()
 */

int RadarTilt::getAzimuthIdex(const double azimuth)
{
  int i = 0;
  int az_index = AZ_TOL + (int)(azimuth * 10.0 + 0.5);
  int lookup_index = _azIndices[az_index];
   
  while (lookup_index == -1 && i < AZ_TOL)
  {
    lookup_index = _azIndices[az_index-i];
    if (lookup_index == -1)
      lookup_index = _azIndices[az_index+i];

    i++;
  }

  return lookup_index;
}


/**
 * getDbzValF()
 */

float RadarTilt::getDbzValF(const int az_index, const int igate) const
{
  if (az_index < 0 || az_index > (int)_dataPtrs.size() ||
      igate < 0 || igate > _maxGates)
    return FLT_MAX;

  return _dataPtrs[az_index].get_scaled_value(_dbzFieldIndex, igate);
}


/**
 * getBeamElev()
 */

double RadarTilt::getBeamElev(const int ibeam) const
{
  if (ibeam < 0 || ibeam >= (int)_elevAngles.size())
    return -1.0;

  return _elevAngles[ibeam];
}
  

/**
 * filter()
 */

int RadarTilt::filter(RadarTilt* upper_tilt)
{
  // Set filter and feature objects for this tilt

  _filterList.setTilt(_maxGates);
  _featureFields.setTilt( _maxGates );

  // Clear out the filtering field info

  _filterList.clearFieldInfo();

  // Set field indices and missing values
  // for the fields the user requested us to
  // filter
  
  for (int i = 0; i < _nFields; ++i)
  {
    DsFieldParams *current_field = _inputFieldParams[i];
    for (size_t j = 0; j < _filterFieldNames.size(); ++j)
    {
      if (current_field->name == *(_filterFieldNames[j]))
	_filterList.setFieldInfo(i, current_field->missingDataValue);
    } /* endfor - j */
  } /* endfor - i */

  // If the tilt is empty we're done

  if (_empty)
    return 0;
   
  POSTMSG(INFO, "Filtering %f degree tilt", _elevAngle);

  // Set the data for this tilt, if it hasn't already been done

  setData();
 
  // Get missing value, scale, bias and elevation for dbz in tilt above

  if (upper_tilt != 0 && upper_tilt->dbzExists())
  {
    upper_tilt->setData();
    _upperDbzExists = true;
  }
   
  // Calculate interest values at each gate

  for (int ibeam = 0; ibeam < _nBeams; ++ibeam)
  {
    PMU_auto_register("Calculate interest values for each beam");

    int d_beam_index = ibeam + _maxAzRadius;

    // Get the azimuth

    double current_az = _azimuths[d_beam_index];
    int terrain_ibeam = (int)(current_az / _deltaAzimuth);

    // Set up a new beam in the feature object and filter
    // objects

    _featureFields.setBeam(current_az);
    _filterList.setBeam(current_az);

    for (int igate = 0; igate < _maxGates; ++igate)
    {
      // Initialize the counts

      int vel_count = 0;
      int sw_count = 0;
      int dbz_diff_count = 0;
      int sc_dbz_thresh_count = 0;
      int ap_dbz_thresh_count = 0;
      int p_dbz_thresh_count = 0;
      int total_dbz_gates = 0;
      int total_vel_gates = 0;
      int total_sw_gates = 0;
      
      // Initialize the sums

      double tdbz_sum = 0.0;
      double sign_sum = 0.0;
      double vel_sum = 0.0;
      double sw_sum = 0.0;
      double vel_sq_sum = 0.0;
      double sw_sq_sum = 0.0;

      // Calculate sums involving dbz
      //   Data wrapping around the 0/360 line was taken care of
      //   already

      if (_dbzFieldIndex >= 0)
      {
	for (int i = d_beam_index - _dbzAzRadius;
	     i <= d_beam_index + _dbzAzRadius; ++i)
	{
	  // If the azimuth differences are too great,
	  // it means we probably have a missing beam
	  // in here - don't do anything

	  if (fabs(current_az - _azimuths[i]) > _azimuthDiffTol)
	    continue;

	  // Initialize values

	  double prev_dbz = FLT_MAX;

	  // Make sure we are not going out of bounds in the gate
	  // dimension.  We took care of data wrapping for the
	  // azimuth dimension only.  We don't want to wrap data
	  // in the gate dimension.
	  
	  int start_gate = MAX(0, igate - _dbzGateRadius);
	  int end_gate = MIN(igate + _dbzGateRadius, _maxGates - 1);

	  // Do the calculations over the gates for the current
	  // beam

	  bool first_gate = true;
	  bool ap_found_first_delta = false;
	  bool sc_found_first_delta = false;
	  bool p_found_first_delta = false;
	  bool ap_last_spin_positive = false;
	  bool sc_last_spin_positive = false;
	  bool p_last_spin_positive = false;
               
	  for (int j = start_gate; j <= end_gate; ++j)
	  {
	    // Get values

	    float dbz_val = _dataPtrs[i].get_scaled_value(_dbzFieldIndex, j);

	    // Do sums involving dbz

	    if (dbz_val != FLT_MAX)
	    {
	      if (prev_dbz != FLT_MAX)
	      {
		dbz_diff_count++;

		// Spin

		_spinEval(prev_dbz, dbz_val, _scSpinThreshold,
			  first_gate, sc_last_spin_positive,
			  sc_found_first_delta,
			  sc_dbz_thresh_count);

		_spinEval(prev_dbz, dbz_val, _apSpinThreshold,
			  first_gate, ap_last_spin_positive,
			  ap_found_first_delta,
			  ap_dbz_thresh_count);

		_spinEval(prev_dbz, dbz_val, _pSpinThreshold,
			  first_gate, p_last_spin_positive,
			  p_found_first_delta,
			  p_dbz_thresh_count);

		// Texture

		double dbz_diff = dbz_val - prev_dbz;
		tdbz_sum += dbz_diff * dbz_diff;

		// Sign

		if (dbz_diff > 0)
		  sign_sum += 1;
		else if (dbz_diff < 0)
		  sign_sum -= 1;

	      }
	    }

	    // Set the previous dbz values for next pass

	    prev_dbz = dbz_val;

	    // We are no longer on the first gate

	    first_gate = false;

	    // Keep track of the total number of gates

	    total_dbz_gates++;
	    
	  } /* endfor - j */
	  
	} /* endfor - i */
	
      }

      // Calculate sums

      if (_velFieldIndex >= 0)
      {
	for (int i = d_beam_index - _velAzRadius; 
	     i <= d_beam_index + _velAzRadius; ++i)
	{
	  // If the azimuth differences are too great,
	  // it means we probably have a missing beam
	  // in here - don't do anything

	  if (fabs(current_az - _azimuths[i]) > _azimuthDiffTol)
	    continue;

	  // Make sure we are not going out of bounds in the gate
	  // dimension.  We took care of data wrapping for the
	  // azimuth dimension only.  We don't want to wrap data
	  // in the gate dimension.
	  
	  int start_gate = MAX(0, igate - _velGateRadius);
	  int end_gate = MIN(igate + _velGateRadius, _maxGates - 1);

	  // Do the calculations over the gates for the current
	  // beam

	  for (int j = start_gate; j <= end_gate; ++j)
	  {
	    // Get values

	    double vel_value = _medianVel[i][j];
               
	    // Do sums

	    if (vel_value != MISSING_MED_VALUE)
	    {
	      vel_sum += vel_value;
	      vel_sq_sum += vel_value * vel_value;
	      vel_count++;
	    }
               
	    total_vel_gates++;
	  
	  } /* endfor - j */
	} /* endfor - i */
      }
      
      // Calculate sw sums

      if (_swFieldIndex >= 0)
      {
	for (int i = d_beam_index - _velAzRadius; 
	     i <= d_beam_index + _velAzRadius; ++i)
	{
	  // If the azimuth differences are too great,
	  // it means we probably have a missing beam
	  // in here - don't do anything

	  if (fabs(current_az - _azimuths[i]) > _azimuthDiffTol)
	    continue;

	  // Make sure we are not going out of bounds in the gate
	  // dimension.  We took care of data wrapping for the
	  // azimuth dimension only.  We don't want to wrap data
	  // in the gate dimension.
	  
	  int start_gate = MAX(0, igate - _velGateRadius);
	  int end_gate = MIN(igate + _velGateRadius, _maxGates - 1);

	  // Do the calculations over the gates for the current
	  // beam

	  for (int j = start_gate; j <= end_gate; ++j)
	  {
	    // Get values

	    double sw_value = _medianSw[i][j];
               
	    // Do sums

	    if (sw_value != MISSING_MED_VALUE)
	    {
	      sw_sum += sw_value;
	      sw_sq_sum += sw_value * sw_value;
	      sw_count++;
	    }

	    total_sw_gates++;
	  
	  } /* endfor - j */
	} /* endfor - i */
      }
      
      // Find the terrain value here, if possible

      if (_terrainVals != 0)
      {
	float terrain_val = 
	  _terrainVals[terrain_ibeam*_terrainNGates + igate];
            
	_filterList.setTerrain(terrain_val);
      }

      // Sums are done, calculate feature and interest values

      double field_val;

      // GDZ fields

      if (upper_tilt != 0 && _dbzFieldIndex >= 0)
      {
	int upper_beam_index = upper_tilt->getAzimuthIdex(current_az);
	_calcGdzFields(upper_tilt, ibeam, upper_beam_index, igate);
      }
         
      // Dbz difference related fields

      if (dbz_diff_count > total_dbz_gates * MIN_GOOD_FRACTION)
      {
	// TDBZ

	field_val = tdbz_sum / dbz_diff_count;
	_calcInterest(FilterBeamInfo::TDBZ, field_val, ibeam, igate);
            
	// Sea clutter SPIN - SC_SPIN

	field_val = 100.0 * (float)sc_dbz_thresh_count / (float)dbz_diff_count;
	_calcInterest(FilterBeamInfo::SC_SPIN, field_val, ibeam, igate);
            
	// AP SPIN - AP_SPIN

	field_val = 100.0 * (float)ap_dbz_thresh_count / (float)dbz_diff_count;
	_calcInterest(FilterBeamInfo::AP_SPIN, field_val, ibeam, igate);
            
	// Precip SPIN - P_SPIN

	field_val = 100.0 * (float)p_dbz_thresh_count / (float)dbz_diff_count;
	_calcInterest(FilterBeamInfo::P_SPIN, field_val, ibeam, igate);
            
	// SIGN

	field_val = sign_sum / dbz_diff_count;
	_calcInterest(FilterBeamInfo::SIGN, field_val, ibeam, igate);
	
      }
           
      // Velocity related fields

      if (_velFieldIndex >= 0)
      {
	if (_medianVel[ibeam][igate] != MISSING_MED_VALUE)
	{
	  // MVE

	  field_val = _medianVel[ibeam][igate];
	  _calcInterest(FilterBeamInfo::MVE, field_val, ibeam, igate);
	}
            
	if (vel_count > total_vel_gates * MIN_GOOD_FRACTION)
	{ 
	  double vel_avg = vel_sum / vel_count;

	  // SDVE

	  field_val = sqrt((vel_sq_sum / vel_count) - (vel_avg * vel_avg));
	  _calcInterest(FilterBeamInfo::SDVE, field_val, ibeam, igate);
	}
      }
      
      // Spectrum width related fields

      if (_swFieldIndex >= 0)
      {
	if (_medianSw[ibeam][igate] != MISSING_MED_VALUE)
	{
	  // MSW

	  field_val = _medianSw[ibeam][igate];
	  _calcInterest(FilterBeamInfo::MSW, field_val, ibeam, igate);
	}
            
	if (sw_count > total_sw_gates * MIN_GOOD_FRACTION)
	{
	  double sw_avg = sw_sum / sw_count;

	  // SDSW
	  
	  field_val = sqrt((sw_sq_sum / sw_count) - (sw_avg * sw_avg));
	  _calcInterest(FilterBeamInfo::SDSW, field_val, ibeam, igate);
	}
      }
      
      // Calculate the final interest

      float dbz_val = FLT_MAX;
      if (_dbzFieldIndex >= 0)
	dbz_val = _dataPtrs[ibeam].get_scaled_value(_dbzFieldIndex, igate);

      _filterList.calcFinal(ibeam, igate, dbz_val);

    } /* endfor - igate */
  } /* endfor - ibeam */
           
  _filterList.filterData(_dataPtrs);

  _filtered = true;

  return 0;
}


/**
 * _calcGdzFields()
 */

void RadarTilt::_calcGdzFields(const RadarTilt* upper_tilt, const int ibeam, 
			       const int upper_beam_index, const int igate)
{ 
  // If one or more of the dbz fields is missing from the
  // data stream, not just filled with missing values, do nothing

  if (!dbzExists() || !_upperDbzExists)
    return;
   
  double field_val      = 0.0;

  // Get range weight for computations below

  double slant_range = igate * _gateSpacing + _startRange;
  double range_weight = _rangeWtFunc.apply(slant_range);

  // Get index into data array 
  //   Use this index when accessing data that is wrapped, e.g.
  //   _dataPtrs.  Otherwise, use ibeam, because that is the 
  //   corresponding beam in the data that is NOT wrapped.

  int d_beam_index = ibeam + _maxAzRadius;

  // Vertical GDZ fields
  //
  //   Get the current value

  float current_dbz =
    _dataPtrs[d_beam_index].get_scaled_value(_dbzFieldIndex, igate);

  float upper_dbz = upper_tilt->getDbzValF(upper_beam_index, igate);

  if( upper_dbz == FLT_MAX)
  {
    double left_upper_dbz =
      upper_tilt->getDbzValF(upper_beam_index - 1, igate);
    double right_upper_dbz =
      upper_tilt->getDbzValF(upper_beam_index + 1, igate);
      
    if( left_upper_dbz != FLT_MAX && right_upper_dbz != FLT_MAX)
      upper_dbz = (left_upper_dbz + right_upper_dbz) / 2.0;
    else
      upper_dbz =
	_noiseFloor + 10 * log10(slant_range * slant_range) + _radarConstant;
  }

  if (current_dbz != FLT_MAX)
  {
    double gdz_val = upper_dbz - current_dbz;

    // GDZ

    _calcInterest(FilterBeamInfo::GDZ, gdz_val, ibeam, igate);

    // RGDZ

    _calcInterest(FilterBeamInfo::RGDZ, gdz_val * range_weight, ibeam, igate);
               
    // RSINZ

    if (slant_range != 0 && _upperDbzExists)
    {
      double theta = (upper_tilt->getBeamElev(d_beam_index) -
		      _elevAngles[d_beam_index]) * DEG_TO_RAD;

      field_val = gdz_val / (slant_range * sin(theta));
      _calcInterest(FilterBeamInfo::RSINZ, field_val, ibeam, igate);
    }
  }
            
  // Horizontal GDZ fields

  if (igate + _slantRangeNGates < _maxGates)
  {
    // SRDZ

    int gate_loc  = (igate + _slantRangeNGates);
    float next_dbz =
      _dataPtrs[d_beam_index].get_scaled_value(_dbzFieldIndex, gate_loc);

    // If at least one of this is not missing, continue

    if (next_dbz != FLT_MAX || current_dbz != FLT_MAX)
    {
      // If neither one is missing...

      if (current_dbz != FLT_MAX && next_dbz != FLT_MAX)
      {
	field_val = (next_dbz - current_dbz) * range_weight;
      }
      else if (current_dbz == FLT_MAX)
      {
	// If the current value is missing, use the default for it

	double default_current = _noiseFloor + 10 * 
	  log10(slant_range * slant_range) + _radarConstant;
            
	field_val = (next_dbz  - default_current) * range_weight;
      }
      else if (next_dbz == FLT_MAX)
      {
	// If the next value is missing, use the default for it

	double next_slant_range = 
	  (igate + _slantRangeNGates) * _gateSpacing + _startRange;

	double default_next = _noiseFloor + 10 * 
	  log10(next_slant_range * next_slant_range) + _radarConstant;
                     
	field_val = (default_next - current_dbz) * range_weight;
      }

      // Calculate the interest value, etc.

      _calcInterest(FilterBeamInfo::SRDZ, field_val, ibeam, igate);
    }
  }

}


/**
 * _calcInterest()
 */

void RadarTilt::_calcInterest(const FilterBeamInfo::InterestType int_type,
			      const double field_val,
			      const int ibeam, const int igate)
{
  _featureFields.setData(int_type, field_val, ibeam, igate);
  _filterList.calcInterest(int_type, field_val, ibeam, igate);
}
               

/**
 * write()
 */

int RadarTilt::write() 
{
  // Make sure we don't write out anything if the tilt wasn't initialized

  if (_initFailed)
  {
    POSTMSG(INFO, "Skipping tilt");
    return 0;
  }

  // Put start of volume flags out if necessary

  if (_startOfVolume)
  {
    _filterList.putStartOfVolume(_startTime, _volNum);
    _featureFields.putStartOfVolume(_startTime, _volNum);
  }

  // If the data has been filtered, write out the new data and
  // interest data if requested

  if (_filtered)
  {
    // Write out the filtered plane

    if (_writeFiltered() != 0)
      return -1;
      
    // Write out the interest data if necessary

    if (_filterList.writeInterest(_startTime, _endTime, *_sampleParams, 
				  _elevAngle, _volNum, _tiltNum) != 0)
      return -1;

    // Write out the feature data if necessary

    if (_featureFields.write(_startTime, _endTime, *_sampleParams,
			     _elevAngle, _volNum, _tiltNum) != 0)
      return -1;
  }
  else
  {
    // Otherwise, do nothing to the data, but write it out as is

    if (_passThrough() != 0)
      return -1;
  }

  // Put out end of volume if necessary

  if (_endOfVolume)
  {
    _filterList.putEndOfVolume(_endTime, _volNum);
    _featureFields.putEndOfVolume(_endTime, _volNum);
  }
   
  return 0;
}


/**
 * _writeFiltered()
 */

int RadarTilt::_writeFiltered() 
{
  // Go through the list of messages

  int ibeam = _maxAzRadius;
  vector< RadarMsg* >::iterator it;
  for (it = _radarMsgs.begin(); it != _radarMsgs.end(); ++it)
  {
    int contents = (*it)->getMsgContent();
    DsRadarMsg current_msg = (*it)->getRadarMsg();

    // Is there beam data in this message?
    
    if (contents & DsRadarMsg::RADAR_BEAM)
    {
      DsRadarBeam &radar_beam = current_msg.getRadarBeam();

      // Tell the user what is happening
      
      if (_summaryCount > RADAR_SUMMARY_COUNT)
      {
	POSTMSG(DEBUG, "Writing: Vol Tilt El_tgt El_act     Az" );
	POSTMSG(DEBUG, "         %4ld %4ld %6.2f %6.2f %6.2f",
		(long)radar_beam.volumeNum,
		(long)radar_beam.tiltNum,
		(double)radar_beam.targetElev,
		(double)radar_beam.elevation,
		(double)radar_beam.azimuth);

	_summaryCount = 0;
      }
      _summaryCount++;

      // Copy the data into this message -
      //   Using dataLen here as the length ensures that we don't
      //   add any more data than this beam originally had and mess
      //   up the parameters associated with the beam.  Usually,
      //   dataLen will be equal to num_gates * _nFields, but if for
      //   some reason it isn't the data past dataLen will be 
      //   truncated.

      if (radar_beam.byteWidth != _dataPtrs[ibeam].get_byte_width())
      {
	POSTMSG(ERROR, "Writing filtered, widths changed %d to %d\n",
		_dataPtrs[ibeam].get_byte_width(), radar_beam.byteWidth);
      }
      else
      {
	radar_beam.loadData(_dataPtrs[ibeam].get_ptr(),
			    radar_beam.dataLen(), radar_beam.byteWidth);
	ibeam++;
      }
    } 

    // Tell the user about any flags we are writing.  Note that the
    // message can contain both flags and beam data, so we cannot
    // use an else if here

    if (contents & DsRadarMsg::RADAR_FLAGS)
    {
      DsRadarFlags &radar_flags = current_msg.getRadarFlags();
      
      if (radar_flags.endOfVolume)
	POSTMSG(DEBUG, "Wrote end of volume flag");

      if (radar_flags.startOfVolume)
	POSTMSG(DEBUG, "Wrote start of volume flag");

      if (radar_flags.endOfTilt)
	POSTMSG(DEBUG, "Wrote end of tilt flag");

      if (radar_flags.startOfTilt)
	POSTMSG(DEBUG, "Wrote start of tilt flag");
    }

    // Have to make sure both of these are not true, because
    // a single message can be just a radar beam, just flags
    // or both.  We only want to print this message if for
    // some reason we are getting a message that is neither
    // of those things.

    if (!(contents & DsRadarMsg::RADAR_BEAM) &&
	!(contents & DsRadarMsg::RADAR_FLAGS))
      POSTMSG(DEBUG, "No beam data - writing message");

    // Write the message out

    if (_outputQueue.putDsMsg( current_msg, contents))
    {
      POSTMSG(ERROR, "Could not write to fmq");
      return -1;
    }
  }

  // Tell the user what we did
  
  POSTMSG(DEBUG, "Wrote plane for %f degree tilt", _elevAngle);
   
  return 0;
}


/**
 * _passThrough()
 */

int RadarTilt::_passThrough() 
{
  // Go through the list of messages

  vector< RadarMsg* >::iterator it;
  for (it = _radarMsgs.begin(); it != _radarMsgs.end(); ++it)
  {
    int contents = (*it)->getMsgContent();
    DsRadarMsg current_msg = (*it)->getRadarMsg();

    // Is there beam data in this message?

    if (contents & DsRadarMsg::RADAR_BEAM)
    {
      DsRadarBeam &radar_beam = current_msg.getRadarBeam();

      // Tell the user what is happening

      if (_summaryCount > RADAR_SUMMARY_COUNT)
      {
	POSTMSG(DEBUG, "Writing: Vol Tilt El_tgt El_act     Az");
	POSTMSG(DEBUG, "         %4ld %4ld %6.2f %6.2f %6.2f",
		(long)radar_beam.volumeNum,
		(long)radar_beam.tiltNum,
		(double)radar_beam.targetElev,
		(double)radar_beam.elevation,
		(double)radar_beam.azimuth);

	_summaryCount = 0;
      }
      _summaryCount++;

    }

    // Tell the user about any flags we are writing.  Note that the
    // message can contain both flags and beam data, so we cannot
    // use an else if here

    if (contents & DsRadarMsg::RADAR_FLAGS)
    {
      DsRadarFlags &radar_flags = current_msg.getRadarFlags();
      
      if (radar_flags.endOfVolume)
	POSTMSG(DEBUG, "Wrote end of volume flag");

      if (radar_flags.startOfVolume)
	POSTMSG(DEBUG, "Wrote start of volume flag");

      if (radar_flags.endOfTilt)
	POSTMSG(DEBUG, "Wrote end of tilt flag");

      if (radar_flags.startOfTilt)
	POSTMSG(DEBUG, "Wrote start of tilt flag");
    } 

    // Have to make sure both of these are not true, because
    // a single message can be just a radar beam, just flags
    // or both.  We only want to print this message if for
    // some reason we are getting a message that is neither
    // of those things.

    if (!(contents & DsRadarMsg::RADAR_BEAM) &&
	!(contents & DsRadarMsg::RADAR_FLAGS))
      POSTMSG(DEBUG, "No beam data - writing message");

    // Write the message out

    if (_outputQueue.putDsMsg(current_msg, contents))
    {
      POSTMSG(ERROR, "Could not write to fmq");
      return -1;
    }

  }

  // Tell the user what we did
  
  POSTMSG(DEBUG, "Wrote plane for %f degree tilt", _elevAngle);

  return 0;
}


/**
 * _spinEval()
 */

void RadarTilt::_spinEval(const float first_val, const float second_val,
			  const float threshold,
			  const bool first_gate, bool &last_spin_positive,
			  bool &found_first_delta,
			  int& cnt_spin_change) const
{
  if (first_gate)
    found_first_delta = false;
   
  if (second_val > (first_val + threshold))
  {
    if (found_first_delta)
    {
      if (!last_spin_positive)
	++cnt_spin_change;
    }

    found_first_delta  = true;  // we've seen our first delta over threshold
    last_spin_positive = true;
  }
  else if (second_val < (first_val-threshold))
  {
    if (found_first_delta)
    {
      if (last_spin_positive)
	++cnt_spin_change;
    }

    found_first_delta = true;  // we've seen our first delta over threshold
    last_spin_positive = false;
  }
}
