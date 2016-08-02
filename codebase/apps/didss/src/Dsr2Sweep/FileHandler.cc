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
////////////////////////////////////////////////////////////////////////
// FileHandler - Base class for classes that handle the sweep files.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////////////

#include <RayConst.h>
#include <toolsa/Path.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <dsserver/DsLdataInfo.hh>
#include "FileHandler.hh"

using namespace std;

const int FileHandler::BAD_DATA_VALUE = -9999;

/*********************************************************************
 * Constructor
 */

FileHandler::FileHandler(const Params &params) :
        _params(params)
{

  _debug = _params.debug >= Params::DEBUG_NORM;
  _verbose = _params.debug >= Params::DEBUG_VERBOSE;

  _tiltNum = 0;
  _volNum = 0;
  _radarParamsSet = false;
  _radarCalibSet = false;
  _overrideRadarLocationFlag = false;
  _overrideBeamWidthFlag = false;
  _overrideNyquistFlag = false;

  _outputDir = _params.output_dir;
  _writeToDatedDir = _params.write_to_dated_dir;
  _writeLatestDataInfoFile = _params.write_latest_data_info_file;
  
  _projectName = _params.project_name;
  _producerName = _params.producer_name;
  
  _minBeamsInSweep = _params.min_beams_in_sweep;
  _constrainElevation = _params.constrain_elevation;
  _minElevation = _params.min_elevation;
  _maxElevation = _params.max_elevation;
  _maxNbeamsInFile = _params.max_nbeams_in_file;
 
  _checkAntennaMoving = _params.check_antenna_moving;
  _minAngleChange = _params.min_angle_change;
  _prevElev = -999;
  _prevAz = -999;

  // Check that we have at least 1 beam in sweep, to avoid SEGV

  if (_minBeamsInSweep < 1) {
    _minBeamsInSweep = 1;
  }

  // Set the end-of-sweep detector for the file handler

  _eosDetector = NULL;

  switch (_params.end_of_sweep_detection) {

    case Params::SCAN_STRATEGY_EOS_DETECT :
      {
        vector< double > scan_strategy;
        for (int i = 0; i < _params.scan_strategy_n; ++i)
          scan_strategy.push_back(_params._scan_strategy[i]);
        _eosDetector =
          new ScanStrategyEosDetector(scan_strategy,
                                      _params.scan_strategy_epsilon,
                                      _params.debug >= Params::DEBUG_NORM);
        break;
      }
    
    case Params::SWEEP_NUM_EOS_DETECT :
    default:
      _eosDetector = new SweepNumEosDetector(_params.debug >= Params::DEBUG_NORM);
      break;
      
  }

  // set overrides

  if (_params.override_radar_location) {
    _radarLatOverride = _params.radar_location.latitude;
    _radarLonOverride = _params.radar_location.longitude;
    _radarAltOverride = _params.radar_location.altitude;
    _overrideRadarLocationFlag = true;
  }

  if (_params.override_beam_width) {
    _beamWidthOverride = _params.beam_width;
    _overrideBeamWidthFlag = true;
  }
  
  if (_params.override_nyquist) {
    _nyquistOverride = _params.nyquist_velocity;
    _overrideNyquistFlag = true;
  }
  
  _useTargetElev = _params.use_target_elev;

  if (_params.remove_test_pulse) {
    _ngatesTestPulse = _params.ngates_test_pulse;
    _removeTestPulse = true;
  }
  
}


/*********************************************************************
 * Destructor
 */

FileHandler::~FileHandler()
{
  delete _eosDetector;

  _clearBeams();
}


/*********************************************************************
 * processMsg() - Process the given radar message.
 */
  
