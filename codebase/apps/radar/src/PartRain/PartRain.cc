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
// PartRain.cc
//
// PartRain object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////////////
//
// PartRain reads dual pol moments in a DsRadar FMQ,
// compute rain rate and particle ID, and writes these out
// to a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/uusleep.h>
#include <toolsa/ucopyright.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/FilterUtils.hh>
#include <radar/TempProfile.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/Sndg.hh>
#include <iomanip>
#include "PartRain.hh"

const double PartRain::_missingDouble = -9999.0;
const double PartRain::_missingTest = -9998.0;

using namespace std;

// Constructor

PartRain::PartRain(int argc, char **argv)

{
  
  isOK = true;
  _inputQueue = NULL;
  _inputContents = 0;
  _inputNFail = 0;
  _outputQueue = NULL;
  _nFieldsIn = 0;
  _nFieldsOut = 0;
  _nGates = 0;
  _volNum = -9999;
  _nGatesAlloc = 0;
  _debugPrintNeedsNewline = false;
  _nErrorPrintPid = 0;

  _snrIndex = -1;
  _dbzIndex = -1;
  _velIndex = -1;
  _widthIndex = -1;
  _zdrIndex = -1;
  _ldrIndex = -1;
  _kdpIndex = -1;
  _phidpIndex = -1;
  _rhohvIndex = -1;

  _snrAvail = false;
  _dbzAvail = false;
  _zdrAvail = false;
  _ldrAvail = false;
  _kdpAvail = false;
  _phidpAvail = false;
  _rhohvAvail = false;

  _snr = NULL;
  _dbz = NULL;
  _vel = NULL;
  _width = NULL;
  _zdr = NULL;
  _ldr = NULL;
  _kdp = NULL;
  _phidp = NULL;
  _rhohv = NULL;
  _tempForPid = NULL;

  _latestSoundingRequestTime = 0;

  // set programe name
  
  _progName = "PartRain";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // initialize wavelength
  
  _wavelengthCm = _params.wavelength_cm;
  
  // initialize KDP object

  if (_params.apply_median_filter_to_PHIDP) {
    _kdpBringi.setApplyMedianFilterToPhidp(_params.PHIDP_median_filter_len);
  }
  if (_params.KDP_fir_filter_len == Params::FIR_LEN_125) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_125);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_30) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_30);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_20) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_20);
  } else {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_10);
  }
  _kdpBringi.setPhidpDiffThreshold(_params.KDP_phidp_difference_threshold);
  _kdpBringi.setPhidpSdevThreshold(_params.KDP_phidp_sdev_threshold);
  _kdpBringi.setZdrSdevThreshold(_params.KDP_zdr_sdev_threshold);
  _kdpBringi.setRhohvWxThreshold(_params.KDP_rhohv_threshold);
  _kdpBringi.setWavelengthCm(_wavelengthCm);

  // initialize precip rate object

  _rate.setZhAa(_params.zh_aa);
  _rate.setZhBb(_params.zh_bb);

  _rate.setZhAaSnow(_params.zh_aa_snow);
  _rate.setZhBbSnow(_params.zh_bb_snow);

  _rate.setZzdrAa(_params.zzdr_aa);
  _rate.setZzdrBb(_params.zzdr_bb);
  _rate.setZzdrCc(_params.zzdr_cc);
  
  _rate.setKdpAa(_params.kdp_aa);
  _rate.setKdpBb(_params.kdp_bb);

  _rate.setKdpZdrAa(_params.kdpzdr_aa);
  _rate.setKdpZdrBb(_params.kdpzdr_bb);
  _rate.setKdpZdrCc(_params.kdpzdr_cc);
  
  _rate.setHybridDbzThreshold(_params.hybrid_dbz_threshold);
  _rate.setHybridKdpThreshold(_params.hybrid_kdp_threshold);
  _rate.setHybridZdrThreshold(_params.hybrid_zdr_threshold);

  _rate.setHidroDbzThreshold(_params.hidro_dbz_threshold);
  _rate.setHidroKdpThreshold(_params.hidro_kdp_threshold);
  _rate.setHidroZdrThreshold(_params.hidro_zdr_threshold);

  _rate.setBringiDbzThreshold(_params.bringi_dbz_threshold);
  _rate.setBringiKdpThreshold(_params.bringi_kdp_threshold);
  _rate.setBringiZdrThreshold(_params.bringi_zdr_threshold);

  _rate.setMinValidRate(_params.min_valid_rate);

  if (_params.apply_median_filter_to_DBZ) {
    _rate.setApplyMedianFilterToDbz(_params.DBZ_median_filter_len);
  }
  if (_params.apply_median_filter_to_ZDR) {
    _rate.setApplyMedianFilterToZdr(_params.ZDR_median_filter_len);
  }

  _rate.setWavelengthCm(_wavelengthCm);
  _rate.setSnrThresholdDb(_params.PRECIP_snr_threshold);
  
 // initialize particle ID object

  if (_params.apply_median_filter_to_DBZ) {
    _pid.setApplyMedianFilterToDbz(_params.DBZ_median_filter_len);
  }
  if (_params.apply_median_filter_to_ZDR) {
    _pid.setApplyMedianFilterToZdr(_params.ZDR_median_filter_len);
  }

  if (_params.apply_median_filter_to_LDR) {
    _pid.setApplyMedianFilterToLdr(_params.LDR_median_filter_len);
  }
  if (_params.replace_missing_LDR) {
    _pid.setReplaceMissingLdr(_params.LDR_replacement_value);
  }

  if (_params.apply_median_filter_to_RHOHV) {
    _pid.setApplyMedianFilterToRhohv(_params.RHOHV_median_filter_len);
  }
  if (_params.apply_median_filter_to_PID) {
    _pid.setApplyMedianFilterToPid(_params.PID_median_filter_len);
  }

  _pid.setNgatesSdev(_params.ngates_for_sdev);
  _pid.setMinValidInterest(_params.PID_min_valid_interest);

  _pid.setWavelengthCm(_wavelengthCm);

  _pid.setSnrThresholdDb(_params.PID_snr_threshold);
  _pid.setSnrUpperThresholdDb(_params.PID_snr_upper_threshold);
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  
}

