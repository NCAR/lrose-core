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
 * @file Calib.cc
 *
 * @class Calib
 *
 * Class for doing all of the data processing.
 *  
 * @date 1/15/2008
 *
 */

#include "Calib.hh"
#include "Reader.hh"
#include <Refract/RefractConstants.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>
#include <toolsa/LogStream.hh>
using std::string;
using std::vector;

const size_t Calib::MINIMUM_TARGET_LIST_FILES = 2;

//-----------------------------------------------------------------------
Calib::Calib(const Params &params) :
        _params(params),
        _inputHandler(_params),
        _numAzim(0),
        _numRangeBins(0),
        _rMin(_params.r_min),
        _beamWidth(_params.beam_width),
        _sideLobePower(_params.side_lobe_pow),
        _refFilePath(_params.output_file_path),
        _refUrl(_params.output_dir),
        _fluctSnr(0),
        _pixelCount(0),
        _writeDebugMdvFiles(false),
        _debugMdvUrl("")
{

  
  if (_params.write_debug_mdv_files) {
    setDebugMdvUrl(_params.debug_mdv_url);
  }
  
  switch (_params.entry_type){
    case Params::ENTER_P_T_TD :
      _nValue = RefractUtil::deriveNValue(_params.calib_pressure,
                                          _params.calib_temperature,
                                          _params.calib_dewpoint_temperature);
      break;
    case Params::ENTER_N :
    default:
      _nValue = _params.calib_n;
      break;
  }

  if (_params.debug) {
    cerr << "---> Setting N value to " << _nValue << endl;
  }
  
}

//-----------------------------------------------------------------------
Calib::~Calib()
{
  // Free global arrays

  if (_fluctSnr != NULL)
    delete [] _fluctSnr;
  if (_pixelCount != NULL)
    delete [] _pixelCount;
}

//-----------------------------------------------------------------------
/*
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

bool Calib::findReliableTargets(const std::vector< std::string > &file_list,
                                double &gate_spacing)
{
  // Check for the minimum number of files that can be included in the
  // file list.

  if (file_list.size() < MINIMUM_TARGET_LIST_FILES)
  {
    LOG(ERROR) << "Not enough files in the file list for finding targets.";
    LOG(ERROR) << "Have " << file_list.size() << " files in list, Need"
	       << MINIMUM_TARGET_LIST_FILES << " files";
    return false;
  }
  
  // Process all of the files
  
  vector< string >::const_iterator file_iter;
  bool first_file = true;
  int file_num = 1;

  LOG(DEBUG) << "Going through all the files:";

  FieldDataPair *av_iq_field=NULL, *dif_from_ref_iq_field=NULL;
  
  for (file_iter = file_list.begin(); file_iter != file_list.end();
       ++file_iter, ++file_num)
  {
    if (!_findReliableTargetsOneFile(file_num, *file_iter,
				     gate_spacing, first_file,
				     &av_iq_field, &dif_from_ref_iq_field))
    {
      if (av_iq_field != NULL)  delete av_iq_field;
      if (dif_from_ref_iq_field != NULL) delete dif_from_ref_iq_field;
      return false;
    }
      
  } /* endfor - file_iter */

  int num_gates = av_iq_field->numGates();
  int num_azim = av_iq_field->numAzim();

  // debugging
  vector<double> phase_targ = av_iq_field->createDebugPhaseVector();
  if (!phase_targ.empty())
  {
    _writeDebugPhaseCalibFiles(phase_targ, num_azim, num_gates);
  }

  // Do the actual computation of reliability based on how "coherently" the
  // _difPrevScan were summed up.  The calculation is very much like
  // that of normalized lag 1 power (NCP) in pulse-pair computation.
  LOG(DEBUG) << "Now completing the calculation of reliability...";
  _reliability();
  
  // And try to eliminate (or at least dampen the accuracy of) echoes that
  // could be the result of sidelobes.  Echoes that are around or below
  // SideLobePow dB of the integrated power strength for all azimuths at a
  // given range are affected, as well as those that are immediately next to
  // much stronger targets.

  float contamin_pow =
    exp(log(4.0) * RefractUtil::SQR(360.0 / (float)num_azim / _beamWidth)) ;

  // First, look for mainlobe contamination on nearby azimuths
  _calibNcpField.mainlobeElimination(_rMin, contamin_pow, _calibStrengthField);
  _calibPhaseErField.setPhaseError(_calibNcpField, refract::VERY_LARGE);
  
  // Then, look for possible range sidelobes
  // #### Assumption: PulseLength = GateSpacing with a matched filter
  contamin_pow = 4.0;
  _calibNcpField.rangeSidelobeElimination(_rMin, contamin_pow, refract::VERY_LARGE,
					  _calibStrengthField);
  _calibPhaseErField.setPhaseError(_calibNcpField, refract::VERY_LARGE);

  // Finally handle 360 deg sidelobes
  LOG(DEBUG) << "In code Using side lobe power parameter " << _sideLobePower;
  _calibNcpField.sidelobe360Elimination(_rMin, _sideLobePower,
					_calibStrengthField, _oldSnrField);

  _calibPhaseErField.setPhaseError(_calibNcpField, refract::VERY_LARGE);

  // Calculate the quality field which is the second field displayed
  // in n_viewcalib.  This field is not included in the original
  // calibration files, but is included in the new ones so it can be
  // displayed in CIDD
  _calibQualityField.setAbsValue(_calibNcpField, refract::INVALID);
  
  LOG(DEBUG) << "Task completed.";
  
  if (av_iq_field) {
    delete av_iq_field;
  }
  if (dif_from_ref_iq_field) {
    delete dif_from_ref_iq_field;
  }
  
  return true;
}