void FileHandler::processMsg(DsRadarMsg &radar_msg,
			     const int contents)
{
  static const string method_name = "FileHandler::processMsg()";
  
  // set radar parameters if avaliable
  
  if (contents & DsRadarMsg::RADAR_PARAMS)
    _loadRadarParams(radar_msg);

  // set radar calibration if avaliable
  
  if (contents & DsRadarMsg::RADAR_CALIB)
    _loadRadarCalib(radar_msg);

  // set field parameters if available
  
  if (contents & DsRadarMsg::FIELD_PARAMS)
    _loadFieldParams(radar_msg);
  
  // If we have radar and field params, and there is good beam data,
  // add to the vector
  
  if (contents & DsRadarMsg::RADAR_BEAM) { 

    if (radar_msg.allParamsSet()) {
      // See if we need to update the tilt number in the beam.  Some radars
      // don't set this value.
      
      DsBeamHdr_t *beam_hdr = radar_msg.getRadarBeam().getBeamHdr();
      
      // See if we need to start a new file.  We output the current file and
      // start a new one whenever we start a new sweep.
      
      if (_radarParamsSet ) {
	if ( _eosDetector->isNewSweep(radar_msg)) {
          // Write out current file
          _writeFile();
          ++_tiltNum;
	}
      }

    // check for non-save conditions
      
    if (beam_hdr->tilt_num < 0)
    {
      if (_debug)
	cerr << "Skipping beam between tilts, tilt num ("
             <<  beam_hdr->tilt_num << " < 0)" << endl;
      
      return;
    }

    if (_checkAntennaMoving) {
      if (!_isAntennaMoving(beam_hdr)) {
	if (_debug) {
	  cerr << "WARNING - antenna not moving - will ignore beam" << endl;
	  cerr << "  el, az: ;"
	       << beam_hdr->elevation
	       << ", " << beam_hdr->azimuth << endl;
	}
	return;
      }
    }

    if (_params.filter_antenna_transitions) {
      if (beam_hdr->antenna_transition) {
	return;
      }
    }

    // save the beam
      
    Beam *new_beam = new Beam(radar_msg, _debug, _verbose);

    _addFieldData(*new_beam, radar_msg);
    
    _beams.push_back(new_beam);

    if (_maxNbeamsInFile > 0) {
      if ((int) _beams.size() > _maxNbeamsInFile) {
	if (_debug) {
	  cerr << "Writing out file because too many beams: "
	       << _beams.size() << endl;
	}
	_writeFile();
      }
    }
    
  }
  } /* if (contents ... */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addFieldData() - Add the field data to the given beam
 */

void FileHandler::_addFieldData(Beam &beam, const DsRadarMsg &radar_msg)
{
  static const string method_name = "FileHandler::_addFieldData()";

  int num_gates = _radarParams.numGates;

  const DsRadarBeam &dsr_beam = radar_msg.getRadarBeam();
  
  vector< FieldInfo >::const_iterator field;

  for (field = _fields.begin(); field != _fields.end(); ++field)
  {
    RayDoubles *beam_data = new RayDoubles;
    beam_data->reserve(num_gates);
      
    for (int gate = 0; gate < num_gates; ++gate)
    {

      double value;
      
      if (field->dsrIndex < 0)
      {
	value = BAD_DATA_VALUE;
      }
      else
      {
	int byte_width = dsr_beam.byteWidth;
	int dsr_index = (gate * _radarParams.numFields) + field->dsrIndex;
	
	if (byte_width == 4)
        {
	  fl32 fval =
	    *((fl32 *)dsr_beam.data() + dsr_index);
          if (fval == (fl32) field->inputMissingValue) {
            value = BAD_DATA_VALUE;
          } else {
            value = fval;
          }
        }
        else if (byte_width == 2)
        {
	  int ival =
	    *((ui16 *)dsr_beam.data() + dsr_index);
          if ((fl32) ival == field->inputMissingValue) {
            value = BAD_DATA_VALUE;
          } else {
            value = ival * field->inputScale + field->inputBias;
          }
        }
	else
        {
	  int ival =
	    *((ui08 *)dsr_beam.data() + dsr_index);
          if (ival == field->inputMissingValue) {
            value = BAD_DATA_VALUE;
          } else {
            value = ival * field->inputScale + field->inputBias;
          }
        }

      }


      beam_data->push_back((double) value);
	
    } /* endfor - gate */
    
    beam.addFieldData(beam_data);
    
  } /* endfor - field */

}


/*********************************************************************
 * _buildHeader() - Build the ray file header.
 */

void FileHandler::_buildHeader(RayFile &ray_file,
			       const ForayUtility::RaycTime &startTime,
			       const ForayUtility::RaycTime &endTime,
			       const double target_angle,
			       const int vol_num, const int tilt_num)
{

  static const string method_name = "FileHandler::_buildHeader()";
  
  try
  {
    if (_debug)
    {
      cerr << "Setting start_time to "
	   << DateTime::str(startTime.seconds())
	   << " (" << startTime.seconds() << ")" << endl;
      cerr << "Setting number_of_cells to " << _radarParams.numGates << endl;
    }
    
    ray_file.set_time("start_time", startTime);
    ray_file.set_time("stop_time", endTime);
    ray_file.set_angle("fixed_angle", target_angle);   
    ray_file.set_angle("start_angle", -9999.0);
    ray_file.set_angle("stop_angle", -9999.0);
    ray_file.set_integer("number_of_cells", _radarParams.numGates);
    ray_file.set_integer("cell_spacing_method",
			  (int)RayConst::cellSpacingBySegment);
    ray_file.set_integer("number_of_frequencies",1); 
    ray_file.set_integer("number_of_prfs",1); 
    ray_file.set_integer("number_of_segments", 1);
    ray_file.set_integer("number_of_cell_segments", 1);
    ray_file.set_double ("segment_cell_spacing", 0,
			  _radarParams.gateSpacing * 1000.0);
    ray_file.set_integer("segment_cell_count", 0, _radarParams.numGates);
    ray_file.set_double ("meters_to_first_cell",
			  _radarParams.startRange * 1000.0);
    ray_file.set_double ("nyquist_velocity", _nyquist);
    ray_file.set_double ("unambiguous_range",
			  _radarParams.unambigRange * 1000.0);
    ray_file.set_double ("platform_longitude", _radarParams.longitude);
    ray_file.set_double ("platform_latitude", _radarParams.latitude);
    ray_file.set_double ("platform_altitude", _radarParams.altitude * 1000.0);
    ray_file.set_double ("radar_constant", _radarParams.radarConstant);
    ray_file.set_double ("receiver_gain", _radarParams.receiverGain);
    ray_file.set_double ("antenna_gain", _radarParams.antennaGain);
    ray_file.set_double ("system_gain", _radarParams.systemGain);
    ray_file.set_double ("pulse_width", _radarParams.pulseWidth);
    ray_file.set_double ("band_width", -9999.0);
    ray_file.set_double ("peak_power", _radarParams.xmitPeakPower);
    ray_file.set_double ("ray_peak_power", _radarParams.xmitPeakPower);
    ray_file.set_double ("transmitter_power", _radarCalib.getXmitPowerDbmH());
    // ray_file.set_double ("noise_power", _radarCalib.getNoiseDbmHc());
    ray_file.set_double ("noise_power", _radarCalib.getNoiseDbmHc() - _radarCalib.getReceiverGainDbHc());
    ray_file.set_boolean("test_pulse_present", _params.test_pulse_present);
    ray_file.set_double ("test_pulse_power", _params.test_pulse_power);
    ray_file.set_double ("test_pulse_start_range", _params.test_pulse_start_range);
    ray_file.set_double ("test_pulse_end_range", _params.test_pulse_end_range);
    double frequency = 3.0e8 / (_radarCalib.getWavelengthCm() / 100.);
    ray_file.set_double ("frequency", 0, frequency);
    ray_file.set_double ("pulse_repetition_frequency", 0,
			  _radarParams.pulseRepFreq);
    
    ray_file.set_string ("platform_name", _params.platform_name);
    ray_file.set_integer("platform_type", (int)RayConst::radarTypeGround);
    ray_file.set_integer("scan_mode",
			  _getForayScanMode(_radarParams.scanMode));
    ray_file.set_integer("volume_number", vol_num);
    ray_file.set_integer("sweep_number", tilt_num);
    ray_file.set_integer("number_of_samples", _radarParams.samplesPerBeam);
    ray_file.set_string ("project_name", _projectName);
    ray_file.set_string ("producer_name", _producerName);

    if (_radarCalibSet) {
      ray_file.set_boolean("calibration_data_present", true);
      ray_file.set_double ("horizontal_beam_width",
                           _radarCalib.getBeamWidthDegH());
      ray_file.set_double ("vertical_beam_width",
                           _radarCalib.getBeamWidthDegV());
      ray_file.set_double
        ("horizontal_antenna_gain_db", _radarCalib.getAntGainDbH());
      ray_file.set_double
        ("vertical_antenna_gain_db", _radarCalib.getAntGainDbV());
      if (_params.set_calib_power_to_measured_power) {
	ray_file.set_double
	  ("horizontal_transmitter_power_dbm", _radarParams.measXmitPowerDbmH);
	ray_file.set_double
	  ("vertical_transmitter_power_dbm", _radarParams.measXmitPowerDbmV);
      } else {
	ray_file.set_double
	  ("horizontal_transmitter_power_dbm", _radarCalib.getXmitPowerDbmH());
	ray_file.set_double
	  ("vertical_transmitter_power_dbm", _radarCalib.getXmitPowerDbmV());
      }
      ray_file.set_double
        ("horizontal_two_way_waveguide_loss_db", _radarCalib.getTwoWayWaveguideLossDbH());
      ray_file.set_double
        ("vertical_two_way_waveguide_loss_db", _radarCalib.getTwoWayWaveguideLossDbV());
      ray_file.set_double
        ("horizontal_two_way_radome_loss_db", _radarCalib.getTwoWayRadomeLossDbH());
      ray_file.set_double
        ("vertical_two_way_radome_loss_db", _radarCalib.getTwoWayRadomeLossDbV());
      ray_file.set_double
        ("receiver_mismatch_loss_db", _radarCalib.getReceiverMismatchLossDb());
      ray_file.set_double
        ("horizontal_radar_constant", _radarCalib.getRadarConstH());
      ray_file.set_double
        ("vertical_radar_constant", _radarCalib.getRadarConstV());
      ray_file.set_double
        ("horizontal_co_polar_noise_dbm", _radarCalib.getNoiseDbmHc());
      ray_file.set_double
        ("vertical_co_polar_noise_dbm", _radarCalib.getNoiseDbmVc());
      ray_file.set_double
        ("horizontal_cross_polar_noise_dbm", _radarCalib.getNoiseDbmHx());
      ray_file.set_double
        ("vertical_cross_polar_noise_dbm", _radarCalib.getNoiseDbmVx());
      ray_file.set_double
        ("horizontal_co_polar_receiver_gain_db", _radarCalib.getReceiverGainDbHc());
      ray_file.set_double
        ("vertical_co_polar_receiver_gain_db", _radarCalib.getReceiverGainDbVc());
      ray_file.set_double
        ("horizontal_cross_polar_receiver_gain_db", _radarCalib.getReceiverGainDbHx());
      ray_file.set_double
        ("vertical_cross_polar_receiver_gain_db", _radarCalib.getReceiverGainDbVx());
      ray_file.set_double
        ("horizontal_co_polar_base_dbz_at_1km", _radarCalib.getBaseDbz1kmHc());
      ray_file.set_double
        ("vertical_co_polar_base_dbz_at_1km", _radarCalib.getBaseDbz1kmVc());
      ray_file.set_double
        ("horizontal_cross_polar_base_dbz_at_1km", _radarCalib.getBaseDbz1kmHx());
      ray_file.set_double
        ("vertical_cross_polar_base_dbz_at_1km", _radarCalib.getBaseDbz1kmVx());
      ray_file.set_double
        ("horizontal_co_polar_sun_power_dbm", _radarCalib.getSunPowerDbmHc());
      ray_file.set_double
        ("vertical_co_polar_sun_power_dbm", _radarCalib.getSunPowerDbmVc());
      ray_file.set_double
        ("horizontal_cross_polar_sun_power_dbm", _radarCalib.getSunPowerDbmHx());
      ray_file.set_double
        ("vertical_cross_polar_sun_power_dbm", _radarCalib.getSunPowerDbmVx());
      ray_file.set_double
        ("horizontal_noise_source_power_dbm", _radarCalib.getNoiseSourcePowerDbmH());
      ray_file.set_double
        ("vertical_noise_source_power_dbm", _radarCalib.getNoiseSourcePowerDbmV());
      ray_file.set_double
        ("horizontal_power_measurement_loss_db", _radarCalib.getPowerMeasLossDbH());
      ray_file.set_double
        ("vertical_power_measurement_loss_db", _radarCalib.getPowerMeasLossDbV());
      ray_file.set_double
        ("horizontal_coupler_forward_loss_db", _radarCalib.getCouplerForwardLossDbH());
      ray_file.set_double
        ("vertical_coupler_forward_loss_db", _radarCalib.getCouplerForwardLossDbV());
      ray_file.set_double
        ("zdr_correction_db", _radarCalib.getZdrCorrectionDb());
      ray_file.set_double
        ("horizontal_ldr_correction_db", _radarCalib.getLdrCorrectionDbH());
      ray_file.set_double
        ("vertical_ldr_correction_db", _radarCalib.getLdrCorrectionDbV());
      ray_file.set_double
        ("system_phidp_degrees", _radarCalib.getSystemPhidpDeg());
    } else {
      ray_file.set_double ("horizontal_beam_width",
                           _radarParams.horizBeamWidth);
      ray_file.set_double ("vertical_beam_width",
                         _radarParams.vertBeamWidth);
      ray_file.set_boolean("calibration_data_present", false);
    }

    ray_file.set_integer("number_of_fields", (int)_fields.size());
    ray_file.set_integer("number_of_rays", _beams.size());

    vector< FieldInfo >::const_iterator field;
    int i;
    for (field = _fields.begin(), i = 0; field != _fields.end(); ++field, ++i)
    {

      ray_file.set_string("field_name", i, field->name);
      ray_file.set_string("field_long_name", i, field->name);

      if (field->outputByteWidth == 2) {

        ray_file.set_integer("binary_format", i,
                             (int)RayConst::binaryFormat2ByteInt);

	// this hack needed for S-Pol legacy rain and particle id 
	// that assume scale of 100, bias of 0
	if (_params.scale_by_100_bias_0) {
	  ray_file.set_double("parameter_scale", i, 0.01 );
	  ray_file.set_double("parameter_bias", i, 0.0 );
	} else {
          if (field->outputScale < -9990) {
            ray_file.set_double("parameter_scale", i, field->inputScale);
          } else {
            ray_file.set_double("parameter_scale", i, field->outputScale);
          }
	  ray_file.set_double("parameter_bias", i, field->outputBias);
	}
	
      } else {

        ray_file.set_integer("binary_format", i,
                             (int)RayConst::binaryFormat4ByteFloat);
        ray_file.set_double("parameter_scale", i, 1.0);
        ray_file.set_double("parameter_bias", i, 0.0);

      }

      ray_file.set_integer("field_units", i, field->units);
      ray_file.set_integer("bad_data", i, BAD_DATA_VALUE);
      ray_file.set_integer("field_polarization", i,
			    _getForayPolarization(_radarParams.polarization));
      
    } /* endfor - field, i */
  }

  catch (ForayUtility::Fault &fault)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << fault.msg() << endl;
  }
  
}