/////////////////////////////////////////////////////////
// destructor

PartRain::~PartRain()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PartRain::Run ()
{
  
  // register with procmap
  
  PMU_auto_register("Run");
  
  while (true) {
    if (_params.debug) {
      cerr << "PartRain::Run:" << endl;
      cerr << "  Reading input queue at url: "
	   << _params.input_fmq_url << endl;
    }
    _run();
    umsleep(1000);
  }

  return 0;

}

//////////////////////////////////////////////////
// _run

int PartRain::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // iniitialize particle id object as needed

  _pid.setMissingDouble(_missingDouble);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _pid.setDebug(true);
  }
  if (_pid.readThresholdsFromFile(_params.pid_thresholds_file_path)) {
    cerr << "ERROR - PartRain::_run" << endl;
    cerr << "  Cannot read in pid thresholds from file: "
         << _params.pid_thresholds_file_path << endl;
    cerr << "  Cannot compute PID" << endl;
    return -1;
  }

  // open the input queue

  if (_openInputQueue()) {
    return -1;
  }

  // open the output queue

  if (_openOutputQueue()) {
    return -1;
  }

  while (true) { 
    
    PMU_auto_register("Reading radar queue");
    
    // get a message from the radar queue
    
    int iret = _inputQueue->getDsMsg(_inputMsg, &_inputContents);

    if (iret) {

      cerr << "ERROR - PartRain" << endl;
      cerr << "  inputQueue:getDsBeam() failed, retval: " << iret << endl;
      
      // Keep count of consecuive failures.
      _inputNFail++;

      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      if (_inputNFail > 100) {
	cerr << "The program is failing consistently, reopen the queues ..." << endl;
	return -1;
      }

      umsleep(1000);

    } else { 

      // getDsBeam succeded, reset the count of consecutive failures.
      _inputNFail = 0;

      // process the message

      _processInputMessage();

    } // if (inputQueue ...

  } // while

  return 0;

}

////////////////////////////////////////////////////////////////
// open the input queue
//
// Returns 0 on success, -1 on failure

int PartRain::_openInputQueue()

