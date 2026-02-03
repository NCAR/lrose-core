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

#include "Processor.hh"
#include "PhaseFit.hh"
#include "Input.hh"
#include <Refract/RefractConstants.hh>
#include <Refract/FieldWithData.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/str.h>
#include <rapmath/math_macros.h>
#include <cmath>

// Global variables

const double Processor::C_VACUUM = 299792458.0;
const double Processor::ABRUPT_FACTOR = 4.0;
const double Processor::THRESH_WIDTH = 5.0;

const double Processor::MIN_N_VALUE = -200.0;
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
  _calib(0),
  _refparms(),
  _parms(),
  _firstFile(true),
  _nptScan(0),
  _first(true)
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

bool Processor::init(CalibDayNight *calib, const RefParms &refparms,
		     const Params &params)
{
  // Copy the paramter values

  _calib = calib;
  _refparms = refparms;
  _parms = params;

  // Check the parameter values in the parameter file for consistency
  if (params.r_min >= refparms.num_range_bins)
  {
    LOG(ERROR) << "r_min value (" << params.r_min << ") out of range.";
    LOG(ERROR) << "Must be less than num_range_bins ("
	       << _refparms.num_range_bins;
    return false;
  }
  
  _wavelength = C_VACUUM / params.frequency;
  _first = true;
  _nptScan = 0;
  return true;
}


/*********************************************************************
 * processScan()
 */
bool Processor::processScan(const RefractInput &input, const time_t &t,
			    DsMdvx &data_file)
{
  // Get pointers to the data fields.  We know that all of the fields
  // exist if we get to this point.
  FieldWithData I = input.getI(data_file);
  FieldWithData Q = input.getQ(data_file);
  FieldDataPair iq(I, Q);
  FieldWithData Qual = input.getQuality(data_file);
  FieldWithData Snr = input.getSNR(data_file);
  
  // Create the output fields
  if (_first)
  {
    if (!_createOutputFields(iq))
    {
      return false;
    }
    _first = false;
  }
  
  // Copy the raw phase data.  Set missing data values to 0.0 as needed
  // for the algorithm.
  _rawPhase.copyIQFilterBadOrMissing(iq);

  FieldWithData qualityField(I.getFieldPtrConst(), "Quality", "none", 0.0);

  // Compute the quality
  _getQuality(Snr, Qual, t, qualityField);
  
  // Compute phase diff. fields
  LOG(DEBUG_VERBOSE) << "Computing phase differences...";
  if (!_difPhase(t))
  {
    return false;
  }
  
  // Fit N & DN field to phase data

  LOG(DEBUG_VERBOSE) << "Generating N field...";
  if (!_fitPhases(iq.numAzim(), iq.numGates(), iq.gateSpacing()*1000.0))
  {
    return false;
  }
  
  _firstFile = false;
  
  // Add copies of the output fields to the data file.  Use the I field
  // to mask out missing beams.
  MdvxField  *sigmaN = _maskOutputField(*(iq.getI()), _sigmaNField,
					MIN_SIGMA_N_VALUE, MAX_SIGMA_N_VALUE);

  if (_parms.threshold_using_sigma_n)
  {
    data_file.addField(_maskOutputField(*(iq.getI()), *sigmaN,
					_parms.max_sigma_n, _nField,
					MIN_N_VALUE, MAX_N_VALUE, qualityField,
					_parms.quality_threshold));
    data_file.addField(_maskOutputField(*(iq.getI()), *sigmaN,
					_parms.max_sigma_n, _dnField,
					MIN_DN_VALUE, MAX_DN_VALUE,
					qualityField,
					_parms.quality_threshold));
  }
  else
  {
    data_file.addField(_maskOutputField(*(iq.getI()), *sigmaN, _nField,
					MIN_N_VALUE, MAX_N_VALUE, qualityField,
					_parms.quality_threshold));
    data_file.addField(_maskOutputField(*(iq.getI()), *sigmaN, _dnField,
					MIN_DN_VALUE, MAX_DN_VALUE,
					qualityField,
					_parms.quality_threshold));
  }
    

  // mask the N and DN fields both from iq and from sigmaN

  data_file.addField(_maskOutputField(*(iq.getI()), _sigmaDnField,
				      MIN_SIGMA_DN_VALUE, MAX_SIGMA_DN_VALUE));
  data_file.addField(_maskOutputField(*(iq.getI()), qualityField, 0, 1));
  
  data_file.addField(sigmaN);

  // Transfer old I/Q data to _difPrevScan to get ready for next scan
  _difPrevScan.copyIQ(_rawPhase);
  return true;
}