/*********************************************************************
 * _getForayPolarization() - Convert the given DsRadar polarization to
 *                           a foray polarization.
 */

int FileHandler::_getForayPolarization(const int ds_radar_polar)
{
  switch (ds_radar_polar)
  {
  case DS_POLARIZATION_HORIZ_TYPE :
    return RayConst::horizontalPolarization;
    
  case DS_POLARIZATION_VERT_TYPE :
    return RayConst::verticalPolarization;

  case DS_POLARIZATION_RIGHT_CIRC_TYPE :
      return RayConst::noPolarization;
    
  case DS_POLARIZATION_ELLIPTICAL_TYPE :
    return RayConst::noPolarization;
    
  case DS_POLARIZATION_LEFT_CIRC_TYPE :
    return RayConst::noPolarization;
    
  case DS_POLARIZATION_DUAL_TYPE :
    return RayConst::noPolarization;
    
  }
  
  return RayConst::noPolarization;
}


/*********************************************************************
 * _getForayScanMode() - Convert the given DsRadar scan mode to a foray
 *                       scan mode.
 */

int FileHandler::_getForayScanMode(const int ds_radar_scan_mode)
{
  switch (ds_radar_scan_mode)
  {
  case DS_RADAR_UNKNOWN_MODE :
    return RayConst::scanModeSUR;
    
  case DS_RADAR_CALIBRATION_MODE :
    return RayConst::scanModeCAL;

  case DS_RADAR_SECTOR_MODE :
      return RayConst::scanModePPI;
    
  case DS_RADAR_COPLANE_MODE :
    return RayConst::scanModeCOP;
    
  case DS_RADAR_RHI_MODE :
    return RayConst::scanModeRHI;
    
  case DS_RADAR_VERTICAL_POINTING_MODE :
    return RayConst::scanModeVER;
    
  case DS_RADAR_TARGET_MODE :
    return RayConst::scanModeTAR;
    
  case DS_RADAR_MANUAL_MODE :
    return RayConst::scanModeTAR;
    
  case DS_RADAR_IDLE_MODE :
    return RayConst::scanModeIDL;
    
  case DS_RADAR_SURVEILLANCE_MODE :
    return RayConst::scanModeSUR;

  default:
    return RayConst::scanModeSUR;

  }

}