{
  
  // if already open, delete queue to close it

  if (_inputQueue != NULL) {
    delete _inputQueue;
  }

  // create queue

  _inputQueue = new DsRadarQueue();

  // initialize the input DsRadar queue and message
  
  DsFmq::openPosition open_pos;
  if (_params.seek_to_start_of_input) {
    open_pos = DsFmq::START;
  } else {
    open_pos = DsFmq::END;
  }
  if (_inputQueue->init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::READ_ONLY, open_pos)) {
    cerr << "ERROR - PartRain" << endl;
    cerr << "  Could not initialize input radar queue: "
	 << _params.input_fmq_url << endl;
    delete _inputQueue;
    _inputQueue = NULL;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// open the output queue
//
// Returns 0 on success, -1 on failure

int PartRain::_openOutputQueue()

{
  
  // if already open, delete queue to close it

  if (_outputQueue != NULL) {
    delete _outputQueue;
  }

  // create queue

  _outputQueue = new DsRadarQueue();

  // initialize the output DsRadar queue
  
  if (_outputQueue->init(_params.output_fmq_url,
			 _progName.c_str(),
			 _params.debug >= Params::DEBUG_VERBOSE,
			 DsFmq::READ_WRITE, DsFmq::END,
			 _params.output_fmq_compress,
			 _params.output_fmq_nslots,
			 _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    delete _outputQueue;
    _outputQueue = NULL;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _outputQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.write_blocking) {
    _outputQueue->setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _outputQueue->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  // set up the output fields

  _nFieldsOut = 0;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    // field params, byte width 2, missing value 0
    DsFieldParams fparams(field.name, field.units, field.scale, field.bias, 2, 0);
    _outputMsg.addFieldParams(fparams);
    _nFieldsOut++;
  }

  if (_params.output_particle_interest_fields) {
    const vector<NcarParticleId::Particle*> plist = _pid.getParticleList();
    for (int ii = 0; ii < (int) plist.size(); ii++) {
      string fieldName = plist[ii]->label;
      fieldName += "_interest";
      // field params, byte width 2, missing value 0
      DsFieldParams fparams(fieldName.c_str(), "", 0.0001, -1, 2, 0);
      _outputMsg.addFieldParams(fparams);
      _nFieldsOut++;
    }
  }
    
  return 0;

}

////////////////////////////////////////////////////////////////
// process the input message
// Returns 0 on success, -1 on failure

int PartRain::_processInputMessage()

{

  // copy flags from input queue to output queue
  
  if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
    _copyFlags();
  }

  // modify radar params for number of fields, write to output queue

  if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
    _inputRadarParams =  _inputMsg.getRadarParams();
    _nFieldsIn = _inputRadarParams.numFields;
    _nGates = _inputRadarParams.numGates;
    DsRadarParams outputParams = _inputRadarParams;
    outputParams.numFields = _nFieldsOut;
    _outputMsg.setRadarParams(outputParams);
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_PARAMS)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put radar params to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote radar params");
    }
  }

  // write fields params to output queue

  if (_inputContents & DsRadarMsg::FIELD_PARAMS) {
    _inputFieldParams =  _inputMsg.getFieldParams();
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::FIELD_PARAMS)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put field params to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote field params");
    }
  }

  // write radar calib to output queue

  if (_inputContents & DsRadarMsg::RADAR_CALIB) {
    _inputRadarCalib =  _inputMsg.getRadarCalib();
    _outputMsg.setRadarCalib(_inputRadarCalib);
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_CALIB)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put calib to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote radar calib");
    }
  }

  // process a beam
  
  if (_inputContents & DsRadarMsg::RADAR_BEAM) {
    if (_processBeam()) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot process beam" << endl;
      return -1;
    }
  }

  return 0;

}
      
////////////////////////////////////////////////////////////////
// copy flags in the input message to output message

void PartRain::_copyFlags()

{

  // copy the flags to the output queue

  const DsRadarFlags &flags = _inputMsg.getRadarFlags();
  
  if (flags.startOfTilt) {
    _outputQueue->putStartOfTilt(flags.tiltNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("Start of tilt");
    }
  }
  
  if (flags.endOfTilt) {
    _outputQueue->putEndOfTilt(flags.tiltNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("End of tilt");
    }
  }
  
  if (flags.startOfVolume) {
    _outputQueue->putStartOfVolume(flags.volumeNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("Start of volume");
    }
  }
  
  if (flags.endOfVolume) {
    _outputQueue->putEndOfVolume(flags.volumeNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("End of volume");
    }
  }
  
  if (flags.newScanType) {
    _outputQueue->putNewScanType(flags.scanType, flags.time);
    if (_params.debug) {
      _printDebugMsg("New scan type");
    }
  }
  
}
      
////////////////////////////////////////////////////////////////
// process a beam
// Returns 0 on success, -1 on failure

int PartRain::_processBeam()