/*********************************************************************
 * _createOutputFields()
 */

bool Processor::_createOutputFields(const FieldDataPair &iq)
{
  // If the fields already exist, keep them
  _nptScan = iq.scanSize();
  _difFromRef = FieldDataPair(iq, "difFromRef_I", "none", 0.0, "difFromRef_Q",
			      "none", 0.0);
  _difPrevScan = FieldDataPair(iq,"difPrev_I", "none", 0.0, "difPrev_Q", "none",
			       0.0);
  _rawPhase = FieldDataPair(iq, "rawPhase_I", "none", 0.0, "rawPhase_Q", "none",
			    0.0);

  _target = TargetVector(_nptScan);
  _nField = iq.createFromI("N", "none");
  _dnField = iq.createFromI("DELTA_N", "none");
  _sigmaNField = iq.createFromI("SIGMA_N", "none");
  _sigmaDnField = iq.createFromI("SIGMA_DN", "none");
  return true;
}

/*********************************************************************
 * _difPhase()
 *
 * Computes the phase difference between the last two scans as well as
 * between the current scan and the reference scan.  These two arrays will be
 * the base data for the processing to follow elsewhere.
 */
bool Processor::_difPhase(const time_t &t)
{
  LOG(DEBUG) << "And phase / phase difference arrays";

  const FieldDataPair calib_av_iq = _calib->avIqPtr(t);

  _difPrevScan.phaseDiffV(_rawPhase);

  vector<double> norm = _difPrevScan.createNormSquaredVector();
  for (int i=0; i<_nptScan; ++i)
  {
    norm[i] = pow(norm[i], 0.375);
  }
  _difPrevScan.normalizeWithVector(norm);
  _difFromRef.phaseDiffV(calib_av_iq, _rawPhase);

  _target.compute_phase_diff(_difPrevScan, norm, _difFromRef);

  // And normalize for the size of calib_av_i/q
  norm = calib_av_iq.createNormVector();
  _difFromRef.normalizeWithVector(norm);
  _target.setIQ(_difFromRef);
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

bool Processor::_fitPhases(int numBeams, int numGates,  double gate_spacing)
{
  // Create the phase fitting object

  PhaseFit phase_fit;

  if (!phase_fit.init(numBeams, numGates, gate_spacing,
		      _wavelength, _parms.min_consistency, _parms.r_min))

    return false;
  
  // Start with deltaN.  Note that we can only calculating deltaN if we
  // are on our second scan.
  if (!_firstFile)
  {
    LOG(DEBUG) << "Fitting the difference between last two scans";
    phase_fit.initFit(_target, _difPrevScan, true);
    phase_fit.setRefN(0.0);
    phase_fit.setSmoothSideLen(_parms.dn_smoothing_side_len);
    phase_fit.setNOutput(_dnField.getDataPtr());
    phase_fit.setNError((_sigmaDnField.getDataPtr()));
    
    // Perform the fit for DN
    if (!phase_fit.fitPhaseField(_parms.do_relax))
    {
      _firstFile = true;
      return false;
    }
    
  } /* endif - !_firstFile */
  
  // Now proceed with N calculation: reset the data structure
  LOG(DEBUG) << "Now fitting the absolute phase difference";
  phase_fit.initFit(_target, _difFromRef, false);
  phase_fit.setRefN(_calib->refNDay());
  phase_fit.setSmoothSideLen(_parms.n_smoothing_side_len);
  phase_fit.setNOutput(_nField.getDataPtr());
  phase_fit.setNError(_sigmaNField.getDataPtr());

  // Perform the fit for N
  if (!phase_fit.fitPhaseField(_parms.do_relax))
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

bool Processor::_getQuality(const FieldWithData &snr,
			    const FieldWithData &qual, const time_t &t,
			    FieldWithData &qualityOutputField)
{
  LOG(DEBUG) << "Computing echo quality";

  const FieldWithData calib_phase_er = _calib->phaseErPtr(t);
  const FieldDataPair calib_av_iq = _calib->avIqPtr(t);
  
  // set a quality array from snr and qual data
  vector<double> quality =
    snr.setQualityVector(qual,
			_refparms.quality_source ==RefParms::QUALITY_FROM_WIDTH,
			_refparms.quality_source == RefParms::QUALITY_FROM_CPA,
			THRESH_WIDTH, ABRUPT_FACTOR);

  for (size_t i=0; i<quality.size(); ++i)
    qualityOutputField[i] = quality[i];

  // set a phase error array from the quality array
  vector<double> phase_er =
    calib_phase_er.setPhaseErVector(quality, calib_av_iq,
				    _refparms.num_range_bins/6);

  _target.setStrengthAndPhaseErr(snr, _rawPhase, phase_er);

  _rawPhase.normalizeWithQuality(quality);
      
  return true;
}

/*********************************************************************
 * _maskOutputField()
 */

MdvxField *Processor::_maskOutputField(const MdvxField &mask_field,
				       const FieldWithData &output_field,
				       const double min_data_value,
				       const double max_data_value) const
{
  // Create a copy of the output field to add to the output file

  MdvxField *masked_field = output_field.fieldCopy();
  
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


MdvxField *Processor::_maskOutputField(const MdvxField &mask_field,
				       const FieldWithData &output_field,
				       const double min_data_value,
				       const double max_data_value,
				       const FieldWithData &quality,
				       double quality_thresh) const
{
  // Create a copy of the output field to add to the output file

  MdvxField *masked_field = output_field.fieldCopy();
  
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
    if (quality.isBadAtIndex(i))
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    if (quality[i] <= quality_thresh)
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

MdvxField *Processor::_maskOutputField(const MdvxField &mask_field,
				       const MdvxField &mask_field2,
				       const FieldWithData &output_field,
				       const double min_data_value,
				       const double max_data_value,
				       const FieldWithData &quality,
				       double quality_thresh) const
{
  // Create a copy of the output field to add to the output file

  MdvxField *masked_field = output_field.fieldCopy();
  
  // Set the output field values to missing where the mask field or mask2
  // field is missing
  Mdvx::field_header_t mask_field_hdr = mask_field.getFieldHeader();
  fl32 *mask_data = (fl32 *)mask_field.getVol();
  
  Mdvx::field_header_t output_field_hdr = masked_field->getFieldHeader();
  fl32 *output_data = (fl32 *)masked_field->getVol();
  
  int volume_size = mask_field_hdr.nx * mask_field_hdr.ny * mask_field_hdr.nz;
  
  Mdvx::field_header_t mask_field_hdr2 = mask_field2.getFieldHeader();
  fl32 *mask_data2 = (fl32 *)mask_field2.getVol();
  
  for (int i = 0; i < volume_size; ++i)
  {
    // Mask the data based on the mask field.

    if (mask_data[i] == mask_field_hdr.bad_data_value ||
	mask_data[i] == mask_field_hdr.missing_data_value ||
	mask_data2[i] == mask_field_hdr2.bad_data_value ||
	mask_data2[i] == mask_field_hdr2.missing_data_value)
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    if (quality.isBadAtIndex(i))
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    if (quality[i] <= quality_thresh)
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

MdvxField *Processor::_maskOutputField(const MdvxField &mask_field,
				       const MdvxField &mask_field2,
				       double mask_field2_threshold,
				       const FieldWithData &output_field,
				       const double min_data_value,
				       const double max_data_value,
				       const FieldWithData &quality,
				       double quality_thresh) const
{
  // Create a copy of the output field to add to the output file

  MdvxField *masked_field = output_field.fieldCopy();
  
  // Set the output field values to missing where the mask field or mask2
  // field is missing
  Mdvx::field_header_t mask_field_hdr = mask_field.getFieldHeader();
  fl32 *mask_data = (fl32 *)mask_field.getVol();
  
  Mdvx::field_header_t output_field_hdr = masked_field->getFieldHeader();
  fl32 *output_data = (fl32 *)masked_field->getVol();
  
  int volume_size = mask_field_hdr.nx * mask_field_hdr.ny * mask_field_hdr.nz;
  
  Mdvx::field_header_t mask_field_hdr2 = mask_field2.getFieldHeader();
  fl32 *mask_data2 = (fl32 *)mask_field2.getVol();
  
  for (int i = 0; i < volume_size; ++i)
  {
    // Mask the data based on the mask field.

    if (mask_data[i] == mask_field_hdr.bad_data_value ||
	mask_data[i] == mask_field_hdr.missing_data_value ||
	mask_data2[i] == mask_field_hdr2.bad_data_value ||
	mask_data2[i] == mask_field_hdr2.missing_data_value)
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    if (quality.isBadAtIndex(i))
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    if (quality[i] <= quality_thresh)
    {
      output_data[i] = output_field_hdr.missing_data_value;
      continue;
    }
    
    if (mask_data2[i] > mask_field2_threshold)
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