/*********************************************************************
 * _loadRadarParams() - load radar params
 */

void FileHandler::_loadRadarParams(const DsRadarMsg &radar_msg)
{

  DsRadarParams new_radar_params = radar_msg.getRadarParams();

  if (!_radarParamsSet ||
      _radarParamsChanged(_radarParams, new_radar_params))
  {
    if (_radarParamsSet)
    {
      if (_debug)
	cerr << "Radar params changed -- writing sweep file" << endl;
      
      _writeFile();
      ++_tiltNum;
    }
    
    if (_debug)
      cerr << "Resetting radar params" << endl;
    
    _radarParams = new_radar_params;
    
    if (_overrideRadarLocationFlag)
    {
      _radarParams.latitude = _radarLatOverride;
      _radarParams.longitude = _radarLonOverride;
      _radarParams.altitude = _radarAltOverride;
    }
    
    // check to make sure altitude is in km, not meters
    
    if (_radarParams.altitude > 8.0 && _debug)
    {
      cerr << "WARNING : Sensor altitude is " << _radarParams.altitude
	   << " Km." << endl;
      cerr << "  Are the right units being used for altitude?" << endl;
      cerr << "  Incorrect altitude results in bad cart remapping." << endl;
    }
    
    if (_overrideBeamWidthFlag)
      _radarParams.vertBeamWidth = _beamWidthOverride;

    _radarParamsSet = true;
  }

  if (_overrideNyquistFlag)
    _nyquist = _nyquistOverride;
  else
    _nyquist =
      (_radarParams.pulseRepFreq * (_radarParams.wavelength / 100.0)) / 4.0;
}