{

  const DsRadarParams &rparams = _inputMsg.getRadarParams();
  _wavelengthCm = rparams.wavelength;
  _radarHtKm = rparams.altitude;
  _kdpBringi.setWavelengthCm(_wavelengthCm);
  _rate.setWavelengthCm(_wavelengthCm);
  _pid.setWavelengthCm(_wavelengthCm);

  const DsRadarBeam &beam = _inputMsg.getRadarBeam();
  _elev = beam.elevation;
  _az = beam.azimuth;

  // check for new vol

  if (_volNum != beam.volumeNum) {

    // re-read in PID parameters
    // only if the previous read was successful
    
    _pid.setMissingDouble(_missingDouble);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _pid.setDebug(true);
    }
    if (_pid.readThresholdsFromFile(_params.pid_thresholds_file_path)) {
      cerr << "ERROR - PartRain::_processBeam" << endl;
      cerr << "  Cannot read in pid thresholds from file: "
           << _params.pid_thresholds_file_path << endl;
      cerr << "  PID will not be computed" << endl;
      return -1;
    }

    _volNum = beam.volumeNum;

  }

  // allocate the data arrays

  _allocateArrays(_nGates);

  // compute the indices of each input field in the Dsr beam
  // if field is missing, these are set to -1
  
  if (_params.SNR_available) {
    _snrIndex = _getInputFieldIndex(_params.dsr_name_SNR);
  }
  _dbzIndex = _getInputFieldIndex(_params.dsr_name_DBZ);
  _velIndex = _getInputFieldIndex(_params.dsr_name_VEL);
  _widthIndex = _getInputFieldIndex(_params.dsr_name_WIDTH);
  _zdrIndex = _getInputFieldIndex(_params.dsr_name_ZDR);
  _ldrIndex = _getInputFieldIndex(_params.dsr_name_LDR);
  _kdpIndex = _getInputFieldIndex(_params.dsr_name_KDP);
  _phidpIndex = _getInputFieldIndex(_params.dsr_name_PHIDP);
  _rhohvIndex = _getInputFieldIndex(_params.dsr_name_RHOHV);

  // load up input field arrays, censoring as appropriate

  if (_params.SNR_available) {
    _loadInputField(beam, _snrIndex, _snr); 
  }
  _loadInputField(beam, _dbzIndex, _dbz); 
  _loadInputField(beam, _velIndex, _vel); 
  _loadInputField(beam, _widthIndex, _width); 
  _loadInputField(beam, _zdrIndex, _zdr); 
  _loadInputField(beam, _ldrIndex, _ldr); 
  _loadInputField(beam, _kdpIndex, _kdp); 
  _loadInputField(beam, _phidpIndex, _phidp); 
  _loadInputField(beam, _rhohvIndex, _rhohv); 

  // set field availability;

  _snrAvail = false;
  _dbzAvail = false;
  _zdrAvail = false;
  _ldrAvail = false;
  _kdpAvail = false;
  _phidpAvail = false;
  _rhohvAvail = false;
  
  if (_dbzIndex >= 0) {
    _dbzAvail = true;
  }
  if (_snrIndex >= 0) {
    _snrAvail = true;
  } else {
    if (_dbzAvail) {
      _computeSnrFromDbz(_dbz, _snr);
      _snrAvail = true;
    }
  }
  if (_velIndex >= 0) {
    _velAvail = true;
  }
  if (_widthIndex >= 0) {
    _widthAvail = true;
  }
  if (_zdrIndex >= 0) {
    _zdrAvail = true;
  }
  if (_ldrIndex >= 0) {
    _ldrAvail = true;
  }
  if (_kdpIndex >= 0) {
    _kdpAvail = true;
  }
  if (_phidpIndex >= 0) {
    _phidpAvail = true;
  }
  if (_rhohvIndex >= 0) {
    _rhohvAvail = true;
  }

  // compute KDP if needed

  if (!_params.KDP_available || !_kdpAvail) {
    if (_computeKdp() == 0) {
      _kdpAvail = true;
    }
  }

  // particle identification
  
  if (_params.use_soundings_from_spdb) {
    _overrideTempProfile(beam.dataTime);
  }
  
  if (_dbzAvail && _zdrAvail && _kdpAvail &&
      _rhohvAvail && _phidpAvail) {
    
    // create temperature array
    
    _pid.fillTempArray(_radarHtKm,
                       _params.override_standard_pseudo_earth_radius,
                       _params.pseudo_earth_radius_ratio,
                       beam.elevation, _nGates,
                       _inputRadarParams.startRange,
                       _inputRadarParams.gateSpacing,
                       _tempForPid);
    
    // compute particle ID
    
    _pid.computePidBeam(_nGates, _snr, _dbz, _zdr,
                        _kdp, _ldr, _rhohv, _phidp, _tempForPid);
    
  } else {
    
    if ((_nErrorPrintPid % 100) == 0) {
      cerr << "ERROR - PartRain, computing PID" << endl;
      if (!_dbzAvail) { cerr << "  DBZ missing" << endl; }
      if (!_zdrAvail) { cerr << "  ZDR missing" << endl; }
      if (!_kdpAvail) { cerr << "  KDP missing" << endl; }
      if (!_rhohvAvail) { cerr << "  RHOHV missing" << endl; }
      if (!_phidpAvail) { cerr << "  PHIDP missing" << endl; }
    }
    _nErrorPrintPid++;
  }
  
  // compute precip rate
  
  _rate.computePrecipRates(_nGates, _snr,
                           _dbz, _zdr, _kdp, 
                           _missingDouble,
                           &_pid);
  
  // write beam to output FMQ
  
  if (_writeBeam()) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// get index for a specified field name