//-----------------------------------------------------------------------
bool Calib::calibTargets(const std::vector< std::string > &file_list,
			 const double required_gate_spacing)
{
  // Initialize some variables
  LOG(DEBUG) << "Calibrating ground targets for N = " << _nValue << "...";
  
  _sumABField.setAllZero();
  
  _numAzim = _sumABField.numAzim();
  _numRangeBins = _sumABField.numGates();
  int scan_size = _sumABField.scanSize();
  
  // Read phase data and normalize

  LOG(DEBUG) << "Going through all the files:";
  
  vector< string >::const_iterator file_iter;
  int file_num = 1;
  
  time_t last_data_time;
  
  for (file_iter = file_list.begin(); file_iter != file_list.end();
       ++file_iter, ++file_num)
  {
    if (!_calibTargetsOneFile(file_num, *file_iter, last_data_time))
    {
      return false;
    }
  }

  // Normalize the reference information

  LOG(DEBUG) << "Normalizing reference information...";

  // build up a normalization array, negative where ncp is invalid or
  // zero...this is ugly code

  vector<double> norm3 = _calibNcpField.normalizationVector(refract::INVALID);
  for (int index = 0; index < scan_size; ++index)
  {
    if (norm3[index] < 0)
    {
      continue;
    }
    float norm =  _sumABField[index].norm() / norm3[index];
    if (norm == 0.0)
    {
      _calibAvIQField[index].set(0.0, 0.0);
      _calibPhaseErField[index] = refract::VERY_LARGE;
    }
    else
    {
      _calibAvIQField[index] = _sumABField[index] / norm;

    }
  } /* endfor - index */

  // Write the calibration file

  if (!_writeCalibrationFile(last_data_time))
    return false;
  
  LOG(DEBUG) << "Calibration completed.";
  return true;
}


//-----------------------------------------------------------------------
bool
  Calib::_findReliableTargetsOneFile(int file_num, const std::string &filename,
                                     double &gate_spacing, bool &first_file,
                                     FieldDataPair **av_iq_field,
                                     FieldDataPair **dif_from_ref_iq_field)
{
  LOG(DEBUG) << "   " << file_num << " - " << filename;

  // Read in the data from the file
  DsMdvx input_file;
    
  if (!_inputHandler.getNextScan(filename, input_file))
  {
    LOG(ERROR) << "reading input file:" << filename;
    return false;
  }
  
  FieldDataPair iq;
  FieldWithData SNR;
  if (!_setupInputs(input_file, iq, SNR))
  {
    return false;
  }

  // printf("Number of az = %d\n", iq.numAzim());
  // printf("azimuthal spacing = %f\n", iq.azimuthalSpacing());
  // return true;

  // Initialize things and process the first file.

  if (first_file)
  {
    if (!_findReliableTargetsFirstFile(input_file, iq, SNR, gate_spacing,
                                       av_iq_field, dif_from_ref_iq_field))
    {
      return false;
    }
    first_file = false;
  }
  else
  {
    if (!_findReliableTargetsNonFirstFile(input_file, iq, SNR, gate_spacing,
					  **av_iq_field,
					  **dif_from_ref_iq_field))
    {
      return false;
    }
	
  }
    
  // Transfer old I/Q data in _difPrevScan to be ready for the next round
  _difPrevScanIQField = FieldDataPair(iq, "dif_prev_scan_i", "none",
				      "dif_prev_scan_q", "none");
    
  _oldSnrField = FieldWithData(SNR, "oldSnr", "none");
  return true;
}