/*********************************************************************
 * _loadRadarCalib() - load radar calib
 */

void FileHandler::_loadRadarCalib(const DsRadarMsg &radar_msg)
{
  _radarCalib = radar_msg.getRadarCalib();
  _radarCalibSet = true;
}

/*********************************************************************
 * _loadFieldParams() - load field params
 */

void FileHandler::_loadFieldParams(const DsRadarMsg &radar_msg)
{
  for (size_t ii = 0; ii < radar_msg.getFieldParams().size(); ii++)
  {
    const DsFieldParams *rfld = radar_msg.getFieldParams(ii);

    for (size_t ifield = 0; ifield < _fields.size(); ifield++)
    {
      FieldInfo &field = _fields[ifield];
      if (field.dsrName == rfld->name)
      {
	field.inputByteWidth = rfld->byteWidth;
	field.inputMissingValue = rfld->missingDataValue;
	field.inputScale = rfld->scale;
	field.inputBias = rfld->bias;
	if (field.name.size() == 0)
	  field.name = rfld->name;
	field.dsrIndex = ii;
      } // if (!strcmp ...
    } // ifield
  } // ii
}


/*********************************************************************
 * _writeFile() - Write the current file to disk.
 */

void FileHandler::_writeFile()
{
  static const string method_name = "FileHandler::_writeFile()";
  
  // check that the output dir exists

  if (ta_makedir_recurse(_outputDir.c_str()))
  {
    cerr << "ERROR - FileHandler::_writeFile" << endl;
    cerr << "  Cannot make output dir: " << _outputDir << endl;
    return;
  }

  if (_beams.size() < 1) {
    return;
  }
  string outputSubDir = _outputDir;
  string file_name;
  Beam *firstBeam = _beams[0];
  Beam *lastBeam = _beams[_beams.size()-1];
  time_t firstBeamTime = firstBeam->getTime().seconds();
  time_t lastBeamTime = lastBeam->getTime().seconds();
  int scanMode = _radarParams.scanMode;
  double targetAngle = 0.0;

  if (_params.use_fixed_angle_for_filename) {
    if (scanMode == DS_RADAR_RHI_MODE) {
      targetAngle = _getTargetAz();
    } else {
      targetAngle = _getTargetEl();
    }
  } else {
    if (scanMode == DS_RADAR_RHI_MODE) {
      targetAngle = _computeMeanAz();
    } else {
      targetAngle = _computeMeanEl();
    }
  }

  int volNum = firstBeam->getVolumeNum();
  int tiltNum = firstBeam->getTiltNum();

  if ((int)_beams.size() < _minBeamsInSweep)
  {
    if (_debug)
      cerr << "Skipping sweep with only " << _beams.size() << " beams" << endl;
    
    _clearBeams();
    return;
  }

  if (_constrainElevation && scanMode != DS_RADAR_RHI_MODE)
  {
    if (targetAngle < _minElevation || targetAngle > _maxElevation)
    {
      if (_debug)
        cerr << "Skipping sweep with elev " << targetAngle << " deg" << endl;
      
      _clearBeams();
      return;
    }
  }
  
  if (_debug)
    cerr << "Writing sweep file containing " << _beams.size()
	 << " beams" << endl;

  // Create the file object

  RayFile *ray_file = _createFileObj();
  string outputPath;

  int iret = 0;
  try
  {
  
    // Generate the file name
    
    if (_debug)
    {
      cerr << "Generating file name:" << endl;
      cerr << "   Time = " << DateTime::str(firstBeamTime)
	   << " (" << firstBeamTime << ")" << endl;
      cerr << "   Time = " << DateTime::str(lastBeamTime)
	   << " (" << lastBeamTime << ")" << endl;
      cerr << "   Scan mode = " << _radarParams.scanMode << endl;
      cerr << "   Target angle = " << targetAngle << endl;
      cerr << "   Vol num = " << volNum << endl;
      cerr << "   Tilt num = " << tiltNum << endl;
    }
    
    file_name =
      generateFileName(firstBeam->getTime(),
		       _getForayScanMode(_radarParams.scanMode),
		       targetAngle, volNum, tiltNum);

    if (_writeToDatedDir)
    {
      char dateStr[32];
      DateTime dtime(firstBeamTime);
      sprintf(dateStr, "%.4d%.2d%.2d",
              dtime.getYear(), dtime.getMonth(), dtime.getDay());
      outputSubDir += PATH_DELIM;
      outputSubDir += dateStr;
      if (ta_makedir_recurse(outputSubDir.c_str()))
      {
        cerr << "ERROR - FileHandler::_writeFile" << endl;
        cerr << "  Cannot make output sub dir: " << outputSubDir << endl;
        return;
      }
    }

    outputPath = outputSubDir;
    outputPath += PATH_DELIM;
    outputPath += file_name;

    if (_debug)
    {
      cerr << "Output filename: " << file_name << endl;
      cerr << "Output dir: " << outputSubDir << endl;
      cerr << "Output path: " << outputPath << endl;
    }
    
    // Open the output file

    ray_file->open_file(outputPath, true);
  
    // Build and store the header information

    _buildHeader(*ray_file,
		 firstBeam->getTime(),
		 lastBeam->getTime(),
		 targetAngle, volNum, tiltNum);
    ray_file->write_ground_headers();

    if (_debug)
      cerr << "Ground headers successfully written" << endl;
    
    // Store the beams

    vector< Beam* >::const_iterator beam_iter;
    for (beam_iter = _beams.begin(); beam_iter != _beams.end(); ++beam_iter)
    {
      if (_verbose)
	cerr << "    Storing beam" << endl;
      
      Beam *beam = *beam_iter;
      beam->addBeamToFile(ray_file);
      ray_file->write_ground_ray();
    }

    ray_file->write_ground_tail();
    
    ray_file->close_file();
  }
  catch (ForayUtility::Fault &fault)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << fault.msg() << endl;
    iret = -1;
  }

  if (iret == 0 && _writeLatestDataInfoFile)
  {

    string relPath;
    Path::stripDir(_outputDir, outputPath, relPath);
    DsLdataInfo ldata(_outputDir);
    ldata.setRelDataPath(relPath.c_str());
    ldata.setWriter("Dsr2Sweep");
    ldata.setDataType(_dataType);
    Path name(file_name);
    ldata.setDataFileExt(name.getExt().c_str());
    ldata.write(firstBeamTime);
  }
  
  // Clear out the message queue and the file object

  _clearBeams();
  delete ray_file;
  
}