// Returns -1 on failure

int PartRain::_getInputFieldIndex(const string &dsrName)

{

  // compute the indices of each input field in the Dsr beam
  
  const vector<DsFieldParams*> &fieldParams = _inputMsg.getFieldParams();
  for (int ii = 0; ii < (int) fieldParams.size(); ii++) {
    if (dsrName == fieldParams[ii]->name) {
      return ii;
    }
  }

  return -1;

}

////////////////////////////////////////////////////////////////
// load field data

void PartRain::_loadInputField(const DsRadarBeam &beam,
                               int index, double *fldData)

{

  // start by filling with missing values

  for (int ii = 0; ii < _nGates; ii++) {
    fldData[ii] = _missingDouble;
  }

  if (index < 0) {
    // field not in input data
    return;
  }

  if (beam.byteWidth == 4) {

    fl32 *fval = (fl32 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, fval += _nFieldsIn) {
      fldData[ii] = *fval;
    }

  } else if (beam.byteWidth == 2) {

    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui16 *sval = (ui16 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, sval += _nFieldsIn) {
      fldData[ii] = *sval * scale + bias;
    }

  } else {

    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui08 *bval = (ui08 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, bval += _nFieldsIn) {
      fldData[ii] = *bval * scale + bias;
    }

  }

}

////////////////////////////////////////////////////////////////
// compute SNR from DBZ

void PartRain::_computeSnrFromDbz(const double *dbz, double *snr)

{
  
  double startRangeKm = _inputRadarParams.startRange;
  double gateSpacingKm = _inputRadarParams.gateSpacing;
  double range = startRangeKm;

  for (int ii = 0; ii < _nGates; ii++, range += gateSpacingKm) {
    double rangeCorrection = 20.0 * (log10(range) - log10(100.0));
    double noiseDbz = _params.noise_dbz_at_100km + rangeCorrection;
    _snr[ii] = _dbz[ii] - noiseDbz;
  } // ii

}

///////////////////////////////////////////
// compute KDP
// returns 0 on success, -1 on failure

int PartRain::_computeKdp()
  
{

  if (!_snrAvail || !_dbzAvail ||  !_zdrAvail ||
      !_phidpAvail || !_rhohvAvail) {
    cerr << "ERROR - PartRain::_computeKdp" << endl;
    cerr << "  Cannot compute KDP" << endl;
    if (!_snrAvail) {
      cerr << "  SNR data not available" << endl;
    }
    if (!_dbzAvail) {
      cerr << "  DBZ data not available" << endl;
    }
    if (!_zdrAvail) {
      cerr << "  ZDR data not available" << endl;
    }
    if (!_phidpAvail) {
      cerr << "  PHIDP data not available" << endl;
    }
    if (!_rhohvAvail) {
      cerr << "  RHOHV data not available" << endl;
    }
    return -1;
  }

  // compute range array

  TaArray<double> range_;
  double *range = range_.alloc(_nGates);
  for (int ii = 0; ii < _nGates; ii++) {
    range[ii] = _inputRadarParams.startRange +
      ii * _inputRadarParams.gateSpacing;
  }

  // compute KDP
  
  _kdpBringi.compute(_elev, _az, _nGates, range, _dbz,
                     _zdr, _phidp, _rhohv, _snr, _missingDouble);

  // store KDP
  
  const double *kdp = _kdpBringi.getKdp();
  for (int ii = 0; ii < _nGates; ii++) {
    double kdpii = kdp[ii];
    if (std::isnan(kdpii) || std::isinf(kdpii)) {
      cerr << "ERROR - bad kdp: " << kdpii << endl;
      _kdp[ii] = _missingDouble;
    } else {
      _kdp[ii] = kdp[ii];
    }
  } // ii

  return 0;

}

