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
 * @date 1/15/2008
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Processor.hh"
#include "Params.hh"

using namespace std;

// Global variables

const size_t Processor::MINIMUM_TARGET_LIST_FILES = 2;
const float Processor::INVALID = -999999.0;
const float Processor::VERY_LARGE = 2147483647;



/*********************************************************************
 * Constructor
 */

Processor::Processor() :
  _debug(false),
  _verbose(false),
  _inputHandler(0),
  _numAzim(0),
  _numRangeBins(0),
  _rMin(0),
  _beamWidth(0.0),
  _sideLobePower(0.0),
  _refFilePath(""),
  _fluctSnr(0),
  _meanSnrField(0),
  _pixelCount(0),
  _sumAField(0),
  _sumBField(0),
  _sumPField(0),
  _difPrevScanIField(0),
  _difPrevScanQField(0),
  _oldSnrField(0),
  _calibStrengthField(0),
  _calibAvIField(0),
  _calibAvQField(0),
  _calibPhaseErField(0),
  _calibQualityField(0),
  _calibNcpField(0),
  _writeDebugMdvFiles(false),
  _debugMdvUrl("")
{
  static const string method_name = "Processor::Processor()";
}


/*********************************************************************
 * Destructor
 */

Processor::~Processor()
{
  // Free global arrays

  delete [] _fluctSnr;
  delete _meanSnrField;
  delete [] _pixelCount;
  delete  _sumAField;
  delete  _sumBField;
  delete  _sumPField;
  delete _difPrevScanIField;
  delete _difPrevScanQField;
  delete _oldSnrField;

  delete _inputHandler;
  
  // Don't delete the calibration field pointers since these will be
  // taken over by the calibration MDV file object and deleted there.
}


/*********************************************************************
 * init()
 */

bool Processor::init(const int num_azim, const int num_range_bins,
		     const int r_min, const double beam_width,
		     const double side_lobe_power,
		     const string &ref_file_path,
		     Input *input_handler,
		     const bool debug_flag, const bool verbose_flag)
{
  static const string method_name = "Processor::init()";
  
  // Set global members

  _debug = debug_flag | verbose_flag;
  _verbose = verbose_flag;
  _inputHandler = input_handler;
  _numAzim = num_azim;
  _numRangeBins = num_range_bins;
  _rMin = r_min;
  _beamWidth = beam_width;
  _sideLobePower = side_lobe_power;
  _refFilePath = ref_file_path;

  return true;
}


/*********************************************************************
 * calibTargets()
 */

bool Processor::calibTargets(const vector< string > &file_list,
			     const double required_gate_spacing)
{
  static const string method_name = "Processor::calibTargets()";
  
  // Initialize some variables

  if (_debug)
    cerr << endl << "Calibrating ground targets for N = "
	 << _nValue << "..." << endl;
  
  fl32 *sum_a_data = (fl32 *)_sumAField->getVol();
  fl32 *sum_b_data = (fl32 *)_sumBField->getVol();
  
  memset(sum_a_data, 0, _numAzim * _numRangeBins * sizeof(fl32));
  memset(sum_b_data, 0, _numAzim * _numRangeBins * sizeof(fl32));

  // Read phase data and normalize

  if (_debug)
    cerr << "Going through all the files:" << endl;
  
  vector< string >::const_iterator file_iter;
  int file_num = 1;
  
  int scan_size = 0;
  
  DateTime last_data_time;
  
  for (file_iter = file_list.begin(); file_iter != file_list.end();
       ++file_iter, ++file_num)
  {
    if (_debug)
      cerr << "   " << file_num << " - " << *file_iter << endl;
    
    DsMdvx mdvx;
    
    if (!_inputHandler->getNextScan(*file_iter, mdvx))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading input file:" << *file_iter << endl;
    
      return false;
    }
  
    // Get pointers into the data.  Make sure that the important 
    // characteristics of the data don't change between scans.  Note
    // that we are guaranteed that num_azim and num_gates will remain
    // the same because of the way we do the input.

    MdvxField *i_field = mdvx.getField(Input::I_FIELD_INDEX);
    Mdvx::field_header_t i_field_hdr = i_field->getFieldHeader();
    fl32 *i_data = (fl32 *)i_field->getVol();
  
    MdvxField *q_field = mdvx.getField(Input::Q_FIELD_INDEX);
    Mdvx::field_header_t q_field_hdr = q_field->getFieldHeader();
    fl32 *q_data = (fl32 *)q_field->getVol();
  
    if (i_field_hdr.grid_dx != required_gate_spacing ||
	q_field_hdr.grid_dx != required_gate_spacing)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Gate spacing changes between scans:" << endl;
      cerr << "    prev gate spacing = " << required_gate_spacing << endl;
      cerr << "    I field gate spacing = " << i_field_hdr.grid_dx << endl;
      cerr << "    Q field gate spacing = " << q_field_hdr.grid_dx << endl;
      
      return false;
    }
    
    int num_azim = i_field_hdr.ny;
    int num_gates = i_field_hdr.nx;
    scan_size = num_azim * num_gates;
    last_data_time = mdvx.getMasterHeader().time_centroid;
    
    // Normalize the I and Q values

    for (int index = 0; index < scan_size; ++index)
    {
      float norm = sqrt(SQR(i_data[index]) + SQR(q_data[index]));

      if (norm != 0.0)
      {
	i_data[index] /= norm;
	q_data[index] /= norm;
      }
    } /* endfor - index */
    
    // Average phase data by averaging I and Q

    for (int index = 0; index < scan_size; ++index)
    {
	sum_a_data[index] += i_data[index];
	sum_b_data[index] += q_data[index];
    } /* endfor - index */

  } /* endfor - file_iter */

  // Normalize the reference information

  if (_debug)
    cerr << endl << "Normalizing reference information..." << endl;
  
  fl32 *calib_av_i_data = (fl32 *)_calibAvIField->getVol();
  fl32 *calib_av_q_data = (fl32 *)_calibAvQField->getVol();
  fl32 *phase_er_data = (fl32 *)_calibPhaseErField->getVol();
  fl32 *calib_ncp_data = (fl32 *)_calibNcpField->getVol();
  
  for (int index = 0; index < scan_size; ++index)
  {
    if (calib_ncp_data[index] == INVALID)
      continue;
    
    float norm2 = sqrt(SQR(calib_ncp_data[index]));

    if (norm2 == 0.0)
      continue;
    
    float norm =
      sqrt(SQR(sum_a_data[index]) + SQR(sum_b_data[index])) / norm2 ;

    if (norm == 0.0)
    {
      calib_av_i_data[index] = 0.0;
      calib_av_q_data[index] = 0.0;
      phase_er_data[index] = VERY_LARGE;
    }
    else
    {
      calib_av_i_data[index] = sum_a_data[index] / norm;
      calib_av_q_data[index] = sum_b_data[index] / norm;
    }
  } /* endfor - index */

  // Write the calibration file

  if (!_writeCalibrationFile(last_data_time))
    return false;
  
  if (_debug)
    cerr << "Calibration completed." << endl;
  
  return true;
}