/*********************************************************************
 * _radarParamsChanged() -  check if radar params have changed
 *
 * returns true if params have changed
 */

bool FileHandler::_radarParamsChanged(const DsRadarParams &oldParams,
                                      const DsRadarParams &newParams)
{

  if (oldParams.numFields != newParams.numFields)
  {
    return true;
  }

  if (oldParams.numGates != newParams.numGates)
  {
    return true;
  }

  if (oldParams.scanType != newParams.scanType)
  {
    return true;
  }

  if (oldParams.scanMode != newParams.scanMode)
  {
    return true;
  }

  if (oldParams.polarization != newParams.polarization)
  {
    return true;
  }

  if (fabs(oldParams.gateSpacing - newParams.gateSpacing) > 0.001)
  {
    return true;
  }

  if (fabs(oldParams.startRange - newParams.startRange) > 0.001)
  {
    return true;
  }

  return false;

}
  
////////////////////////////////////////////////////////////////
// is antenna moving?

bool FileHandler::_isAntennaMoving(const DsBeamHdr_t *beamHdr)

{

  double el = beamHdr->elevation;
  double az = beamHdr->azimuth;
  double deltaEl = fabs(el - _prevElev);
  double deltaAz = fabs(az - _prevAz);

  bool isMoving;
  if (deltaEl > _minAngleChange ||
      deltaAz > _minAngleChange) {
    isMoving = true;
  } else {
    isMoving = false;
  }

  _prevElev = el;
  _prevAz = az;

  return isMoving;

}

