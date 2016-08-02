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
///////////////////////////////////////////////////////////////
// RapidDow2Dsr.cc
//
// RapidDow2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2011
//
///////////////////////////////////////////////////////////////
//
// RapidDow2Dsr reads radial moments data from the RapidDow,
// corrects the data appropriately for pointing angles and
// range, and writes the data to a DsRadarQueue beam by beam.
//
////////////////////////////////////////////////////////////////

#include <Radx/RadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxReadDir.hh>
#include <vector>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <rapmath/math_macros.h>
#include <rapformats/DsRadarMsg.hh>
#include "RapidDow2Dsr.hh"
using namespace std;

// Constructor

RapidDow2Dsr::RapidDow2Dsr(int argc, char **argv)

{

  isOK = true;
  _input = NULL;
  
  _channelNum = 0;
  _outputNRays = 0;
  _outputRayNum = 0;
  _outputDeltaTime = 0;

  _fileStartTimeSecs = 0;
  _fileStartPartialSecs = 0;
  _fileDuration = 0;

  _endOfVol = false;
  _minEl = 180.0;
  _maxEl = -180.0;
  _volNum = 0;
  _nBeamsInVol = 1000000;
  _sweepNum = 0;

  // set programe name
  
  _progName = "RapidDow2Dsr";
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
    return;
  }

  // check that start and end time is set in archive mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // create the data input object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_latest_data_info,
			     _params.latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression != Params::NO_COMPRESSION,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_rQueue.init(_params.output_url,
		     _progName.c_str(),
                     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression != Params::NO_COMPRESSION,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - RapidDow2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      isOK = false;
      return;
    }
  }
      
  _rQueue.setCompressionMethod((ta_compression_method_t)
			       _params.output_compression);

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _rQueue.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  return;

}

// destructor