//-----------------------------------------------------------------------
bool Calib::_calibTargetsOneFile(int file_num, const std::string &fileName,
				 time_t &last_data_time)
{

  LOG(DEBUG) << "   " << file_num << " - " << fileName;
  DsMdvx mdvx;
  if (!_inputHandler.getNextScan(fileName, mdvx))
  {
    LOG(ERROR) << "reading input file:" << fileName;
    return false;
  }
  
  FieldDataPair iq;
  FieldWithData SNR;
  if (!_setupInputs(mdvx, iq, SNR))
  {
    return false;
  }
    
  last_data_time = mdvx.getMasterHeader().time_centroid;
    
  // Normalize the I and Q values
  iq.normalize();

  // Average phase data by averaging I and Q
  _sumABField += iq;
  return true;
}

//-----------------------------------------------------------------------
bool
  Calib::_findReliableTargetsFirstFile(const DsMdvx &input_file,
                                       const FieldDataPair &iq,
                                       const FieldWithData &SNR,
                                       double &gate_spacing, 
                                       FieldDataPair **av_iq_field,
                                       FieldDataPair **dif_from_ref_iq_field)
{  
  // Save the gate spacing value so we can make sure it doesn't change
  // between scans.

  gate_spacing = iq.gateSpacing();
      
  // Allocate space for global fields
  if (!_allocateGlobalFields(iq))
  {
    return false;
  }
      
  // Allocate space for local fields

  *av_iq_field = new FieldDataPair(iq, "av_i", "none", "av_q", "none");
  *dif_from_ref_iq_field = new FieldDataPair(iq, "dif_from_ref_i", "none",
					     "dif_from_ref_q", "none");
      
  // Process the first file.  This really just initializes the av_i
  // and av_q fields to start the process.
  if (!_findReliableTargets(iq, **av_iq_field))
  {
    return false;
  }      
  if (_writeDebugMdvFiles)
  {
    DsMdvx debug_file;
  
    _updateDebugMasterHdr(debug_file, input_file);
    iq.addToOutput(debug_file);
    debug_file.addField(SNR.fieldCopy());
    debug_file.addField(_meanSnrField.fieldCopy());
    (*av_iq_field)->addToOutput(debug_file);
    if (debug_file.writeToDir(_debugMdvUrl) != 0)
    {
      LOG(ERROR)<< "Error writing debug MDV file to URL: "
		<< _debugMdvUrl;
      LOG(ERROR) << debug_file.getErrStr();
    }
  }
  return true;
}

//-----------------------------------------------------------------------
bool
  Calib::_findReliableTargetsNonFirstFile(const DsMdvx &input_file,
                                          const FieldDataPair &iq,
                                          const FieldWithData &SNR,
                                          double gate_spacing,
                                          FieldDataPair &av_iq_field,
                                          FieldDataPair &dif_from_ref_iq_field)
{
  // Make sure the gate spacing hasn't changed between scans
  if (iq.wrongGateSpacing(gate_spacing))
  {
    LOG(ERROR) << "Wrong gate spacing";
    return false;
  }
      
  // Add in the information from this file
  if (!_findReliableTargets(iq, SNR, av_iq_field, dif_from_ref_iq_field))
  {
    LOG(ERROR) << "No reliable targets";
    return false;
  }
  if (_writeDebugMdvFiles)
  {
    DsMdvx debug_file;
    _updateDebugMasterHdr(debug_file, input_file);
    iq.addToOutput(debug_file);
    debug_file.addField(SNR.fieldCopy());
    debug_file.addField(_meanSnrField.fieldCopy());
    av_iq_field.addToOutput(debug_file);
    dif_from_ref_iq_field.addToOutput(debug_file);
    _difPrevScanIQField.addToOutput(debug_file);
    _sumABField.addToOutput(debug_file);
    debug_file.addField(_sumPField.fieldCopy());
    if (debug_file.writeToDir(_debugMdvUrl) != 0)
    {
      LOG(ERROR) << "Error writing debug MDV file to URL: "
		 << _debugMdvUrl;
      LOG(ERROR) << debug_file.getErrStr();
    }
  }
  return true;
}