////////////////////////////////////////////////////////////////
// compute mean elevation angle

double FileHandler::_computeMeanEl()

{

  if (_beams.size() == 0) {
    return 0.0;
  }

  double sumx = 0.0;
  double sumy = 0.0;
  double count = 0.0;
  
  for (int ii = 0; ii < (int) _beams.size(); ii++) {
    const Beam *beam = _beams[ii];
    double el = beam->getElevation() * DEG_TO_RAD;
    sumx += cos(el);
    sumy += sin(el);
    count++;
  }
  
  double meanx = sumx / count;
  double meany = sumy / count;
  double meanEl = atan2(meany, meanx) * RAD_TO_DEG;
  if (meanEl < 0) {
    meanEl += 360.0;
  }

  return meanEl;

}

////////////////////////////////////////////////////////////////
// compute mean azimuth angle

double FileHandler::_computeMeanAz()

{

  if (_beams.size() == 0) {
    return 0.0;
  }

  double sumx = 0.0;
  double sumy = 0.0;
  double count = 0.0;
  
  for (int ii = 0; ii < (int) _beams.size(); ii++) {
    const Beam *beam = _beams[ii];
    double az = beam->getAzimuth() * DEG_TO_RAD;
    sumx += cos(az);
    sumy += sin(az);
    count++;
  }
  
  double meanx = sumx / count;
  double meany = sumy / count;
  double meanAz = atan2(meany, meanx) * RAD_TO_DEG;
  if (meanAz < 0) {
    meanAz += 360.0;
  }

  return meanAz;

}

////////////////////////////////////////////////////////////////
// get target elevation angle from center of beam

double FileHandler::_getTargetEl()

{

  if (_beams.size() == 0) {
    return 0.0;
  }

  int midIndex = (int) _beams.size() / 2;
  return _beams[midIndex]->getTargetElevation();

}

////////////////////////////////////////////////////////////////
// get target azimuth angle from center of beam

double FileHandler::_getTargetAz()

{

  if (_beams.size() == 0) {
    return 0.0;
  }

  int midIndex = (int) _beams.size() / 2;
  return _beams[midIndex]->getTargetAzimuth();

}