RapidDow2Dsr::~RapidDow2Dsr()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RapidDow2Dsr::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input->reset();
  char *input_file_path;
  
  while ((input_file_path = _input->next()) != NULL) {
    
    PMU_auto_register("In main loop");
    
    if (_processFile(input_file_path)) {
      cerr << "ERROR - RapidDow2Dsr::Run" << endl;
      cerr << "  Cannot process file: " << input_file_path << endl;
      iret = -1;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// process this file

int RapidDow2Dsr::_processFile(const string filePath)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  int iret = 0;
  for (int ii = 0; ii < _params.channels_n; ii++) {
    _channelNum = ii;
    _channel = _params._channels[_channelNum];
    if (_processFileForChannel(filePath)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// process channel for this file

int RapidDow2Dsr::_processFileForChannel(const string filePath)
  
{

  if (_params.debug) {
    cerr << "Processing channel: " << _channelNum << endl;
  }

  // find the file for the channel

  string chanPath;
  if (_findFileForChannel(filePath, chanPath)) {
    cerr << "ERROR - RapidDow2Dsr::_processFileForChannel" << endl;
    cerr << "  Cannot match file for channel: " << _channelNum << endl;
    cerr << "  Input path: " << filePath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Processing channel file: " << chanPath << endl;
  }

  // set up RadxFile object
  
  RadxFile file;
  _setupRead(file);
  
  // read in file

  if (file.readFromPath(chanPath, _vol)) {
    cerr << "ERROR - RapidDow2Dsr::Run" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _vol.print(cerr);
  } else if (_params.debug >= Params::DEBUG_EXTRA) {
    _vol.printWithRayMetaData(cerr);
  }

  // for first channel, compute times

  if (_channelNum == 0) {
    _computeTimes();
  }
  
  // load fields onto volume

  _vol.loadFieldsFromRays();
  
  // convert all fields to same encoding
  
  _convertFieldsToUniformType();
  
  // loop through the sweeps
  
  _rQueue.putStartOfTilt(_sweepNum);

  int iret = 0;
  for (int ii = 0; ii < (int) _vol.getSweeps().size(); ii++) {
    if (_processSweep(ii)) {
      iret = -1;
    }
  }

  _rQueue.putEndOfTilt(_sweepNum);
  _sweepNum++;

  return iret;

}

//////////////////////////////////////////////////
// get the file path for a given channel

int RapidDow2Dsr::_findFileForChannel(const string filePath,
                                      string &chanPath)
  
{

  // initialize

  chanPath = "";

  // compute search string

  RadxPath fpath(filePath);
  string fileName(fpath.getFile());
  string searchPath(fileName.substr(0, 18));

  RadxReadDir rdir;
  
  if (rdir.open(_channel.dir)) {
    cerr << "ERROR - RapidDow2Dsr::_findFileForChannel" << endl;
    cerr << "  Channel num: " << _channelNum << endl;
    cerr << "  Cannot open directory for reading: " << _channel.dir << endl;
    return -1;
  }
  
  // Loop through directory looking for the data file names
  // or forecast directories
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    string dName(dp->d_name);

    if (dName.find(searchPath) != string::npos) {
      chanPath = _channel.dir;
      chanPath += PATH_DELIM;
      chanPath += dName;
      rdir.close();
      return 0;
    }

  } // dp
    
  return -1;

}

//////////////////////////////////////////////////
// compute the times

void RapidDow2Dsr::_computeTimes()

{

  _fileStartTimeSecs = _vol.getStartTimeSecs();
  _fileStartPartialSecs = _vol.getStartNanoSecs() / 1.0e9;
  
  _fileDuration =
    (double) (_vol.getEndTimeSecs() - _vol.getStartTimeSecs()) +
    (_vol.getEndNanoSecs() - _vol.getStartNanoSecs()) / 1.0e9;
    
  _outputNRays = _vol.getNRays() * _params.channels_n;
  _outputRayNum = 0;
  _outputDeltaTime = _fileDuration / (_outputNRays + 1);

}

//////////////////////////////////////////////////
// process a sweep

int RapidDow2Dsr::_processSweep(int sweepIndex)

{

  const RadxSweep &sweep = *_vol.getSweeps()[sweepIndex];

  // write parameters
  
  if (_writeParams()) {
    cerr << "ERROR - RapidDow2Dsr::_processSweep" << endl;
    cerr << "  Cannot write params" << endl;
    return -1;
  }

  if (sweepIndex == 0) {
    if (_writeCalibration()) {
      cerr << "ERROR - RapidDow2Dsr::_processSweep" << endl;
      cerr << "  Cannot write calibration" << endl;
      return -1;
    }
  }

  // write beams

  for (int ii = sweep.getStartRayIndex(); ii <= sweep.getEndRayIndex(); ii++) {
    if (_writeRay(*_vol.getRays()[ii])) {
      return -1;
    }
  }

  return 0;

}
  
//////////////////////////////////////////////////
// set up read for 

void RapidDow2Dsr::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.print(cerr);
  }
  
}

//////////////////////////////////////////////////
// convert all fields to same data type
// widening as required

void RapidDow2Dsr::_convertFieldsToUniformType()

{

  // search for the narrowest data type which works for all
  // fields
  
  _dataType = Radx::SI08;
  _dataByteWidth = 1;
  for (int ii = 0; ii < (int) _vol.getFields().size(); ii++) {
    const RadxField &fld = *_vol.getFields()[ii];
    if (fld.getByteWidth() > _dataByteWidth) {
      _dataByteWidth = fld.getByteWidth();
    }
  }

  if (_dataByteWidth == 1) {
    _dataType = Radx::SI08;
  } else if (_dataByteWidth == 2) {
    _dataType = Radx::SI16;
    for (int ii = 0; ii < (int) _vol.getFields().size(); ii++) {
      _vol.getFields()[ii]->convertToSi16();
    }
  } else {
    _dataType = Radx::FL32;
    for (int ii = 0; ii < (int) _vol.getFields().size(); ii++) {
      _vol.getFields()[ii]->convertToFl32();
    }
  }
    
  // ensure we have aconstant number of gates

  _vol.setNGatesConstant();

}

//////////////////////////////////////////////////
// write radar and field parameters

int RapidDow2Dsr::_writeParams()

{

  // radar parameters

  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();

  rparams.radarId = 0;
  rparams.radarType = _getDsRadarType(_vol.getPlatformType());
  rparams.numFields = _vol.getFields().size();
  rparams.numGates = _vol.getMaxNGates();
  if (_vol.getRays().size() > 0) {
    rparams.samplesPerBeam = _vol.getRays()[0]->getNSamples();
  }
  rparams.scanType = 0;
  if (_vol.getSweeps().size() > 0) {
    const RadxSweep &sweep0 = *(_vol.getSweeps()[0]);
    rparams.scanMode = _getDsScanMode(sweep0.getSweepMode());
    rparams.followMode = _getDsFollowMode(sweep0.getFollowMode());
    rparams.polarization =
      _getDsPolarizationMode(sweep0.getPolarizationMode());
    rparams.scanTypeName =
      Radx::sweepModeToStr(sweep0.getSweepMode());
    if (_vol.getRays().size() > 0) {
      const RadxRay &ray = *_vol.getRays()[0];
      rparams.prfMode = _getDsPrfMode(sweep0.getPrtMode(),
                                      ray.getPrtRatio());
    }
  }

  if (_vol.getCalibs().size() > 0) {
    rparams.radarConstant = _vol.getCalibs()[0]->getRadarConstantH();
  }

  rparams.altitude = _vol.getAltitudeKm();
  rparams.latitude = _vol.getLatitudeDeg();
  rparams.longitude = _vol.getLongitudeDeg();

  if (_params.override_radar_location) {
    rparams.altitude = _params.radar_altitude_km;
    rparams.latitude = _params.radar_latitude;
    rparams.longitude = _params.radar_longitude;
  }

  rparams.gateSpacing = _channel.gate_spacing;
  rparams.startRange = _channel.start_range;
  rparams.horizBeamWidth = _vol.getRadarBeamWidthDegH();
  rparams.vertBeamWidth = _vol.getRadarBeamWidthDegV();
  rparams.antennaGain = _vol.getRadarAntennaGainDbH();

  double freqHz = _channel.frequency * 1.0e9;
  double wavelengthCm = (Radx::LIGHT_SPEED / freqHz) * 100.0;
  rparams.wavelength = wavelengthCm;

  if (_vol.getRays().size() > 0) {
    const RadxRay &ray = *_vol.getRays()[0];
    rparams.pulseWidth = ray.getPulseWidthUsec();
    rparams.pulseRepFreq = 1.0 / ray.getPrtSec();
    rparams.prt = ray.getPrtSec();
    rparams.prt2 = ray.getPrtSec() / ray.getPrtRatio();
    rparams.unambigRange = ray.getUnambigRangeKm();
    rparams.unambigVelocity = ray.getNyquistMps();
  }

  if (_vol.getCalibs().size() > 0) {
    const RadxRcalib &cal = *_vol.getCalibs()[0];
    rparams.xmitPeakPower = pow(10.0, cal.getXmitPowerDbmH() / 10.0) / 1000.0;
    rparams.receiverGain = cal.getReceiverGainDbHc();
    rparams.receiverMds = cal.getNoiseDbmHc() - rparams.receiverGain;
    rparams.systemGain = rparams.antennaGain + rparams.receiverGain;
    rparams.measXmitPowerDbmH = cal.getXmitPowerDbmH();
    rparams.measXmitPowerDbmV = cal.getXmitPowerDbmV();
  }

  rparams.radarName = _vol.getInstrumentName() + "/" + _vol.getSiteName();
  if (_params.override_radar_name) {
    rparams.radarName = _params.radar_name;
  }
  rparams.scanTypeName = _vol.getScanName();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    rparams.print(cerr);
  }

  // field parameters

  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  
  for (int ifield = 0; ifield < (int) _vol.getFields().size(); ifield++) {

    const RadxField &fld = *_vol.getFields()[ifield];
    double dsScale = 1.0, dsBias = 0.0;
    int dsMissing = 0;
    if (_dataType == Radx::SI08) {
      dsMissing = fld.getMissingSi08() + 128;
      dsScale = fld.getScale();
      dsBias = fld.getOffset() - 128.0 * dsScale;
    } else if (_dataType == Radx::SI16) {
      dsMissing = fld.getMissingSi16() + 32768;
      dsScale = fld.getScale();
      dsBias = fld.getOffset() - 32768.0 * dsScale;
    } else if (_dataType == Radx::FL32) {
      dsMissing = (int) floor(fld.getMissingFl32() + 0.5);
    }

    DsFieldParams *fParams = new DsFieldParams(fld.getName().c_str(),
                                               fld.getUnits().c_str(),
                                               dsScale, dsBias,
                                               _dataByteWidth,
                                               dsMissing);
    fieldParams.push_back(fParams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fParams->print(cerr);
    }

  } // ifield

  // put params
  
  int content = DsRadarMsg::FIELD_PARAMS | DsRadarMsg::RADAR_PARAMS;
  if(_rQueue.putDsMsg(msg, content)) {
    cerr << "ERROR - RapidDow2Dsr::_writeFieldParams()" << endl;
    cerr << "  Cannot write field params to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write calibration

int RapidDow2Dsr::_writeCalibration()
  
{

  if (_vol.getCalibs().size() < 1) {
    // no cal data
    return 0;
  }

  // use first calibration

  DsRadarMsg msg;
  DsRadarCalib &calOut = msg.getRadarCalib();
  const RadxRcalib &calIn = *_vol.getCalibs()[0];

  calOut.setCalibTime(calIn.getCalibTime());

  calOut.setWavelengthCm(_vol.getWavelengthCm());
  calOut.setBeamWidthDegH(_vol.getRadarBeamWidthDegH());
  calOut.setBeamWidthDegV(_vol.getRadarBeamWidthDegV());
  calOut.setAntGainDbH(_vol.getRadarAntennaGainDbH());
  calOut.setAntGainDbV(_vol.getRadarAntennaGainDbV());

  calOut.setPulseWidthUs(calIn.getPulseWidthUsec());
  calOut.setXmitPowerDbmH(calIn.getXmitPowerDbmH());
  calOut.setXmitPowerDbmV(calIn.getXmitPowerDbmV());
  
  calOut.setTwoWayWaveguideLossDbH(calIn.getTwoWayWaveguideLossDbH());
  calOut.setTwoWayWaveguideLossDbV(calIn.getTwoWayWaveguideLossDbV());
  calOut.setTwoWayRadomeLossDbH(calIn.getTwoWayRadomeLossDbH());
  calOut.setTwoWayRadomeLossDbV(calIn.getTwoWayRadomeLossDbV());
  calOut.setReceiverMismatchLossDb(calIn.getReceiverMismatchLossDb());
  
  calOut.setRadarConstH(calIn.getRadarConstantH());
  calOut.setRadarConstV(calIn.getRadarConstantV());
  
  calOut.setNoiseDbmHc(calIn.getNoiseDbmHc());
  calOut.setNoiseDbmHx(calIn.getNoiseDbmHx());
  calOut.setNoiseDbmVc(calIn.getNoiseDbmVc());
  calOut.setNoiseDbmVx(calIn.getNoiseDbmVx());
  
  calOut.setReceiverGainDbHc(calIn.getReceiverGainDbHc());
  calOut.setReceiverGainDbHx(calIn.getReceiverGainDbHx());
  calOut.setReceiverGainDbVc(calIn.getReceiverGainDbVc());
  calOut.setReceiverGainDbVx(calIn.getReceiverGainDbVx());
  
  calOut.setReceiverSlopeDbHc(calIn.getReceiverSlopeDbHc());
  calOut.setReceiverSlopeDbHx(calIn.getReceiverSlopeDbHx());
  calOut.setReceiverSlopeDbVc(calIn.getReceiverSlopeDbVc());
  calOut.setReceiverSlopeDbVx(calIn.getReceiverSlopeDbVx());
  
  calOut.setBaseDbz1kmHc(calIn.getBaseDbz1kmHc());
  calOut.setBaseDbz1kmHx(calIn.getBaseDbz1kmHx());
  calOut.setBaseDbz1kmVc(calIn.getBaseDbz1kmVc());
  calOut.setBaseDbz1kmVx(calIn.getBaseDbz1kmVx());
  
  calOut.setSunPowerDbmHc(calIn.getSunPowerDbmHc());
  calOut.setSunPowerDbmHx(calIn.getSunPowerDbmHx());
  calOut.setSunPowerDbmVc(calIn.getSunPowerDbmVc());
  calOut.setSunPowerDbmVx(calIn.getSunPowerDbmVx());
  
  calOut.setNoiseSourcePowerDbmH(calIn.getNoiseSourcePowerDbmH());
  calOut.setNoiseSourcePowerDbmV(calIn.getNoiseSourcePowerDbmV());
  
  calOut.setPowerMeasLossDbH(calIn.getPowerMeasLossDbH());
  calOut.setPowerMeasLossDbV(calIn.getPowerMeasLossDbV());
  
  calOut.setCouplerForwardLossDbH(calIn.getCouplerForwardLossDbH());
  calOut.setCouplerForwardLossDbV(calIn.getCouplerForwardLossDbV());
  
  calOut.setZdrCorrectionDb(calIn.getZdrCorrectionDb());
  calOut.setLdrCorrectionDbH(calIn.getLdrCorrectionDbH());
  calOut.setLdrCorrectionDbV(calIn.getLdrCorrectionDbV());
  calOut.setSystemPhidpDeg(calIn.getSystemPhidpDeg());
  
  calOut.setTestPowerDbmH(calIn.getTestPowerDbmH());
  calOut.setTestPowerDbmV(calIn.getTestPowerDbmV());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    calOut.print(cerr);
  }
  
  // put calibration
  
  int content = (int) DsRadarMsg::RADAR_CALIB;
  if(_rQueue.putDsMsg(msg, content)) {
    cerr << "ERROR - RapidDow2Dsr::_writeCalibration()" << endl;
    cerr << "  Cannot write calibration to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write ray

int RapidDow2Dsr::_writeRay(const RadxRay &ray)
  
{

  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  // meta-data

  double timeSinceFileStart = _outputRayNum * _outputDeltaTime;
  _outputRayNum++;
  time_t timeSecs = _fileStartTimeSecs + (int) timeSinceFileStart;
  int nanoSecs = (int) (fmod(timeSinceFileStart, 1.0) * 1.0e9 + 0.5);

  beam.dataTime = timeSecs;
  beam.nanoSecs = nanoSecs;
  beam.referenceTime = 0;

  beam.byteWidth = _dataByteWidth;

  beam.volumeNum = _volNum;
  beam.tiltNum = _sweepNum;
  Radx::SweepMode_t sweepMode = ray.getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);

  double az = ray.getAzimuthDeg() + _channel.azimuth_offset;
  if (_params.correct_for_azimuth_error) {
    az += _params.azimuth_correction;
  }
  az = _constrainAz(az);
  double el = ray.getElevationDeg() + _channel.elevation_offset;

  if (el > _maxEl) _maxEl = el;
  if (el < _minEl) _minEl = el;

  // detect end of volume

  _nBeamsInVol++;
  if (_nBeamsInVol == _params.min_beams_in_volume) {
    _minEl = 180.0;
    _maxEl = -180.0;
    _endOfVol = false;
  }

  if (_params.locate_end_of_volume & !_endOfVol && _channelNum == 0) {
    double maxElChange = _params.elev_change_for_end_of_volume;
    double deltaEl = 0;
    if (maxElChange < 0) {
      deltaEl = _maxEl - el;
    } else {
      deltaEl = el - _minEl;
    }
    if (deltaEl > fabs(maxElChange)) {
      if (_params.debug) {
        cerr << "===>> endOfVol: el, _minEl, _maxEl, deltaEl, "
             << "maxElChange, nBeamsVol: "
             << el << ", "
             << _minEl << ", "
             << _maxEl << ", "
             << deltaEl << ", "
             << maxElChange << ", "
             << _nBeamsInVol << endl;
      }
      _endOfVol = true;
      _rQueue.putEndOfVolume(_volNum);
      _volNum++;
      _sweepNum = 0;
      _rQueue.putStartOfVolume(_volNum);
      _minEl = 180.0;
      _maxEl = -180.0;
      _nBeamsInVol = 0;
    }
  }

  beam.azimuth = az;
  beam.elevation = el;
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray.getFixedAngleDeg();
  } else {
    beam.targetElev = ray.getFixedAngleDeg() + _channel.elevation_offset;
  }

  beam.antennaTransition = ray.getAntennaTransition();
  if (_params.set_transition_flag) {
    double elError = fabs(beam.targetElev - beam.elevation);
    if (elError > _params.max_elevation_error) {
      beam.antennaTransition = true;
    } else {
      beam.antennaTransition = false;
    }
  }

  beam.beamIsIndexed = ray.getIsIndexed();
  beam.angularResolution = ray.getAngleResDeg();
  beam.nSamples = ray.getNSamples();

  beam.measXmitPowerDbmH = ray.getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray.getMeasXmitPowerDbmV();

  // field data - must be re-ordered to gate-by-gate

  int nGates = ray.getNGates();
  int nFields = _vol.getFields().size();
  int nPoints = nGates * nFields;

  if (_dataByteWidth == 1) {

    // 1-byte ints

    ui08 *data = new ui08[nPoints];
    for (int ifield = 0; ifield < (int) _vol.getFields().size(); ifield++) {
      ui08 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::si08 *fd = (Radx::si08 *) fld.getData();
      for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
        ui32 uu = *fd + 32768;
        *dd = (ui08) uu;
      }
    }
    beam.loadData(data, nPoints * sizeof(ui08), sizeof(ui08));
    delete[] data;
    
  } else if (_dataByteWidth == 2) {

    // 2-byte ints

    ui16 *data = new ui16[nPoints];
    for (int ifield = 0; ifield < (int) _vol.getFields().size(); ifield++) {
      ui16 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::si16 *fd = (Radx::si16 *) fld.getData();
      for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
        ui32 uu = *fd + 32768;
        *dd = (ui16) uu;
      }
    }
    beam.loadData(data, nPoints * sizeof(ui16), sizeof(ui16));
    delete[] data;
    
  } else {

    // 4-byte floats

    fl32 *data = new fl32[nPoints];
    for (int ifield = 0; ifield < (int) _vol.getFields().size(); ifield++) {
      fl32 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::fl32 *fd = (Radx::fl32 *) fld.getData();
      for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
        *dd = *fd;
      }
    }
    beam.loadData(data, nPoints * sizeof(fl32), sizeof(fl32));
    delete[] data;
    
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    beam.print(cerr);
  }
  
  // put beam
  
  int content = (int) DsRadarMsg::RADAR_BEAM;
  if(_rQueue.putDsMsg(msg, content)) {
    cerr << "ERROR - RapidDow2Dsr::_writeBeam()" << endl;
    cerr << "  Cannot write beam to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// constrain azimuth

double RapidDow2Dsr::_constrainAz(double az)
  
{

  if (az < 0) {
    return az + 360.0;
  }

  if (az > 360) {
    return az - 360.0;
  }

  return az;

}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int RapidDow2Dsr::_getDsRadarType(Radx::PlatformType_t ptype)

{
  switch (ptype) {
    case Radx::PLATFORM_TYPE_VEHICLE:
      return DS_RADAR_VEHICLE_TYPE;
    case Radx::PLATFORM_TYPE_SHIP:
      return DS_RADAR_SHIPBORNE_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_FORE:
      return DS_RADAR_AIRBORNE_FORE_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_AFT:
      return DS_RADAR_AIRBORNE_AFT_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL:
      return DS_RADAR_AIRBORNE_TAIL_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY:
      return DS_RADAR_AIRBORNE_LOWER_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF:
      return DS_RADAR_AIRBORNE_UPPER_TYPE;
    default:
      return DS_RADAR_GROUND_TYPE;
  }
}

int RapidDow2Dsr::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

int RapidDow2Dsr::_getDsFollowMode(Radx::FollowMode_t mode)

{
  switch (mode) {
    case Radx::FOLLOW_MODE_SUN:
      return DS_RADAR_FOLLOW_MODE_SUN;
    case Radx::FOLLOW_MODE_VEHICLE:
      return DS_RADAR_FOLLOW_MODE_VEHICLE;
    case Radx::FOLLOW_MODE_AIRCRAFT:
      return DS_RADAR_FOLLOW_MODE_AIRCRAFT;
    case Radx::FOLLOW_MODE_TARGET:
      return DS_RADAR_FOLLOW_MODE_TARGET;
    case Radx::FOLLOW_MODE_MANUAL:
      return DS_RADAR_FOLLOW_MODE_MANUAL;
    default:
      return DS_RADAR_FOLLOW_MODE_NONE;
  }
}

int RapidDow2Dsr::_getDsPolarizationMode(Radx::PolarizationMode_t mode)

{
  switch (mode) {
    case Radx::POL_MODE_HORIZONTAL:
      return DS_POLARIZATION_HORIZ_TYPE;
    case Radx::POL_MODE_VERTICAL:
      return DS_POLARIZATION_VERT_TYPE;
    case Radx::POL_MODE_HV_ALT:
      return DS_POLARIZATION_DUAL_HV_ALT;
    case Radx::POL_MODE_HV_SIM:
      return DS_POLARIZATION_DUAL_HV_SIM;
    case Radx::POL_MODE_CIRCULAR:
      return DS_POLARIZATION_RIGHT_CIRC_TYPE;
    default:
      return DS_POLARIZATION_HORIZ_TYPE;
  }
}

int RapidDow2Dsr::_getDsPrfMode(Radx::PrtMode_t mode,
                                double prtRatio)
  
{
  switch (mode) {
    case Radx::PRT_MODE_FIXED:
      return DS_RADAR_PRF_MODE_FIXED;
    case Radx::PRT_MODE_STAGGERED:
    case Radx::PRT_MODE_DUAL:
      if (fabs(prtRatio - 0.6667 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_2_3;
      } else if (fabs(prtRatio - 0.75 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_3_4;
      } else if (fabs(prtRatio - 0.8 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_4_5;
      } else {
        return DS_RADAR_PRF_MODE_FIXED;
      }
    default:
      return DS_RADAR_PRF_MODE_FIXED;
  }
}