//-----------------------------------------------------------------------
void Calib::_reliability(void)
{
  int scan_size = _sumABField.scanSize();
  
  // make an array that at each point is the product of the two things
  vector<double> denom = _sumPField.productVector(_pixelCount);

  vector<int> imask;
  imask.reserve(scan_size);
  for (int i=0; i<scan_size; ++i)
  {
    imask.push_back((int)denom[i]);
  }
      
  // fill in when not invalid with meanSNR/pixelCount
  _calibStrengthField.setStrength(_meanSnrField, _pixelCount, imask,
				  refract::INVALID);
  
  // at each point where pixel count not zero, divide fluctSnr by that count
  for (int i=0; i<_sumABField.scanSize(); ++i)
  {
    if (denom[i] != 0.0 && _pixelCount != 0)
    {
      _fluctSnr[i] /= (float)_pixelCount[i];
    }
  }
  
  // set the ncp field
  _calibNcpField.setNcp(_sumABField, denom, _rMin, _pixelCount,
			_calibStrengthField, _fluctSnr);


  // use ncp to set phase error
  _calibPhaseErField.setPhaseErr(_calibNcpField, imask, _rMin,
				 refract::VERY_LARGE);
  // delete [] denom;
  // delete [] imask;
  
}

//-----------------------------------------------------------------------
bool Calib::_allocateGlobalFields(const FieldDataPair &IQ)
{
  int scan_size = IQ.scanSize();
  
  // Allocate the global fields

  _fluctSnr = new float[scan_size];
  memset(_fluctSnr, 0, scan_size * sizeof(float));

  _meanSnrField = IQ.createFromI("mean_snr", "dB", 0.0);
  
  _pixelCount = new int[scan_size];
  memset(_pixelCount, 0, scan_size * sizeof(int));

  _sumABField = FieldDataPair(IQ, "sum_a", "none", 0.0,
			      "sum_b", "none", 0.0);

  _sumPField = IQ.createFromI("sum_p", "none", 0.0);

  _difPrevScanIQField = FieldDataPair(IQ, "dif_prev_scan_i", "none",
                                      "dif_prev_scan_q", "none");

  _calibStrengthField = IQ.createFromI("strength", "none", 0.0);
  _calibAvIQField = FieldDataPair(IQ, "av_i", "none", 0.0,
				  "av_q", "none", 0.0);
  _calibPhaseErField = IQ.createFromI("phase_er", "none", refract::VERY_LARGE);
  _calibQualityField = IQ.createFromI("quality", "none", 0.0);
  _calibNcpField = IQ.createFromI("ncp", "none", 0.0);
  return true;
}

//-----------------------------------------------------------------------
bool Calib::_findReliableTargets(const FieldDataPair &iq,
				 FieldDataPair &av_iq_field) const
{
  av_iq_field.copyIQ(iq);
  return true;
}


//-----------------------------------------------------------------------
bool Calib::_findReliableTargets(const FieldDataPair &iq,
				 const FieldWithData &SNR,
				 FieldDataPair &av_iq_field,
				 FieldDataPair &dif_from_ref_iq_field)
{
  int num_gates = iq.numGates();
  int scan_size = iq.scanSize();
  
  // Compute phase difference wrt previous scan (now in _difPrevScan_i/q).
  // Normalize I & Q, with some snr-based weighting.
  // (Somewhat similar to beginning of get_quality())
  av_iq_field += iq;
  _difPrevScanIQField.phaseDiffV(iq);

  // normalize using SNR
  _difPrevScanIQField.normalizeUsingSNR(SNR);

  // Smooth (a lot!) _difPrevScan in dif_from_ref.  As a result,
  // dif_from_ref will map changes of phase over large areas (due to
  // meteorology) which could then be compensated for later.  First do it
  // at close range (average across all azimuths and a few range gates).

  int max_r_smooth = _rMin + (num_gates / 10);
  if (max_r_smooth >= num_gates) max_r_smooth = num_gates - 1;

  dif_from_ref_iq_field.smoothClose(_rMin, max_r_smooth, _difPrevScanIQField);

  // And then to ranges further away (sector average).
  dif_from_ref_iq_field.smoothFar(max_r_smooth, _difPrevScanIQField);

  // Smooth phase field done.  Now compensate the previously computed
  // phase difference field for the average change caused by meteorology
  // (i.e. the smoothed phase field).

  _difPrevScanIQField.phaseDiff2V(dif_from_ref_iq_field);

  // Sum up the _difPrevScan_i/q in sum_a/b and sum_p (to be used later to
  // compute, in conjunction with sum_a/b the actual reliability).
  _sumABField += _difPrevScanIQField;

  for (int index = 0; index < scan_size; ++index)
  {
    _sumPField[index] += _difPrevScanIQField[index].normSquared();
    if ((!SNR.isBadAtIndex(index)) && (!_oldSnrField.isBadAtIndex(index)))
    {
      _fluctSnr[index] += fabs(SNR[index] - _oldSnrField[index]);
      _meanSnrField[index] += SNR[index];
      _pixelCount[index]++;
    }
  } /* endfor - index */

  return true;
}