///////////////////////
// write out beam

int PartRain::_writeBeam()

{

  // input beam

  const DsRadarBeam &inBeam = _inputMsg.getRadarBeam();

  // output beam

  DsRadarBeam &outBeam = _outputMsg.getRadarBeam();
  outBeam.copyHeader(inBeam);

  // set output byte width - we are using 16-bit integers
  
  outBeam.byteWidth = 2;
  
  // create output data array
  
  int nData = _nFieldsOut * _nGates;
  TaArray<ui16> outData_;
  ui16 *outData = (ui16 *) outData_.alloc(nData);
  memset(outData, 0, nData * sizeof(ui16));
  
  // load up field data

  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    
    const Params::output_field_t &fld = _params._output_fields[ii];
    int fIndx = ii;
    
    switch (fld.id) {
      
    case Params::SNR:
      if (_snrAvail) {
	_loadOutputField(_snr, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::DBZ:
      if (_dbzAvail) {
	_loadOutputField(_dbz, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::VEL:
      if (_velAvail) {
	_loadOutputField(_vel, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::WIDTH:
      if (_widthAvail) {
	_loadOutputField(_width, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::ZDR:
      if (_zdrAvail) {
	_loadOutputField(_zdr, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::LDR:
      if (_ldrAvail) {
	_loadOutputField(_ldr, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::PHIDP:
      if (_phidpAvail) {
	_loadOutputField(_phidp, fIndx, fld.scale, fld.bias, outData);
      }
      break;
      
    case Params::RHOHV:
      if (_rhohvAvail) {
        _loadOutputField(_rhohv, fIndx, fld.scale, fld.bias, outData);
      }
      break;
        
    case Params::KDP:
      if (_kdpAvail) {
        if (_kdpBringi.getKdp() != NULL) {
          _loadOutputField(_kdpBringi.getKdp(), fIndx,
                           fld.scale, fld.bias, outData);
        } else if (_kdpIndex >= 0) {
          _loadOutputField(_kdp, fIndx, fld.scale, fld.bias, outData);
        }
      }
      break;
      
    case Params::DBZ_FOR_KDP:
      _loadOutputField(_kdpBringi.getDbz(), fIndx, fld.scale, fld.bias, outData);
      break;
        
    case Params::ZDR_FOR_KDP:
      _loadOutputField(_kdpBringi.getZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::RHOHV_FOR_KDP:
      _loadOutputField(_kdpBringi.getRhohv(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::SNR_FOR_KDP:
      _loadOutputField(_kdpBringi.getSnr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PHIDP_FOR_KDP:
      _loadOutputField(_kdpBringi.getPhidp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PHIDP_FILT_FOR_KDP:
      _loadOutputField(_kdpBringi.getPhidpFilt(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::SDPHIDP_FOR_KDP:
      _loadOutputField(_kdpBringi.getSdPhidp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::SDZDR_FOR_KDP:
      _loadOutputField(_kdpBringi.getSdZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_ZH:
      _loadOutputField(_rate.getRateZ(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_ZH_SNOW:
      _loadOutputField(_rate.getRateZSnow(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_KDP:
      _loadOutputField(_rate.getRateKdp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_KDP_ZDR:
      _loadOutputField(_rate.getRateKdpZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_Z_ZDR:
      _loadOutputField(_rate.getRateZZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_HYBRID:
      _loadOutputField(_rate.getRateHybrid(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_PID:
      _loadOutputField(_rate.getRatePid(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_HIDRO:
      _loadOutputField(_rate.getRateHidro(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PRECIP_RATE_BRINGI:
      _loadOutputField(_rate.getRateBringi(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::DBZ_FOR_RATE:
      _loadOutputField(_rate.getDbz(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::ZDR_FOR_RATE:
      _loadOutputField(_rate.getZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::KDP_FOR_RATE:
      _loadOutputField(_rate.getKdp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PARTICLE_ID:
      _loadOutputField(_pid.getPid(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PID_INTEREST:
      _loadOutputField(_pid.getInterest(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PARTICLE_ID2:
      _loadOutputField(_pid.getPid2(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PID_INTEREST2:
      _loadOutputField(_pid.getInterest2(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PID_CONFIDENCE:
      _loadOutputField(_pid.getConfidence(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::DBZ_FOR_PID:
      _loadOutputField(_pid.getDbz(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::ZDR_FOR_PID:
      _loadOutputField(_pid.getZdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::LDR_FOR_PID:
      _loadOutputField(_pid.getLdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::PHIDP_FOR_PID:
      _loadOutputField(_pid.getPhidp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::RHOHV_FOR_PID:
      _loadOutputField(_pid.getRhohv(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::KDP_FOR_PID:
      _loadOutputField(_pid.getKdp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::SDZDR_FOR_PID:
      _loadOutputField(_pid.getSdzdr(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::SDPHIDP_FOR_PID:
      _loadOutputField(_pid.getSdphidp(), fIndx, fld.scale, fld.bias, outData);
      break;
      
    case Params::TEMP_FOR_PID:
      _loadOutputField(_tempForPid, fIndx, fld.scale, fld.bias, outData);
      break;
      
    }

  } // ii

  if (_params.output_particle_interest_fields) {
    const vector<NcarParticleId::Particle*> plist = _pid.getParticleList();
    for (int ii = 0; ii < (int) plist.size(); ii++) {
      int fIndx = ii + _params.output_fields_n;
      _loadOutputField(plist[ii]->gateInterest, fIndx, 0.0001, -1, outData);
    }
  }

  // load output data into beam

  outBeam.loadData(outData, nData * sizeof(ui16), sizeof(ui16));

  // write the beam

  if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_BEAM)) {
    cerr << "ERROR - OutputFmq::writeBeam" << endl;
    cerr << "  Cannot write output beam to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDebugMsg("Wrote beam, beam_time: ", false);
    _printDebugMsg(DateTime::strm(outBeam.dataTime));
  } else if (_params.debug) {
    cerr << ".";
    _debugPrintNeedsNewline = true;
  }

  return 0;

}

/////////////////////////////////////////////
// load output field data into output array

void PartRain::_loadOutputField(const double *fld, int index,
				double scale, double bias,
				ui16 *outData)

{

  if (fld == NULL) {
    return;
  }

  for (int ii = 0, loc = index; ii < _nGates; ii++, loc += _nFieldsOut) {

    double fldVal = fld[ii];
    int scaledVal = 0;
    if (fldVal > _missingTest) {
      scaledVal = (int) floor(((fldVal - bias) / scale) + 0.5);
    }
    if (scaledVal < 0) {
      scaledVal = 0;
    } else if (scaledVal > 65535) {
     scaledVal = 65535;
    }

    outData[loc] = (ui16) scaledVal;

  }

}

/////////////////////////////////////////////
// load output field data into output array

void PartRain::_loadOutputField(const int *fld, int index,
				double scale, double bias,
				ui16 *outData)

{

  if (fld == NULL) {
    return;
  }

  for (int ii = 0, loc = index; ii < _nGates; ii++, loc += _nFieldsOut) {
    
    double fldVal = (double) fld[ii];
    int scaledVal = 0;
    if (fld[ii] != 0) {
      scaledVal = (int) floor(((fldVal - bias) / scale) + 0.5);
    }

    if (scaledVal < 0) {
      scaledVal = 0;
    } else if (scaledVal > 65535) {
     scaledVal = 65535;
    }

    outData[loc] = (ui16) scaledVal;

  }

}

////////////////////////////////////////////////////////////////////////
// print a debug message, taking account of whether a newline is needed

void PartRain::_printDebugMsg(const string &msg, bool addNewline /* = true */)

{

  if (_debugPrintNeedsNewline) {
    cerr << endl;
    _debugPrintNeedsNewline = false;
  }
  cerr << msg;
  if (addNewline) {
    cerr << endl;
  }

}

/////////////////////////////////////////////////////
// define function to be used for sorting

int PartRain::_doubleCompare(const void *i, const void *j)
{
  double *f1 = (double *) i;
  double *f2 = (double *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////
// allocate data arrays

void PartRain::_allocateArrays(int nGates)

{

  if (nGates == _nGatesAlloc) {
    return;
  }

  _snr = (double *) _snr_.alloc(nGates);
  _dbz = (double *) _dbz_.alloc(nGates);
  _vel = (double *) _vel_.alloc(nGates);
  _width = (double *) _width_.alloc(nGates);
  _zdr = (double *) _zdr_.alloc(nGates);
  _ldr = (double *) _ldr_.alloc(nGates);
  _kdp = (double *) _kdp_.alloc(nGates);
  _phidp = (double *) _phidp_.alloc(nGates);
  _rhohv = (double *) _rhohv_.alloc(nGates);
  _tempForPid = (double *) _tempForPid_.alloc(nGates);

  _nGatesAlloc = nGates;

}

////////////////////////////////////////////////////////////////////////
// override temperature profile using sounding from SPDB

int PartRain::_overrideTempProfile(time_t beamTime)

{

  // only retrieve once every minute

  int secsSinceLastRetrieval = beamTime - _latestSoundingRequestTime;
  if (abs(secsSinceLastRetrieval) < 60) {
    return 0;
  }
  _latestSoundingRequestTime = beamTime;

  // get temperature profile

  TempProfile tempProfile;

  tempProfile.setSoundingLocationName
    (_params.sounding_location_name);
  tempProfile.setSoundingSearchTimeMarginSecs
    (_params.sounding_search_time_margin_secs);
  
  tempProfile.setCheckPressureRange
    (_params.sounding_check_pressure_range);
  tempProfile.setSoundingRequiredMinPressureHpa
    (_params.sounding_required_pressure_range_hpa.min_val);
  tempProfile.setSoundingRequiredMaxPressureHpa
    (_params.sounding_required_pressure_range_hpa.max_val);
  
  tempProfile.setCheckHeightRange
    (_params.sounding_check_height_range);
  tempProfile.setSoundingRequiredMinHeightM
    (_params.sounding_required_height_range_m.min_val);
  tempProfile.setSoundingRequiredMaxHeightM
    (_params.sounding_required_height_range_m.max_val);
  
  tempProfile.setCheckPressureMonotonicallyDecreasing
    (_params.sounding_check_pressure_monotonically_decreasing);

  tempProfile.setHeightCorrectionKm
    (_params.sounding_height_correction_km);

  if (_params.sounding_use_wet_bulb_temp) {
    tempProfile.setUseWetBulbTemp(true);
  }
  
  time_t retrievedTime;
  if (tempProfile.loadFromSpdb(_params.sounding_spdb_url,
                               beamTime,
                               retrievedTime)) {
    cerr << "ERROR - PartRain::tempProfileInit" << endl;
    cerr << "  Cannot retrive profile for time: "
         << DateTime::strm(beamTime) << endl;
    cerr << "  url: " << _params.sounding_spdb_url << endl;
    cerr << "  station name: " << _params.sounding_location_name << endl;
    cerr << "  time margin secs: " << _params.sounding_search_time_margin_secs << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "=====================================" << endl;
    cerr << "Got temp profile, URL: " << _params.sounding_spdb_url << endl;
    cerr << "Overriding temperature profile" << endl;
    cerr << "  vol time: " << DateTime::strm(beamTime) << endl;
    cerr << "  retrievedTime: " << DateTime::strm(retrievedTime) << endl;
    cerr << "  freezingLevel: " << tempProfile.getFreezingLevel() << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    const vector<TempProfile::PointVal> &retrievedProfile = tempProfile.getProfile();
    cerr << "=====================================" << endl;
    cerr << "Temp  profile" << endl;
    int nLevels = (int) retrievedProfile.size();
    int nPrint = 50;
    int printInterval = nLevels / nPrint;
    if (nLevels < nPrint) {
      printInterval = 1;
    }
    for (size_t ii = 0; ii < retrievedProfile.size(); ii++) {
      bool doPrint = false;
      if (ii % printInterval == 0) {
        doPrint = true;
      }
      if (ii < retrievedProfile.size() - 1) {
        if (retrievedProfile[ii].tmpC * retrievedProfile[ii+1].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (ii > 0) {
        if (retrievedProfile[ii-1].tmpC * retrievedProfile[ii].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (doPrint) {
        cerr << "  ilevel, press(Hpa), alt(km), temp(C): " << ii << ", "
             << retrievedProfile[ii].pressHpa << ", "
             << retrievedProfile[ii].htKm << ", "
             << retrievedProfile[ii].tmpC << endl;
      }
    }
    cerr << "=====================================" << endl;
  }
  
  // accept the profile

  _pid.setTempProfile(tempProfile);
  
  return 0;
  
}

