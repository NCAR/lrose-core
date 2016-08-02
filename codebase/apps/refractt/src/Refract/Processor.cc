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
 * @file Processor.cc
 *
 * @class Processor
 *
 * Class for doing all of the data processing.
 *  
 * @date 12/1/2008
 *
 */

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#include <rapmath/math_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>

#include "Input.hh"
#include "PhaseFit.hh"
#include "Processor.hh"

using namespace std;

// Global variables

const string Processor::CALIB_AV_I_FIELD_NAME = "av_i";
const string Processor::CALIB_AV_Q_FIELD_NAME = "av_q";
const string Processor::CALIB_PHASE_ER_FIELD_NAME = "phase_er";

const float Processor::INVALID = -99999.0;
const double Processor::VERY_LARGE = 2147483647.0;

const double Processor::C_VACUUM = 299792458.0;
const double Processor::ABRUPT_FACTOR = 4.0;
const double Processor::THRESH_WIDTH = 5.0;

const double Processor::MIN_N_VALUE = 200.0;
const double Processor::MAX_N_VALUE = 400.0;
const double Processor::MIN_DN_VALUE = -250.0;
const double Processor::MAX_DN_VALUE = 250.0;
const double Processor::MIN_SIGMA_N_VALUE = 0.0;
const double Processor::MAX_SIGMA_N_VALUE = 100.0;
const double Processor::MIN_SIGMA_DN_VALUE = 0.0;
const double Processor::MAX_SIGMA_DN_VALUE = 100.0;


/*********************************************************************
 * Constructor
 */

Processor::Processor() :
  _debug(false),
  _verbose(false),
  _calibFile(0),
  _qualityType(QUALITY_FROM_WIDTH),
  _firstFile(true),
  _difFromRef(0),
  _difPrevScan(0),
  _rawPhase(0),
  _target(0),
  _nField(0),
  _dnField(0),
  _sigmaNField(0),
  _sigmaDnField(0)
{
}


/*********************************************************************
 * Destructor
 */

Processor::~Processor()
{
  // Free contained objects

  _freeArrays() ;
}


/*********************************************************************
 * init()
 */

bool Processor::init(DsMdvx *calib_file,
		     const int num_beams,
		     const int num_gates,
		     const int r_min,
		     const double frequency,
		     const quality_type_t quality_type,
		     const double min_consistency,
		     const bool do_relax,
		     const double n_smoothing_side_len,
		     const double dn_smoothing_side_len,
		     const bool debug_flag,
		     const bool verbose_flag)
{
  static const string method_name = "Processor::init()";
  
  // Copy the paramter values

  _debug = debug_flag | verbose_flag;
  _verbose = verbose_flag;
  
  _calibFile = calib_file;
  _numBeams = num_beams;
  _numGates = num_gates;
  _rMin = r_min;
  _qualityType = quality_type;
  _minConsistency = min_consistency;
  _doRelax = do_relax;
  _nSmoothingSideLen = n_smoothing_side_len;
  _dnSmoothingSideLen = dn_smoothing_side_len;
  
  // Check the parameter values in the parameter file for consistency

  if (_rMin >= _numGates)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "r_min value (" << _rMin << ") out of range." << endl;
    cerr << "Must be less than num_range_bins ("
	 << _numGates << ")" << endl;
    
    return false;
  }
  
  _wavelength = C_VACUUM / frequency;
  
  // Create main data arrays

  _difFromRef = new TargetData[_numBeams * _numGates];
  _difPrevScan = new TargetData[_numBeams * _numGates];
  _rawPhase = new TargetData[_numBeams * _numGates];
  _target = new TargetInfo[_numBeams * _numGates];

  return true;
}


/*********************************************************************
 * processScan()
 */