//-----------------------------------------------------------------------
void Calib::_updateDebugMasterHdr(DsMdvx &debug_file,
				  const DsMdvx &input_file) const
{
  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();

  debug_file.setMasterHeader(master_hdr);
}

//-----------------------------------------------------------------------
bool Calib::_writeCalibrationFile(const DateTime &data_time) const
{
  // Write the calibration information to an MDV file

  Mdvx::field_header_t field_hdr = _calibStrengthField.getFieldHeader();

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
  
  calib_file.addField(_calibStrengthField.fieldCopy());
  _calibAvIQField.addToOutput(calib_file);
  calib_file.addField(_calibPhaseErField.fieldCopy());
  calib_file.addField(_calibQualityField.fieldCopy());
  calib_file.addField(_calibNcpField.fieldCopy());

  // write refract file to specified path
  
  if (calib_file.writeToPath(_refFilePath.c_str()) != 0) {
    LOG(ERROR) << "Error writing calibration MDV file to path: "
	       << _refFilePath;
    LOG(ERROR) << calib_file.getErrStr();
    return false;
  }
  LOG(DEBUG) << "Wrote reference calibration file: " << _refFilePath;
  
  // write refract time-stamped file to specified dir
  
  if (calib_file.writeToDir(_refUrl.c_str()) != 0) {
    LOG(ERROR) << "Error writing calibration MDV file to URL: "
	       << _refUrl;
    LOG(ERROR) << calib_file.getErrStr();
    return false;
  }
  LOG(DEBUG) << "Wrote time-stamped calibration file: " << calib_file.getPathInUse();

  return true;
}

//-----------------------------------------------------------------------
void Calib::_writeDebugPhaseCalibFiles(const std::vector<double> &phase_targ,
				       const int num_azim,
				       const int num_gates) const
{
  
  int scan_size = num_azim * num_gates;
  
  // // Write the data file

  // FILE *debug_phasecalib_dat_fp;

  // if ((debug_phasecalib_dat_fp = fopen("debug_phasecalib.dat", "wb")) == 0)
  // {
  //   LOG(ERROR) << "Error opening debug_phasecalib.dat file for writing";
  // }
  // else
  // {
  //   fwrite(phase_targ, sizeof(double), scan_size, debug_phasecalib_dat_fp);
  //   fclose(debug_phasecalib_dat_fp);
  // }
  
  // Write the image file

  FILE *debug_phasecalib_ppm_fp;

  if ((debug_phasecalib_ppm_fp = fopen("debug_phasecalib.ppm", "wb")) == 0)
  {
    LOG(ERROR) << "Error opening debug_phasecalib.ppm file for writing";
  }
  else
  {
    fprintf(debug_phasecalib_ppm_fp, "P5 %d %d 255 ",
	    (int)num_gates, (int)num_azim);
    for (int index = 0; index < scan_size; ++index)
    {
      char c;
    
      if (phase_targ[index] == refract::INVALID)
	c = 0;
      else
	c = (char)(phase_targ[index] * 127.0 / 180.0);
      fwrite(&c, sizeof(char), 1, debug_phasecalib_ppm_fp);
    }
    fclose(debug_phasecalib_ppm_fp);
  }
}

//-----------------------------------------------------------------------
bool Calib::_setupInputs(DsMdvx &input_file, FieldDataPair &iq,
			 FieldWithData &SNR) const
{
  FieldWithData I = _inputHandler.getI(input_file);
  FieldWithData Q = _inputHandler.getQ(input_file);

  // Make sure the gate spacing is the same for the fields.
  if (I.gateSpacing() != Q.gateSpacing())
  {
    LOG(ERROR) << "Gate spacing does not match between fields:";
    LOG(ERROR) << "    I field gate spacing = " << I.gateSpacing();
    LOG(ERROR) << "    Q field gate spacing = " << Q.gateSpacing();
    return false;
  }
  iq = FieldDataPair(I, Q);
  SNR = _inputHandler.getSNR(input_file);
  return true;
}