/*********************************************************************
 * findReliableTargets()
 *
 * Go through a list of files containing ground echo phase data and use them
 * to determine their suitability (or their reliability) for the computation
 * of N.  Specifically, this involves determining whether the phase of targets
 * are stable (or at least are consistent) in time or if they fluctuate (and
 * if yes, by how much).
 *
 * Good targets are those whose phase is relatively constant with time; others
 * are either precipitation/clear air/insect clutter, or they are from ground
 * targets that move (e.g. vegetation, lakes).
 */

bool Processor::findReliableTargets(const vector< string > &file_list,
				    double &gate_spacing)
{
  static const string method_name = "Processor::findReliableTargets()";
  
  // Check for the minimum number of files that can be included in the
  // file list.

  if (file_list.size() < MINIMUM_TARGET_LIST_FILES)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Not enough files in the file list for finding targets." << endl;
    cerr << "Have " << file_list.size() << " file in list" << endl;
    cerr << "Need " << MINIMUM_TARGET_LIST_FILES << " files" << endl;
    
    return false;
  }
  
  // Process all of the files

  vector< string >::const_iterator file_iter;
  bool first_file = true;
  int file_num = 1;

  if (_debug)
    cerr << endl << "Going through all the files:" << endl;

  MdvxField *av_i_field = NULL;
  MdvxField *av_q_field = NULL;
  MdvxField *dif_from_ref_i_field = NULL;
  MdvxField *dif_from_ref_q_field = NULL;

  int num_azim = 0;
  int num_gates = 0;
  int scan_size = 0;

  for (file_iter = file_list.begin(); file_iter != file_list.end();
       ++file_iter, ++file_num)
  {
    if (_debug)
      cerr << "   " << file_num << " - " << *file_iter << endl;

    // Read in the data from the file

    DsMdvx input_file;
    
    if (!_inputHandler->getNextScan(*file_iter, input_file))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading input file:" << *file_iter << endl;
    
      return false;
    }
  
    // Get the current file information

    Mdvx::field_header_t i_field_hdr =
      input_file.getField(Input::I_FIELD_INDEX)->getFieldHeader();
    Mdvx::field_header_t q_field_hdr =
      input_file.getField(Input::Q_FIELD_INDEX)->getFieldHeader();
    
    num_azim = i_field_hdr.ny;
    num_gates = i_field_hdr.nx;
    scan_size = num_gates * num_azim;
      
    // Make sure the gate spacing is the same for the fields.

    if (i_field_hdr.grid_dx != q_field_hdr.grid_dx)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Gate spacing does not match between fields:" << endl;
      cerr << "    I field gate spacing = " << i_field_hdr.grid_dx << endl;
      cerr << "    Q field gate spacing = " << q_field_hdr.grid_dx << endl;
	
      return false;
    }
      
    // Initialize things and process the first file.

    if (first_file)
    {
      // Save the gate spacing value so we can make sure it doesn't change
      // between scans.

      gate_spacing = i_field_hdr.grid_dx;
      
      // Allocate space for global fields

      if (!_allocateGlobalFields(input_file))
	return false;

      // Allocate space for local fields

      av_i_field = _createField(*(input_file.getField(0)),
				"av_i", "av_i", "none");
      av_q_field = _createField(*(input_file.getField(0)),
				"av_q", "av_q", "none");
      dif_from_ref_i_field =
	_createField(*(input_file.getField(0)),
		     "dif_from_ref_i", "dif_from_ref_i", "none");
      dif_from_ref_q_field =
	_createField(*(input_file.getField(0)),
		     "dif_from_ref_q", "dif_from_ref_q", "none");
      
      // Process the first file.  This really just initializes the av_i
      // and av_q fields to start the process.

      if (!_findReliableTargets(input_file, *av_i_field, *av_q_field))
	return false;
      
      if (_writeDebugMdvFiles)
      {
	DsMdvx debug_file;
  
	_updateDebugMasterHdr(debug_file, input_file);
      
	debug_file.addField(new MdvxField(*(input_file.getField(Input::I_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*(input_file.getField(Input::Q_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*(input_file.getField(Input::SNR_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*_meanSnrField));
	debug_file.addField(new MdvxField(*av_i_field));
	debug_file.addField(new MdvxField(*av_q_field));
	
	if (debug_file.writeToDir(_debugMdvUrl) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error writing debug MDV file to URL: "
	       << _debugMdvUrl << endl;
	  cerr << debug_file.getErrStr() << endl;
	}
      }
      
      first_file = false;
    }
    else
    {
      // Make sure the gate spacing hasn't changed between scans

      if (i_field_hdr.grid_dx != gate_spacing)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Gate spacing not constant between scans:" << endl;
	cerr << "    current scan gate spacing = " << i_field_hdr.grid_dx << endl;
	cerr << "    previous scan gate spacing = " << gate_spacing << endl;
	
        if (av_i_field) {
          delete av_i_field;
        }
        if (av_q_field) {
          delete av_q_field;
        }
	
	return false;
      }
      
      // Add in the information from this file

      if (!_findReliableTargets(input_file, *av_i_field, *av_q_field,
				*dif_from_ref_i_field, *dif_from_ref_q_field))
	return false;
    
      if (_writeDebugMdvFiles)
      {
	DsMdvx debug_file;
  
	_updateDebugMasterHdr(debug_file, input_file);
      
	debug_file.addField(new MdvxField(*(input_file.getField(Input::I_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*(input_file.getField(Input::Q_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*(input_file.getField(Input::SNR_FIELD_INDEX))));
	debug_file.addField(new MdvxField(*_meanSnrField));
	debug_file.addField(new MdvxField(*av_i_field));
	debug_file.addField(new MdvxField(*av_q_field));
	debug_file.addField(new MdvxField(*dif_from_ref_i_field));
	debug_file.addField(new MdvxField(*dif_from_ref_q_field));
	debug_file.addField(new MdvxField(*_difPrevScanIField));
	debug_file.addField(new MdvxField(*_difPrevScanQField));
	debug_file.addField(new MdvxField(*_sumAField));
	debug_file.addField(new MdvxField(*_sumBField));
	debug_file.addField(new MdvxField(*_sumPField));
      
	if (debug_file.writeToDir(_debugMdvUrl) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error writing debug MDV file to URL: "
	       << _debugMdvUrl << endl;
	  cerr << debug_file.getErrStr() << endl;
	}
      }
    }
    
    // Transfer old I/Q data in _difPrevScan to be ready for the next round

    MdvxField *i_field = input_file.getField(Input::I_FIELD_INDEX);
    fl32 *i_data = (fl32 *)i_field->getVol();
  
    MdvxField *q_field = input_file.getField(Input::Q_FIELD_INDEX);
    fl32 *q_data = (fl32 *)q_field->getVol();
  
    MdvxField *snr_field = input_file.getField(Input::SNR_FIELD_INDEX);
  
    delete _difPrevScanIField;
    _difPrevScanIField = _createField(*i_field,
				      "dif_prev_scan_i", "dif_prev_scan_i",
				      "none", i_data);
      
    delete _difPrevScanQField;
    _difPrevScanQField = _createField(*q_field,
				      "dif_prev_scan_q", "dif_prev_scan_q",
				      "none", q_data);

    delete _oldSnrField;
    _oldSnrField = new MdvxField(*snr_field);
  } /* endfor - file_iter */

  // Do the actual computation of reliability based on how "coherently" the
  // _difPrevScan were summed up.  The calculation is very much like
  // that of normalized lag 1 power (NCP) in pulse-pair computation.

  if (_debug)
    cerr << "Now completing the calculation of reliability..." << endl;
  
  fl32 *av_i_data = (fl32 *)av_i_field->getVol();
  fl32 *av_q_data = (fl32 *)av_q_field->getVol();
  
  float *phase_targ = new float[scan_size];

  fl32 *strength_data = (fl32 *)_calibStrengthField->getVol();
  fl32 *phase_er_data = (fl32 *)_calibPhaseErField->getVol();
  fl32 *quality_data = (fl32 *)_calibQualityField->getVol();
  fl32 *ncp_data = (fl32 *)_calibNcpField->getVol();
  fl32 *mean_snr_data = (fl32 *)_meanSnrField->getVol();
  fl32 *sum_a_data = (fl32 *)_sumAField->getVol();
  fl32 *sum_b_data = (fl32 *)_sumBField->getVol();
  fl32 *sum_p_data = (fl32 *)_sumPField->getVol();
  
  // Mdvx::field_header_t old_snr_field_hdr = _oldSnrField->getFieldHeader();
  fl32 *old_snr_data = (fl32 *)_oldSnrField->getVol();
  
  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    for (int r = 0; r < num_gates; ++r, ++index)
    {
      if (av_i_data[index] == 0.0 && av_q_data[index] == 0.0)
	phase_targ[index] = INVALID;
      else
	phase_targ[index] =
	  atan2(av_q_data[index], av_i_data[index]) / DEG_TO_RAD;

      if (sum_p_data[index] == 0.0 || _pixelCount[index] == 0)
      {
	strength_data[index] = INVALID;
      }
      else
      {
	strength_data[index] = mean_snr_data[index] / (float)_pixelCount[index];

	float ncp = (SQR(sum_a_data[index]) + SQR(sum_b_data[index])) /
	  sum_p_data[index] / _pixelCount[index];

	if (ncp != 0.0 && r >= _rMin)
	{
	  if (_pixelCount[index] > 1)
	  {
	    ncp = (ncp - 1.0 / sqrt((float)_pixelCount[index])) /
	      (1.0 - 1.0 / sqrt((float)_pixelCount[index]));

	    if (ncp < 0.001)
	      ncp = 0.001;
	    if (ncp > 0.9999)
	      ncp = 0.9999;
	  }
	  else
	  {
	    ncp = 0.5;
	  }
	  
	  // Add to it a correction for ground echo SNR and its variability

	  ncp *= 1.0 / ( 1.0 + pow(10.0, -0.1 * (strength_data[index])));
	  _fluctSnr[index] /= (float)_pixelCount[index];

	  float strength_correction =
	    exp(-0.001 * pow((double)_fluctSnr[index],
			     (double)4.0)) / exp(-0.002);
	  if (strength_correction > 1.0)
	    strength_correction = 1.0;
	  if (strength_correction < 0.1)
	    strength_correction = 0.1;
	  
	  ncp *= strength_correction;
	  if (ncp > 0.0)
	  {
	    phase_er_data[index] = sqrt(-2.0 * log(ncp) / ncp) / DEG_TO_RAD;
	  }
	  else
	  {
	    ncp = 0.0;
	    phase_er_data[index] = VERY_LARGE;
	  }
	  
	  ncp_data[index] = ncp;
	}
      }

    } /* endfor - r */
  } /* endfor - az */
  
  // Write some debug output files

  _writeDebugPhaseCalibFiles(phase_targ, num_azim, num_gates);
  
  // And try to eliminate (or at least dampen the accuracy of) echoes that
  // could be the result of sidelobes.  Echoes that are around or below
  // SideLobePow dB of the integrated power strength for all azimuths at a
  // given range are affected, as well as those that are immediately next to
  // much stronger targets.

  float contamin_pow =
    exp(log(4.0) * SQR(360.0 / (float)num_azim / _beamWidth)) ;

  // First, look for mainlobe contamination on nearby azimuths

  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    int r;

    for (r = _rMin, index += _rMin; r < num_gates; ++r, ++index)
    {
      if (ncp_data[index] > 0.0)
      {
	// Get the power at the current gate

	float local_pow = pow(10.0, 0.1 * strength_data[index]);

	// Get the power at the gates in the surrounding beams

	int prev_az = (az + num_azim - 1) % num_azim;
	float near_pow =
	  pow(10.0, 0.1 * strength_data[prev_az * num_gates + r]);

	int next_az = (az + 1) % _numAzim;
	near_pow +=
	  pow(10.0, 0.1 * strength_data[next_az * num_gates + r]) ;

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  float side_correction =
	    exp(-0.5 * SQR((near_pow - 2.5 * contamin_pow) /
			   (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	    side_correction = 0.1;

	  ncp_data[index] *= side_correction;
	  if (ncp_data[index] != 0.0)
	    phase_er_data[index] =
	      sqrt(-2.0 * log(ncp_data[index]) / ncp_data[index]) / DEG_TO_RAD;
	  else
	    phase_er_data[index] = VERY_LARGE;
	}
      }
    } /* endfor - r */
  } /* endfor - az, index */
  
  // #### Assumption: PulseLength = GateSpacing with a matched filter

  contamin_pow = 4.0;

  // Then, look for possible range sidelobes

  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    int r;
    for (r = _rMin, index += _rMin; r < num_gates; ++r, ++index)
    {
      if (ncp_data[index] > 0.0)
      {
	// Get the power at the current gate

	float local_pow = pow(10.0, 0.1 * strength_data[index]);

	// Get the power at the surrounding gates along this beam

	float near_pow;
	
	if (r == 0)
	  near_pow = VERY_LARGE;
	else
	  near_pow = pow(10.0, 0.1 * strength_data[index-1]);

	if (r < num_gates - 1)
	  near_pow += pow(10.0, 0.1 * strength_data[index+1]);

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  float side_correction =
	    exp(-0.5 * SQR((near_pow - 2.5 * contamin_pow) /
			   (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	    side_correction = 0.1;

	  ncp_data[index] *= side_correction;
	  if (ncp_data[index] != 0.0)
	    phase_er_data[index] =
	      sqrt(-2.0 * log(ncp_data[index]) / ncp_data[index]) / DEG_TO_RAD;
	  else
	    phase_er_data[index] = VERY_LARGE;
	}
      }
    } /* endfor - r */
  } /* endfor - az, index */
  

  // Finally handle 360 deg sidelobes

  for (int r = _rMin; r < num_gates; ++r)
  {
    // Sum the power values at all gates the same distance as this gate
    // around the radar.  At this point, _oldSnr contains the SNR data
    // of the last radar file read in.

    float sum_pow = 0.0;

    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
      sum_pow += pow(10.0, 0.1 * old_snr_data[index]);

    // Calculate the side power

    float side_pow = sum_pow * pow(10.0, 0.1 * _sideLobePower);

    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
    {
      if (ncp_data[index] > 0.0)
      {
	float side_correction =
	  exp(-side_pow * pow(10.0, -0.1 * strength_data[index]));

	ncp_data[index] *= side_correction;

	if (ncp_data[index] != 0.0)
	  phase_er_data[index] =
	    sqrt(-2.0 * log(ncp_data[index]) / ncp_data[index]) / DEG_TO_RAD;
	else
	  phase_er_data[index] = VERY_LARGE;
      }
    }
  }

  // Calculate the quality field which is the second field displayed
  // in n_viewcalib.  This field is not included in the original
  // calibration files, but is included in the new ones so it can be
  // displayed in CIDD

  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    for (int r = 0; r < num_gates; ++r, ++index)
    {
      if (ncp_data[index] != INVALID)
	quality_data[index] = sqrt(SQR(ncp_data[index]));
      
    } /* endfor - r */
  } /* endfor - az */
  
  if (_debug)
    cerr << "Task completed." << endl;
  
  if (av_i_field) {
    delete av_i_field;
  }
  if (av_q_field) {
    delete av_q_field;
  }
  delete [] phase_targ;
  if (dif_from_ref_i_field) {
    delete dif_from_ref_i_field;
  }
  if (dif_from_ref_q_field) {
    delete dif_from_ref_q_field;
  }
  
  return true;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _allocateGlobalFields()
 */

bool Processor::_allocateGlobalFields(const DsMdvx &input_file)
{
  static const string method_name = "Processor::_allocateGlobalFields()";
  
  // Get the needed information from the base field

  MdvxField *base_field = input_file.getField(0);

  Mdvx::field_header_t base_field_hdr = base_field->getFieldHeader();
  
  int scan_size = base_field_hdr.nx * base_field_hdr.ny;
  
  // Allocate the global fields

  _fluctSnr = new float[scan_size];
  memset(_fluctSnr, 0, scan_size * sizeof(float));

  delete _meanSnrField;
  _meanSnrField = _createField(*base_field,
			       "mean_snr", "mean_snr", "dB", 0.0);
  
  _pixelCount = new int[scan_size];
  memset(_pixelCount, 0, scan_size * sizeof(int));

  delete _sumAField;
  _sumAField = _createField(*base_field, "sum_a", "sum_a", "none", 0.0);

  delete _sumBField;
  _sumBField = _createField(*base_field, "sum_b", "sum_b", "none", 0.0);

  delete _sumPField;
  _sumPField = _createField(*base_field, "sum_p", "sum_p", "none", 0.0);

  delete _difPrevScanIField;
  MdvxField *i_field = input_file.getField(Input::I_FIELD_INDEX);
  _difPrevScanIField = _createField(*i_field,
				    "dif_prev_scan_i", "dif_prev_scan_i",
				    "none", (fl32 *)i_field->getVol());
      
  delete _difPrevScanQField;
  MdvxField *q_field = input_file.getField(Input::Q_FIELD_INDEX); 
  _difPrevScanQField = _createField(*q_field,
				    "dif_prev_scan_q", "dif_prev_scan_q",
				    "none", (fl32 *)q_field->getVol());

  _calibStrengthField = _createField(*base_field,
				     "strength", "strength", "none", 0.0);
  _calibAvIField = _createField(*base_field, "av_i", "av_i", "none", 0.0);
  _calibAvQField = _createField(*base_field, "av_q", "av_q", "none", 0.0);
  _calibPhaseErField = _createField(*base_field, "phase_er", "phase_er",
				    "none", VERY_LARGE);
  _calibQualityField = _createField(*base_field, "quality", "quality",
				    "none");
  _calibNcpField = _createField(*base_field, "ncp", "ncp", "none");
  
  return true;
}


/*********************************************************************
 * _createField()
 */

MdvxField *Processor::_createField(const MdvxField &base_field,
				   const string &field_name,
				   const string &field_name_long,
				   const string &units) const
{
  static const string method_name = "Processor::_createField()";
  
  // Create the new field header

  Mdvx::field_header_t field_hdr = base_field.getFieldHeader();
  
  // Create the new vlevel header

  Mdvx::vlevel_header_t vlevel_hdr = base_field.getVlevelHeader();

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = INVALID;
  field_hdr.missing_data_value = INVALID;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Create the new field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


MdvxField *Processor::_createField(const MdvxField &base_field,
				   const string &field_name,
				   const string &field_name_long,
				   const string &units,
				   const fl32 initial_data_value) const
{
  static const string method_name = "Processor::_createField()";
  
  // Create the field

  MdvxField *field = _createField(base_field, field_name, field_name_long,
				       units);
  
  if (field == 0)
    return 0;
  
  // Set the initial data values

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *data = (fl32 *)field->getVol();
  
  for (int i = 0; i < field_hdr.nx * field_hdr.ny; ++i)
    data[i] = initial_data_value;
  
  return field;
}

MdvxField *Processor::_createField(const MdvxField &base_field,
				   const string &field_name,
				   const string &field_name_long,
				   const string &units,
				   const fl32 *initial_data) const
{
  static const string method_name = "Processor::_createField()";
  
  // Create the field

  MdvxField *field = _createField(base_field, field_name, field_name_long,
				       units);
  
  if (field == 0)
    return 0;
  
  // Set the initial data values

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *data = (fl32 *)field->getVol();
  
  for (int i = 0; i < field_hdr.nx * field_hdr.ny; ++i)
    data[i] = initial_data[i];
  
  return field;
}


/*********************************************************************
 * _findReliableTargets()
 */

bool Processor::_findReliableTargets(const DsMdvx &input_file,
				     MdvxField &av_i_field,
				     MdvxField &av_q_field) const
{
  static const string method_name = "Processor::_findReliableTargets()";
  
  // Get pointers into the data.  Note that we ensure in the Input class
  // that the missing I/Q data values are set to 0.0.

  MdvxField *i_field = input_file.getField(Input::I_FIELD_INDEX);
  Mdvx::field_header_t i_field_hdr = i_field->getFieldHeader();
  fl32 *i_data = (fl32 *)i_field->getVol();
  
  MdvxField *q_field = input_file.getField(Input::Q_FIELD_INDEX);
  // Mdvx::field_header_t q_field_hdr = q_field->getFieldHeader();
  fl32 *q_data = (fl32 *)q_field->getVol();
  
  fl32 *av_i_data = (fl32 *)av_i_field.getVol();
  fl32 *av_q_data = (fl32 *)av_q_field.getVol();
  
  int scan_size = i_field_hdr.nx * i_field_hdr.ny;
  
  for (int i = 0; i < scan_size; ++i)
  {
    av_i_data[i] = i_data[i];
    av_q_data[i] = q_data[i];
  }

  return true;
}


bool Processor::_findReliableTargets(const DsMdvx &input_file,
				     MdvxField &av_i_field,
				     MdvxField &av_q_field,
				     MdvxField &dif_from_ref_i_field,
				     MdvxField &dif_from_ref_q_field) const
{
  static const string method_name = "Processor::_findReliableTargets()";
  
  // Get pointers into the data.  Note that we ensure in the Input class
  // that the missing I/Q data values are set to 0.0.

  MdvxField *i_field = input_file.getField(Input::I_FIELD_INDEX);
  Mdvx::field_header_t i_field_hdr = i_field->getFieldHeader();
  fl32 *i_data = (fl32 *)i_field->getVol();
  
  MdvxField *q_field = input_file.getField(Input::Q_FIELD_INDEX);
  // Mdvx::field_header_t q_field_hdr = q_field->getFieldHeader();
  fl32 *q_data = (fl32 *)q_field->getVol();
  
  MdvxField *snr_field = input_file.getField(Input::SNR_FIELD_INDEX);
  Mdvx::field_header_t snr_field_hdr = snr_field->getFieldHeader();
  fl32 *snr_data = (fl32 *)snr_field->getVol();
  
  Mdvx::field_header_t old_snr_field_hdr = _oldSnrField->getFieldHeader();
  fl32 *old_snr_data = (fl32 *)_oldSnrField->getVol();
  
  fl32 *av_i_data = (fl32 *)av_i_field.getVol();
  fl32 *av_q_data = (fl32 *)av_q_field.getVol();
  fl32 *dif_i_data = (fl32 *)dif_from_ref_i_field.getVol();
  fl32 *dif_q_data = (fl32 *)dif_from_ref_q_field.getVol();
  fl32 *dif_prev_i_data = (fl32 *)_difPrevScanIField->getVol();
  fl32 *dif_prev_q_data = (fl32 *)_difPrevScanQField->getVol();
  fl32 *mean_snr_data = (fl32 *)_meanSnrField->getVol();
  fl32 *sum_a_data = (fl32 *)_sumAField->getVol();
  fl32 *sum_b_data = (fl32 *)_sumBField->getVol();
  fl32 *sum_p_data = (fl32 *)_sumPField->getVol();
  
  int num_azim = i_field_hdr.ny;
  int num_gates = i_field_hdr.nx;
  int scan_size = num_azim * num_gates;
  
  // Compute phase difference wrt previous scan (now in _difPrevScan_i/q).
  // Normalize I & Q, with some snr-based weighting.
  // (Somewhat similar to beginning of get_quality())

  for (int index = 0; index < scan_size; ++index)
  {
    av_i_data[index] += i_data[index];
    av_q_data[index] += q_data[index];

    float tmp_a = dif_prev_i_data[index];
    float tmp_b = dif_prev_q_data[index];

    dif_prev_i_data[index] =
      tmp_a * i_data[index] + tmp_b * q_data[index];
    dif_prev_q_data[index] =
      tmp_a * q_data[index] - tmp_b * i_data[index];

    float norm = sqrt(SQR(dif_prev_i_data[index]) +
		      SQR(dif_prev_q_data[index]));

    if (snr_data[index] != snr_field_hdr.bad_data_value &&
	snr_data[index] != snr_field_hdr.missing_data_value)
    {
      float strength_correction =
	1.0 / (1.0 + pow(10.0, -0.1 * snr_data[index]));

      if (strength_correction != 0.0)
      {
//	cerr << "*** strength_corr[" << index << "] = " << strength_correction << endl;
	
	if (strength_correction > 0.001)
	  norm /= strength_correction;
	else
	  norm = VERY_LARGE;
      }
    }

    if (norm != 0.0)
    {
//      cerr << "    norm[" << index << "] = " << norm << endl;
      
      dif_prev_i_data[index] /= norm;
      dif_prev_q_data[index] /= norm;
    }
  } /* endfor - index */
    
  // Smooth (a lot!) _difPrevScan in dif_from_ref.  As a result,
  // dif_from_ref will map changes of phase over large areas (due to
  // meteorology) which could then be compensated for later.  First do it
  // at close range (average across all azimuths and a few range gates).

  int max_r_smooth = _rMin + (num_gates / 10);
  if (max_r_smooth >= num_gates)
    max_r_smooth = num_gates - 1;
  
  for (int r = _rMin; r <= max_r_smooth; ++r)
  {
    dif_i_data[r] = 0.0;
    dif_q_data[r] = 0.0;

    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
    {
      dif_i_data[r] += dif_prev_i_data[index];
      dif_q_data[r] += dif_prev_q_data[index];
    }
  } /* endfor - r */

  for (int r = _rMin; r < max_r_smooth; ++r)
  {
    float tmp_a = dif_i_data[r-1] + dif_i_data[r] + dif_i_data[r+1];
    float tmp_b = dif_q_data[r-1] + dif_q_data[r] + dif_q_data[r+1];

    float norm = sqrt(SQR(tmp_a) + SQR(tmp_b));

    if( norm != 0.0)
    {
      tmp_a /= norm;
      tmp_b /= norm;
    }

    for (int az = 1, index = num_gates + r;
	 az < num_azim; ++az, index += num_gates)
    {
      dif_i_data[index] = tmp_a;
      dif_q_data[index] = tmp_b;
    } /* endfor - az */
  } /* endfor - r */

  for (int r = _rMin; r < max_r_smooth; ++r)
  {
    dif_i_data[r] = dif_i_data[r + num_gates];
    dif_q_data[r] = dif_q_data[r + num_gates];
  } /* endfor - r */

  // And then to ranges further away (sector average).

  for (int r = max_r_smooth; r < num_gates; ++r)
  {
    double smooth_i = 0.0;
    double smooth_q = 0.0;
      
    for (int dr = -num_gates / 10; dr <= num_gates / 10; ++dr)
    {
      if (r + dr < num_gates)
      {
	for (int daz = -num_azim / 16; daz <= num_azim / 16; ++daz)
	{
	  int meas_az = daz;

	  if (meas_az < 0)
	    meas_az += num_azim;
	  if (meas_az >= num_azim)
	    meas_az -= num_azim;

	  int index = meas_az * num_gates + r + dr;
	  smooth_i += dif_prev_i_data[index];
	  smooth_q += dif_prev_q_data[index];
	} /* endfor - daz */
      }
    } /* endfor - dr */

    dif_i_data[r] = smooth_i;
    dif_q_data[r] = smooth_q;

    for (int az = 1; az < num_azim; ++az)
    {
      for (int dr = -num_gates / 10; dr <= num_gates / 10; ++dr)
      {
	if (r + dr < num_gates)
	{
	  int meas_az = az - 1 - (num_azim / 16);
	  
	  if (meas_az < 0)
	    meas_az += num_azim;

	  int index = meas_az * num_gates + r + dr;
	  smooth_i -= dif_prev_i_data[index];
	  smooth_q -= dif_prev_q_data[index];

	  meas_az = az + (num_azim / 16);

	  if (meas_az >= num_azim)
	    meas_az -= num_azim;

	  index = meas_az * num_gates + r + dr;
	  smooth_i += dif_prev_i_data[index];
	  smooth_q += dif_prev_q_data[index];
	}
      } /* endfor - dr */
	
      int index = az * num_gates + r;
      dif_i_data[index] = smooth_i;
      dif_q_data[index] = smooth_q;
    } /* endfor - az */

    for (int az = 0; az < num_azim; ++az)
    {
      int index = az * num_gates + r;
      float norm = sqrt(SQR(dif_i_data[index]) + SQR(dif_q_data[index]));

      if (norm != 0.0)
      {
	dif_i_data[index] /= norm ;
	dif_q_data[index] /= norm ;
      }
    } /* endfor - az */
  } /* endfor - r */

  // Smooth phase field done.  Now compensate the previously computed
  // phase difference field for the average change caused by meteorology
  // (i.e. the smoothed phase field).

  for (int index = 0; index < scan_size; ++index)
  {
    float tmp_a = dif_prev_i_data[index];
    float tmp_b = dif_prev_q_data[index];

    dif_prev_i_data[index] =
      tmp_a * dif_i_data[index] + tmp_b * dif_q_data[index];
    dif_prev_q_data[index] =
      tmp_b * dif_i_data[index] - tmp_a * dif_q_data[index];
  } /* endfor - index */
    
  // Sum up the _difPrevScan_i/q in sum_a/b and sum_p (to be used later to
  // compute, in conjunction with sum_a/b the actual reliability).

  for (int index = 0; index < scan_size; ++index)
  {
    sum_a_data[index] += dif_prev_i_data[index];
    sum_b_data[index] += dif_prev_q_data[index];
    sum_p_data[index] +=
      SQR(dif_prev_i_data[index]) + SQR(dif_prev_q_data[index]);

    if (snr_data[index] != snr_field_hdr.bad_data_value &&
	snr_data[index] != snr_field_hdr.missing_data_value &&
	old_snr_data[index] != old_snr_field_hdr.bad_data_value &&
	old_snr_data[index] != old_snr_field_hdr.missing_data_value)
    {
      _fluctSnr[index] += fabs(snr_data[index] - old_snr_data[index]);
      mean_snr_data[index] += snr_data[index];
      _pixelCount[index]++;
    }
  } /* endfor - index */

  return true;
}


/*********************************************************************
 * _updateDebugMdvMasterHdr()
 */

void Processor::_updateDebugMasterHdr(DsMdvx &debug_file,
				      const DsMdvx &input_file) const
{
  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();

  debug_file.setMasterHeader(master_hdr);
}


/*********************************************************************
 * _writeCalibrationFile()
 */

bool Processor::_writeCalibrationFile(const DateTime &data_time) const
{
  static const string method_name = "Processor::_writeCalibrationFile()";

  // Write the calibration information to an MDV file

  Mdvx::field_header_t field_hdr = _calibStrengthField->getFieldHeader();

  DsMdvx calib_file;
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time.utime();
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = data_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.user_data_fl32[0] = (fl32)_nValue;
  master_hdr.sensor_lon = field_hdr.proj_origin_lon;
  master_hdr.sensor_lat = field_hdr.proj_origin_lat;
  master_hdr.sensor_alt = 0.0;
  STRcopy(master_hdr.data_set_info, "Refractivity calibration file",
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "RefractCalib", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "RefractCalib", MDV_NAME_LEN);
  
  calib_file.setMasterHeader(master_hdr);
  
  calib_file.addField(_calibStrengthField);
  calib_file.addField(_calibAvIField);
  calib_file.addField(_calibAvQField);
  calib_file.addField(_calibPhaseErField);
  calib_file.addField(_calibQualityField);
  calib_file.addField(_calibNcpField);
  
  if (calib_file.writeToPath(_refFilePath.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing calibration MDV file to path: "
	 << _refFilePath << endl;
    cerr << calib_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeDebugPhaseCalibFiles()
 */

void Processor::_writeDebugPhaseCalibFiles(const float *phase_targ,
					   const int num_azim,
					   const int num_gates) const
{
  static const string method_name = "Processor::_writeDebugPhaseCalibFiles()";
  
  int scan_size = num_azim * num_gates;
  
  // Write the data file

  FILE *debug_phasecalib_dat_fp;

  if ((debug_phasecalib_dat_fp = fopen("debug_phasecalib.dat", "wb")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening debug_phasecalib.dat file for writing" << endl;
  }
  else
  {
    fwrite(phase_targ, sizeof(float), scan_size, debug_phasecalib_dat_fp);
    fclose(debug_phasecalib_dat_fp);
  }
  
  // Write the image file

  FILE *debug_phasecalib_ppm_fp;

  if ((debug_phasecalib_ppm_fp = fopen("debug_phasecalib.ppm", "wb")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening debug_phasecalib.ppm file for writing" << endl;
  }
  else
  {
    fprintf(debug_phasecalib_ppm_fp, "P5 %d %d 255 ",
	    (int)num_gates, (int)num_azim);
    for (int index = 0; index < scan_size; ++index)
    {
      char c;
    
      if (phase_targ[index] == INVALID)
	c = 0;
      else
	c = (char)(phase_targ[index] * 127.0 / 180.0);
      fwrite(&c, sizeof(char), 1, debug_phasecalib_ppm_fp);
    }
    fclose(debug_phasecalib_ppm_fp);
  }
  
}