bool Processor::processScan(DsMdvx &data_file)
{
  static const string method_name = "Processor::processScan()";
  
  // Get pointers to the data fields.  We know that all of the fields
  // exist if we get to this point.

  MdvxField *i_field = data_file.getField(Input::I_FIELD_INDEX);
  MdvxField *q_field = data_file.getField(Input::Q_FIELD_INDEX);
  MdvxField *quality_field = data_file.getField(Input::QUALITY_FIELD_INDEX);
  MdvxField *snr_field = data_file.getField(Input::SNR_FIELD_INDEX);
  
  // Create the output fields

  if (!_createOutputFields(*i_field))
    return false;
  
  // Copy the raw phase data.  Set missing data values to 0.0 as needed
  // for the algorithm.

  fl32 *i_data = (fl32 *)i_field->getVol();
  Mdvx::field_header_t i_field_hdr = i_field->getFieldHeader();
  
  fl32 *q_data = (fl32 *)q_field->getVol();
  Mdvx::field_header_t q_field_hdr = q_field->getFieldHeader();
  
  for (int i = 0; i < _numGates * _numBeams; ++i)
  {
    if (i_data[i] == i_field_hdr.bad_data_value ||
	i_data[i] == i_field_hdr.missing_data_value ||
	q_data[i] == q_field_hdr.bad_data_value ||
	q_data[i] == q_field_hdr.missing_data_value)
    {
      _rawPhase[i].inphase = 0.0;
      _rawPhase[i].quadrature = 0.0;
    }
    else
    {
      _rawPhase[i].inphase = i_data[i];
      _rawPhase[i].quadrature = q_data[i];
    }
  }
  
  // Compute the quality

  _getQuality(*snr_field, *quality_field);
  
  // Compute phase diff. fields

  if (_verbose)
    cerr << endl << "Computing phase differences..." << endl;

  if (!_difPhase())
    return false;

  // Fit N & DN field to phase data

  if (_verbose)
    cerr << "Generating N field..." << endl;

  if (!_fitPhases(i_field_hdr.grid_dx * 1000.0))
    return false;

  _firstFile = false;
  
  // Add copies of the output fields to the data file.  Use the I field
  // to mask out missing beams.

  data_file.addField(_maskOutputField(*i_field, *_nField,
				      MIN_N_VALUE, MAX_N_VALUE));
  data_file.addField(_maskOutputField(*i_field, *_dnField,
				      MIN_DN_VALUE, MAX_DN_VALUE));
  data_file.addField(_maskOutputField(*i_field, *_sigmaNField,
				      MIN_SIGMA_N_VALUE, MAX_SIGMA_N_VALUE));
  data_file.addField(_maskOutputField(*i_field, *_sigmaDnField,
				      MIN_SIGMA_DN_VALUE, MAX_SIGMA_DN_VALUE));
  
  // Transfer old I/Q data to _difPrevScan to get ready for next scan

  memcpy(_difPrevScan, _rawPhase,
	 _numBeams * _numGates * sizeof(TargetData));

  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createMdvField()
 */

MdvxField *Processor::_createMdvField(const MdvxField &base_field,
				      const string &field_name,
				      const string &units) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr = base_field.getFieldHeader();
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = INVALID;
  field_hdr.missing_data_value = INVALID;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr = base_field.getVlevelHeader();
  
  // Create the new field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}

/*********************************************************************
 * _createOutputFields()
 */

bool Processor::_createOutputFields(const MdvxField &base_field)
{
  static const string method_name = "Processor::_createOutputFields()";
  
  // If the fields already exist, keep them

  if (_nField != 0 && _dnField != 0 &&
      _sigmaNField != 0 && _sigmaDnField != 0)
    return true;
  
  // Create the output fields

  delete _nField;
  delete _dnField;
  delete _sigmaNField;
  delete _sigmaDnField;
  
  _nField = _createMdvField(base_field, "N", "none");
  _dnField = _createMdvField(base_field, "DELTA_N", "none");
  _sigmaNField = _createMdvField(base_field, "SIGMA_N", "none");
  _sigmaDnField = _createMdvField(base_field, "SIGMA_DN", "none");
  
  if (_nField == 0 || _dnField == 0 ||
      _sigmaNField == 0 || _sigmaDnField == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output fields" << endl;
    
    return false;
  }
  
  return true;
}

/*********************************************************************
 * _difPhase()
 *
 * Computes the phase difference between the last two scans as well as
 * between the current scan and the reference scan.  These two arrays will be
 * the base data for the processing to follow elsewhere.
 */

bool Processor::_difPhase()
{
  static const string method_name = "Processor::_difPhase()";
  
  if (_debug)
    cerr << "And phase / phase difference arrays" << endl;

  // Get the needed calibration information

  MdvxField *calib_av_i_field =
    _calibFile->getField(CALIB_AV_I_FIELD_NAME.c_str());
  MdvxField *calib_av_q_field =
    _calibFile->getField(CALIB_AV_Q_FIELD_NAME.c_str());
  
  if (calib_av_i_field == 0 || calib_av_q_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing calibration fields in calibration file." << endl;
    
    return false;
  }
  
  // Mdvx::field_header_t calib_av_i_field_hdr =
  //   calib_av_i_field->getFieldHeader();
  fl32 *calib_av_i_data = (fl32 *)calib_av_i_field->getVol();
  
  // Mdvx::field_header_t calib_av_q_field_hdr =
  //   calib_av_q_field->getFieldHeader();
  fl32 *calib_av_q_data = (fl32 *)calib_av_q_field->getVol();
  
  int offset = 0;
  
  for (int az = 0; az < _numBeams; az++)
  {
    for (int r = 0 ; r < _numGates; r++, offset++)
    {
      // Compute phase difference wrt previous scan (now in _difPrevScan).
      // Normalize I & Q with sqrt(quality old * quality new).

      float tmp_i = _difPrevScan[offset].inphase;
      float tmp_q = _difPrevScan[offset].quadrature;
      _difPrevScan[offset].inphase =
	tmp_i * _rawPhase[offset].inphase +
	tmp_q * _rawPhase[offset].quadrature;
      _difPrevScan[offset].quadrature =
	tmp_i * _rawPhase[offset].quadrature -
	tmp_q * _rawPhase[offset].inphase;

      float norm = pow(SQR(_difPrevScan[offset].inphase) +
		       SQR(_difPrevScan[offset].quadrature), 0.375);

      if (norm != 0)
      {
	_target[offset].phase_diff =
	  atan2(_difPrevScan[offset].quadrature,
		_difPrevScan[offset].inphase) / DEG_TO_RAD;
	_difPrevScan[offset].inphase /= norm;
	_difPrevScan[offset].quadrature /= norm;
	_target[offset].phase_diff_er =
	  sqrt(-2.0 * log(norm) / norm) / DEG_TO_RAD;
	_target[offset].dif_i = _difPrevScan[offset].inphase;
	_target[offset].dif_q = _difPrevScan[offset].quadrature;
      }
      else
      {
	_target[offset].phase_diff = INVALID;
	_target[offset].phase_diff_er = INVALID;
	_target[offset].dif_i = 0.0;
	_target[offset].dif_q = 0.0;
      }

      // Compute phase difference wrt master reference

      _difFromRef[offset].inphase =
	calib_av_i_data[offset] * _rawPhase[offset].inphase +
	calib_av_q_data[offset] * _rawPhase[offset].quadrature;
      _difFromRef[offset].quadrature =
	calib_av_i_data[offset] * _rawPhase[offset].quadrature -
	calib_av_q_data[offset] * _rawPhase[offset].inphase;

      if (_difFromRef[offset].inphase != 0.0 ||
	  _difFromRef[offset].quadrature != 0.0)
      {
	_target[offset].phase =
	  atan2(_difFromRef[offset].quadrature,
		_difFromRef[offset].inphase) / DEG_TO_RAD;
	_target[offset].phase_cor = _target[offset].phase;
      }
      else
      {
	_target[offset].phase = INVALID;
	_target[offset].phase_cor = INVALID;
      }
    }
  } /* endfor - az */
  

  // And normalize for the size of calib_av_i/q

  for (int az = 0, offset = 0; az < _numBeams; az++)
  {
    for (int r = 0; r < _numGates; r++, offset++)
    {
      float norm =
	sqrt(SQR(calib_av_i_data[offset]) + SQR(calib_av_q_data[offset]));

      if (norm != 0.0)
      {
	_difFromRef[offset].inphase /= norm;
	_difFromRef[offset].quadrature /= norm;
      }

      _target[offset].i = _difFromRef[offset].inphase;
      _target[offset].q = _difFromRef[offset].quadrature;
    } /* endfor - r */
  } /* endfor - az */
  
  return true;
}


/*********************************************************************
 * _fitPhases()
 *
 * Manages the generation of the estimates of N(x,y) and dN(x,y)/dt.  This
 * method directs the phase smoothing and N generation processes for both the
 * current-to-previous-scan (dN/dt) and the current-to-reference (N) phase
 * difference fields.
 *
 * Reason of existence:  Good quality phase data is required to compute
 * refractivity data (reminder: N is derived from the derivative of phase;
 * noisy phases won't do); this is the most important (and one of the most
 * painful) task of the process.  First, the current-to-previous-scan phase
 * difference map is processed, then the more complex absolute phase
 * difference map.
 */

bool Processor::_fitPhases(const double gate_spacing)
{
  // Create the phase fitting object

  PhaseFit phase_fit;

  if (!phase_fit.init(_numBeams, _numGates, gate_spacing,
		      _wavelength, _minConsistency, _rMin,
		      _debug, _verbose))
    return false;
  
  // Start with deltaN.  Note that we can only calculating deltaN if we
  // are on our second scan.

  if (!_firstFile)
  {
    if (_debug)
      cerr << "Fitting the difference between last two scans" << endl;

    for (int az = 0, offset = 0 ; az < _numBeams; ++az)
    {
      for (int r = 0 ; r < _numGates ; ++r, ++offset)
      {
	if (r < _rMin)
	{
	  phase_fit.phase[offset] = _target[offset].phase_diff;
	  phase_fit.quality[offset] = 0.0;
	  phase_fit.inphase[offset] = 0.0;
	  phase_fit.quadrature[offset] = 0.0;
	}
	else
	{
	  phase_fit.phase[offset] = _target[offset].phase_diff;
	  if (_target[offset].phase_diff_er == INVALID)
	    phase_fit.quality[offset] = 0.0;
	  else
	    phase_fit.quality[offset] =
	      sqrt(SQR(_difPrevScan[offset].inphase) +
		   SQR(_difPrevScan[offset].quadrature));
	  phase_fit.inphase[offset] = _difPrevScan[offset].inphase;
	  phase_fit.quadrature[offset] = _difPrevScan[offset].quadrature;
	}
      } /* endfor - r */
    } /* endfor - az */
    
    phase_fit.setRefN(0.0);
    phase_fit.setSmoothSideLen(_dnSmoothingSideLen);
    phase_fit.setNOutput((fl32 *)_dnField->getVol());
    phase_fit.setNError((fl32 *)_sigmaDnField->getVol());
    
    // Perform the fit for DN

    if (!phase_fit.fitPhaseField(_doRelax))
    {
      _firstFile = true;
      return false;
    }
    
  } /* endif - !_firstFile */
  
  // Now proceed with N calculation: reset the data structure

  if (_debug)
    cerr << "Now fitting the absolute phase difference" << endl;

  for (int az = 0, offset = 0; az < _numBeams; ++az)
  {
    for (int r = 0; r < _numGates; ++r, ++offset)
    {
      if (r < _rMin)
      {
	phase_fit.phase[offset] = _target[offset].phase;
	phase_fit.quality[offset] = 0.0;
	phase_fit.inphase[offset] = 0.0;
	phase_fit.quadrature[offset] = 0.0;
      }
      else
      {
	phase_fit.phase[offset] = _target[offset].phase;
	if (_target[offset].phase_er == INVALID)
	  phase_fit.quality[offset] = 0.0;
	else
	  phase_fit.quality[offset] =
	    sqrt(SQR(_target[offset].i) + SQR(_target[offset].q));
	phase_fit.inphase[offset] = _difFromRef[offset].inphase;
	phase_fit.quadrature[offset] = _difFromRef[offset].quadrature;
      }
    } /* endfor - r */
  } /* endfor - az */
  
  phase_fit.setRefN(_calibFile->getMasterHeader().user_data_fl32[0]);
  phase_fit.setSmoothSideLen(_nSmoothingSideLen);
  phase_fit.setNOutput((fl32 *)_nField->getVol());
  phase_fit.setNError((fl32 *)_sigmaNField->getVol());

  // Perform the fit for N!

  if (!phase_fit.fitPhaseField(_doRelax))
  {
    _firstFile = true;
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _freeArrays()
 */

void Processor::_freeArrays()
{
  delete [] _target;
  delete _sigmaDnField;
  delete _sigmaNField;
  delete _dnField;
  delete _nField;
  delete [] _difPrevScan;
  delete [] _difFromRef;
  delete [] _rawPhase;

  _target = 0;
  _sigmaDnField = 0;
  _sigmaNField = 0;
  _dnField = 0;
  _nField = 0;
  _difPrevScan = 0;
  _difFromRef = 0;
  _rawPhase = 0;
}


/*********************************************************************
 * _getQuality()
 *
 * This method uses the information from the signal-to-noise ratio and spectrum
 * width fields to determine the "quality" of a target for refractive index
 * calculation purposes.  It then normalizes the average I & Q arrays to that
 * quality value.
 *
 * Task: Normalize the I & Q arrays with a quality factor so that their later
 * effect will be weighted with that quality number (and some operations may
 * or may not be applied on the data if their quality does not go over a
 * certain threshold). Quality quantifies the "usefulness" of the data point
 * for N extraction purposes.  It ranges from AbsoluteMinQ (0 is reserved to
 * bad or missing data) to 1.  Higher quality is given to 0 velocity, 0 spectrum
 * width targets whose strength exceed ThreshSNR dB above noise.  Each
 * deviation from this ideal reduces the quality.  Set the thresholds below
 * (those thresholds are used fuzilly rather than absolutely; the parameter
 * ABRUPT_FACTOR defines how fuzzy or abrupt you want your thresholds to be)
 * to define the signal-to-noise ratio, Doppler velocity, and spectrum width
 * that define the regions of "good" ground targets.  That's the theory.  In
 * practice, the quality indicator by itself is only good to reject weather
 * targets; only when combined with the "reliability" parameter computed at
 * calibration time does it become very useful ("reliability" is much a better
 * indicator of the usefulness of an echo than is "quality").
 */

bool Processor::_getQuality(const MdvxField &snr_field,
			    const MdvxField &quality_field)
{
  static const string method_name = "Processor::_getQuality()";
  
  if (_debug)
    cerr << "Computing echo quality" << endl;

  // Get pointers to the needed calibration

  MdvxField *calib_av_i_field =
    _calibFile->getField(CALIB_AV_I_FIELD_NAME.c_str());
  MdvxField *calib_av_q_field =
    _calibFile->getField(CALIB_AV_Q_FIELD_NAME.c_str());
  MdvxField *calib_phase_er_field =
    _calibFile->getField(CALIB_PHASE_ER_FIELD_NAME.c_str());
  
  if (calib_av_i_field == 0 || calib_av_q_field == 0 ||
      calib_phase_er_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error accessing calibration fields in calibration file." << endl;
    
    return false;
  }
  
  // Mdvx::field_header_t calib_av_i_field_hdr =
  //   calib_av_i_field->getFieldHeader();
  fl32 *calib_av_i_data = (fl32 *)calib_av_i_field->getVol();
  
  // Mdvx::field_header_t calib_av_q_field_hdr =
  //   calib_av_q_field->getFieldHeader();
  fl32 *calib_av_q_data = (fl32 *)calib_av_q_field->getVol();
  
  // Mdvx::field_header_t calib_phase_er_field_hdr =
  //   calib_phase_er_field->getFieldHeader();
  fl32 *calib_phase_er_data = (fl32 *)calib_phase_er_field->getVol();
  
  // Get pointers to the local data

  Mdvx::field_header_t snr_field_hdr = snr_field.getFieldHeader();
  fl32 *snr_data = (fl32 *)snr_field.getVol();
  
  Mdvx::field_header_t quality_field_hdr = quality_field.getFieldHeader();
  fl32 *quality_data = (fl32 *)quality_field.getVol();
  
  // For each (azimuth, range) cell

  for (int az = 0, index = 0; az < _numBeams; ++az)
  {
    for (int r = 0; r < _numGates; ++r, ++index)
    {
      float quality;
      float norm;
      float phase_er;
      
      // Compute a data quality value based on the thresholds given

      if (snr_data[index] != snr_field_hdr.bad_data_value &&
	  snr_data[index] != snr_field_hdr.missing_data_value)
      {
	quality = 1.0 / (1.0 + pow(10.0, - 0.1 * snr_data[index]));
	_target[index].strength = snr_data[index];
      }
      else
      {
	quality = 0.5;
	_target[index].strength = INVALID;
      }

      if (quality_data[index] != quality_field_hdr.bad_data_value &&
	  quality_data[index] != quality_field_hdr.missing_data_value)
      {
	switch (_qualityType)
	{
	case QUALITY_FROM_WIDTH :
	  quality *=
	    exp(-pow((quality_data[index]/THRESH_WIDTH), ABRUPT_FACTOR));
	  break;
	  
	case QUALITY_FROM_CPA :
	  quality *= quality_data[index];
	  break;
	} /* endswitch - _params->quality_source */
      }
      
      if (quality > 0.0 && calib_phase_er_data[index] < VERY_LARGE)
      {
	phase_er = sqrt(-2.0 * log( quality ) / quality) / DEG_TO_RAD;
      }
      else
      {
	phase_er = VERY_LARGE;
	quality = 0.0;
      }

      if (phase_er < calib_phase_er_data[index] &&
	  phase_er != VERY_LARGE)
      {
	if (quality > 0.5 && calib_phase_er_data[index] > 2000.0 &&
	    r > (_numGates / 6))
	{
	  // Good-looking unknown distant targets (anoprop?) get minimum
	  // recognition

	  quality *= 0.04;
	  phase_er = sqrt(-2.0 * log(quality) / quality) / DEG_TO_RAD;
	}
	else
	{
	  quality = sqrt(SQR(calib_av_i_data[index]) +
			 SQR(calib_av_q_data[index]));
	  phase_er = calib_phase_er_data[index];
	}
      }

      // And normalize I & Q to that quality value

      norm = sqrt(SQR(_rawPhase[index].inphase) +
		  SQR(_rawPhase[index].quadrature));

      if (norm != 0.0)
      {
	_rawPhase[index].inphase *= quality / norm;
	_rawPhase[index].quadrature *= quality / norm;
	_target[index].phase_er = phase_er;
      }
      else
      {
	_target[index].phase_er = INVALID;
      }
      
    } /* endfor - r */
  } /* endfor - az */
      
  return true;
}


/*********************************************************************
 * _maskOutputField()
 */

MdvxField *Processor::_maskOutputField(const MdvxField &mask_field,
				       const MdvxField &output_field,
				       const double min_data_value,
				       const double max_data_value) const
{
  // Create a copy of the output field to add to the output file

  MdvxField *masked_field = new MdvxField(output_field);
  
  // Set the output field values to missing where the mask field is missing

  Mdvx::field_header_t mask_field_hdr = mask_field.getFieldHeader();
  fl32 *mask_data = (fl32 *)mask_field.getVol();
  
  Mdvx::field_header_t output_field_hdr = masked_field->getFieldHeader();
  fl32 *output_data = (fl32 *)masked_field->getVol();
  
  int volume_size = mask_field_hdr.nx * mask_field_hdr.ny * mask_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    // Mask the data based on the mask field.

    if (mask_data[i] == mask_field_hdr.bad_data_value ||
	mask_data[i] == mask_field_hdr.missing_data_value)
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    
    // Set data values outside of the defined range to bad

    if (output_data[i] != output_field_hdr.bad_data_value &&
	output_data[i] != output_field_hdr.missing_data_value &&
	(output_data[i] < min_data_value ||
	 output_data[i] > max_data_value))
      output_data[i] = output_field_hdr.bad_data_value;
    
  } /* endfor - i */
  
  return masked_field;
}
