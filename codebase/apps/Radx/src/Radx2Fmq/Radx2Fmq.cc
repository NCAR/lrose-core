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
// Radx2Fmq.cc
//
// Radx2Fmq object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2025
//
///////////////////////////////////////////////////////////////
//
// Radx2Fmq reads radial moments data from Radx-supported files
// and writes the data to a DsRadarQueue beam by beam in Radx format.
//
////////////////////////////////////////////////////////////////

#include <vector>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/toolsa_macros.h>
#include <rapformats/DsRadarMsg.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include "Radx2Fmq.hh"
using namespace std;

// Constructor

Radx2Fmq::Radx2Fmq(int argc, char **argv)

{

  isOK = true;
  _input = NULL;
  _prevNGates = -1;
  _prevSweepNum = -1;

  // set programe name
  
  _progName = "Radx2Fmq";
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
    _input->setFileQuiescence(_params.realtime_file_quiescence);
    _input->setDirScanSleep(_params.realtime_wait_between_scans);
    _input->setRecursion(_params.realtime_search_recursively);
    _input->setMaxRecursionDepth(_params.realtime_max_recursion_depth);
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
      cerr << "ERROR - Radx2Fmq" << endl;
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

Radx2Fmq::~Radx2Fmq()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Radx2Fmq::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.do_simulate) {
    return _runSimulate();
  }

  // loop until end of data
  
  _input->reset();
  char *input_file_path;
  
  while ((input_file_path = _input->next()) != NULL) {
    
    PMU_auto_register("In main loop");
    
    if (_processFile(input_file_path)) {
      cerr << "ERROR - Radx2Fmq::Run" << endl;
      cerr << "  Cannot process file: " << input_file_path << endl;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// Run in simulate mode

int Radx2Fmq::_runSimulate()
{
  
  int iret = 0;
  
  // loop until end of data

  while (true) {

    _input->reset();
    char *input_file_path;
    
    while ((input_file_path = _input->next()) != NULL) {
      
      PMU_auto_register("_runSimulate");
      
      if (_processFile(input_file_path)) {
        cerr << "ERROR - Radx2Fmq::Run" << endl;
        cerr << "  Cannot process file: " << input_file_path << endl;
      }
      
    } // while((input_file_path ...

  } // while (true)
  
  return iret;

}

//////////////////////////////////////////////////
// process this file

int Radx2Fmq::_processFile(const string filePath)
  
{

  // check we have not already processed this file
  // in the file aggregation step

  RadxPath thisPath(filePath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  // set up RadxFile object
  
  GenericRadxFile file;
  _setupRead(file);
  
  // read in file

  RadxVol vol;
  if (file.readFromPath(filePath, vol)) {
    cerr << "ERROR - Radx2Fmq::Run" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  _readPaths = file.getReadPaths();
  if (_params.debug) {
    cerr << "Following file paths used: " << endl;
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "    " << _readPaths[ii] << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    vol.printWithRayMetaData(cerr);
    // vol.printWithFieldData(cerr);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    vol.print(cerr);
  }
  
  // load fields onto volume, forcing all rays to
  // have all fields

  vol.loadFieldsFromRays(true);
  
  // convert all fields to same encoding

  _convertFieldsToUniformType(vol);

  // put start of volume

  _rQueue.putStartOfVolume(vol.getVolumeNumber());

  // put status XML is appropriate

  _writeStatusXml(vol);

  // loop through the sweeps

  int iret = 0;
  for (int ii = 0; ii < (int) vol.getSweeps().size(); ii++) {
    if (_processSweep(vol, ii)) {
      iret = -1;
    }
  }

  // put end of volume

  _rQueue.putEndOfVolume(vol.getVolumeNumber());

  return iret;

}

//////////////////////////////////////////////////
// process a sweep

int Radx2Fmq::_processSweep(const RadxVol &vol,
                            int sweepIndex)

{

  const RadxSweep &sweep = *vol.getSweeps()[sweepIndex];

  // put start of tilt

  _rQueue.putStartOfTilt(sweep.getSweepNumber());

  if (sweepIndex == 0) {
    if (_writeCalibration(vol)) {
      cerr << "ERROR - Radx2Fmq::_processSweep" << endl;
      cerr << "  Cannot write calibration" << endl;
      return -1;
    }
  }

  // write beams

  for (size_t ii = sweep.getStartRayIndex();
       ii <= sweep.getEndRayIndex(); ii++) {
    if (_writeBeam(vol, sweep, ii, *vol.getRays()[ii])) {
      return -1;
    }
  }

  // put end of tilt

  _rQueue.putEndOfTilt(sweep.getSweepNumber());

  return 0;

}
  
//////////////////////////////////////////////////
// set up read 

void Radx2Fmq::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.read_set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  } else if (_params.read_set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.read_lower_sweep_num,
                               _params.read_upper_sweep_num);
  }

  if (_params.read_set_field_names) {
    for (int i = 0; i < _params.read_field_names_n; i++) {
      file.addReadField(_params._read_field_names[i]);
    }
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.remove_rays_with_all_data_missing) {
    file.setReadRemoveRaysAllMissing(true);
  }

  if (_params.remove_long_range_rays) {
    file.setReadRemoveLongRange(true);
  }

  if (_params.remove_short_range_rays) {
    file.setReadRemoveShortRange(true);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// convert all fields to same data type
// widening as required

void Radx2Fmq::_convertFieldsToUniformType(RadxVol &vol)

{

  // search for the narrowest data type which works for all
  // fields
  
  _dataByteWidth = 1;
  for (int ii = 0; ii < (int) vol.getFields().size(); ii++) {
    const RadxField &fld = *vol.getFields()[ii];
    if (fld.getByteWidth() > _dataByteWidth) {
      _dataByteWidth = fld.getByteWidth();
    }
  }

  if (_dataByteWidth == 1) {
    _dataType = Radx::SI08;
  } else if (_dataByteWidth == 2) {
    _dataType = Radx::SI16;
  } else {
    _dataType = Radx::FL32;
  }
  
  vol.setFieldsToUniformType(_dataType);

}

//////////////////////////////////////////////////
// write radar and field parameters

int Radx2Fmq::_writeParams(const RadxVol &vol,
                           const RadxSweep &sweep,
                           const RadxRay &ray)

{

  // radar parameters

  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();
  
  rparams.radarId = 0;
  rparams.radarType = _getDsRadarType(vol.getPlatformType());
  rparams.numFields = ray.getFields().size();
  rparams.numGates = ray.getNGates();
  rparams.samplesPerBeam = ray.getNSamples();
  rparams.scanType = 0;
  rparams.scanMode = _getDsScanMode(sweep.getSweepMode());
  rparams.followMode = _getDsFollowMode(sweep.getFollowMode());
  rparams.polarization =
    _getDsPolarizationMode(sweep.getPolarizationMode());
  rparams.scanTypeName =
    Radx::sweepModeToStr(sweep.getSweepMode());
  rparams.prfMode = _getDsPrfMode(sweep.getPrtMode(), ray.getPrtRatio());
  
  if (vol.getRcalibs().size() > 0) {
    rparams.radarConstant = vol.getRcalibs()[0]->getRadarConstantH();
  }

  rparams.altitude = vol.getAltitudeKm();
  rparams.latitude = vol.getLatitudeDeg();
  rparams.longitude = vol.getLongitudeDeg();
  rparams.gateSpacing = vol.getGateSpacingKm();
  rparams.startRange = vol.getStartRangeKm();
  rparams.horizBeamWidth = vol.getRadarBeamWidthDegH();
  rparams.vertBeamWidth = vol.getRadarBeamWidthDegV();
  rparams.antennaGain = vol.getRadarAntennaGainDbH();
  rparams.wavelength = vol.getWavelengthM() * 100.0;

  rparams.pulseWidth = ray.getPulseWidthUsec();
  rparams.pulseRepFreq = 1.0 / ray.getPrtSec();
  rparams.prt = ray.getPrtSec();
  rparams.prt2 = ray.getPrtSec() / ray.getPrtRatio();
  rparams.unambigRange = ray.getUnambigRangeKm();
  rparams.unambigVelocity = ray.getNyquistMps();

  if (vol.getRcalibs().size() > 0) {
    const RadxRcalib &cal = *vol.getRcalibs()[0];
    rparams.xmitPeakPower = pow(10.0, cal.getXmitPowerDbmH() / 10.0) / 1000.0;
    rparams.receiverGain = cal.getReceiverGainDbHc();
    rparams.receiverMds = cal.getNoiseDbmHc() - rparams.receiverGain;
    rparams.systemGain = rparams.antennaGain + rparams.receiverGain;
    rparams.measXmitPowerDbmH = cal.getXmitPowerDbmH();
    rparams.measXmitPowerDbmV = cal.getXmitPowerDbmV();
  }

  rparams.radarName = vol.getInstrumentName() + "/" + vol.getSiteName();
  rparams.scanTypeName = vol.getScanName();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    rparams.print(cerr);
  }

  // field parameters

  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  
  for (int ifield = 0; ifield < (int) vol.getFields().size(); ifield++) {

    const RadxField &fld = *vol.getFields()[ifield];
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
    cerr << "ERROR - Radx2Fmq::_writeFieldParams()" << endl;
    cerr << "  Cannot write field params to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write status xml

int Radx2Fmq::_writeStatusXml(const RadxVol &vol)
  
{

  string statusXml = vol.getStatusXml();
  if (statusXml.size() == 0) {
    return 0;
  }

  // create DsRadar message
  
  DsRadarMsg msg;
  msg.setStatusXml(statusXml);
  
  // write the message
  
  if (_rQueue.putDsMsg(msg, DsRadarMsg::STATUS_XML)) {
    cerr << "ERROR - Radx2Fmq::_writeStatusXml" << endl;
    cerr << "  Cannot put status XML to queue" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// write calibration

int Radx2Fmq::_writeCalibration(const RadxVol &vol)
  
{

  if (vol.getRcalibs().size() < 1) {
    // no cal data
    return 0;
  }

  // use first calibration

  DsRadarMsg msg;
  DsRadarCalib &calOut = msg.getRadarCalib();
  const RadxRcalib &calIn = *vol.getRcalibs()[0];

  calOut.setCalibTime(calIn.getCalibTime());

  calOut.setWavelengthCm(vol.getWavelengthCm());
  calOut.setBeamWidthDegH(vol.getRadarBeamWidthDegH());
  calOut.setBeamWidthDegV(vol.getRadarBeamWidthDegV());
  calOut.setAntGainDbH(vol.getRadarAntennaGainDbH());
  calOut.setAntGainDbV(vol.getRadarAntennaGainDbV());

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
    cerr << "ERROR - Radx2Fmq::_writeCalibration()" << endl;
    cerr << "  Cannot write calibration to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write beam

int Radx2Fmq::_writeBeam(const RadxVol &vol,
                         const RadxSweep &sweep,
                         int rayNumInSweep,
                         const RadxRay &ray)
  
{

  // write params if needed

  int nGates = ray.getNGates();
  const vector<RadxField *> &fields = vol.getFields();
  int nFields = vol.getFields().size();
  int nPoints = nGates * nFields;

  bool needWriteParams = false;
  
  if (rayNumInSweep == 0 || _prevNGates != nGates) {
    needWriteParams = true;
  }

  if (_prevSweepNum != ray.getSweepNumber()) {
    needWriteParams = true;
    _prevSweepNum = ray.getSweepNumber();
  }

  if ((int) _prevFieldNames.size() != nFields) {
    needWriteParams = true;
  } else {
    for (int ii = 0; ii < nFields; ii++) {
      if (_prevFieldNames[ii] != fields[ii]->getName()) {
        needWriteParams = true;
      }
    }
  }

  if (needWriteParams) {

    _prevNGates = nGates;
    _prevFieldNames.clear();
    for (int ii = 0; ii < nFields; ii++) {
      _prevFieldNames.push_back(fields[ii]->getName());
    }
  
    if (_writeParams(vol, sweep, ray)) {
      cerr << "ERROR - Radx2Fmq::_writeBeam" << endl;
      cerr << "  Cannot write params" << endl;
      return -1;
    }

  }

  // meta-data

  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  beam.dataTime = ray.getTimeSecs();
  beam.nanoSecs = (int) (ray.getNanoSecs() + 0.5);
  beam.referenceTime = 0;

  beam.byteWidth = _dataByteWidth;

  beam.volumeNum = ray.getVolumeNumber();
  beam.tiltNum = ray.getSweepNumber();
  Radx::SweepMode_t sweepMode = ray.getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);
  beam.antennaTransition = ray.getAntennaTransition();
  
  beam.azimuth = ray.getAzimuthDeg();
  beam.elevation = ray.getElevationDeg();
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray.getFixedAngleDeg();
  } else {
    beam.targetElev = ray.getFixedAngleDeg();
  }

  beam.beamIsIndexed = ray.getIsIndexed();
  beam.angularResolution = ray.getAngleResDeg();
  beam.nSamples = ray.getNSamples();

  beam.measXmitPowerDbmH = ray.getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray.getMeasXmitPowerDbmV();

  // field data - must be re-ordered to gate-by-gate

  if (_dataByteWidth == 1) {

    // 1-byte ints

    ui08 *data = new ui08[nPoints];
    for (int ifield = 0; ifield < nFields; ifield++) {
      ui08 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::si08 *fd = (Radx::si08 *) fld.getData();
      if (fd == NULL) {
        cerr << "ERROR - Radx2Fmq::_writeBeam" << endl;
        cerr << "  NULL data pointer, field name, elev, az: "
             << fld.getName() << ", "
             << ray.getElevationDeg() << ", "
             << ray.getAzimuthDeg() << endl;
        delete[] data;
        return -1;
      }
      for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
        int uu = *fd + 128;
        *dd = (ui08) uu;
      }
    }
    beam.loadData(data, nPoints * sizeof(ui08), sizeof(ui08));
    delete[] data;
    
  } else if (_dataByteWidth == 2) {

    // 2-byte ints

    ui16 *data = new ui16[nPoints];
    for (int ifield = 0; ifield < nFields; ifield++) {
      ui16 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::si16 *fd = (Radx::si16 *) fld.getData();
      if (fd == NULL) {
        cerr << "ERROR - Radx2Fmq::_writeBeam" << endl;
        cerr << "  NULL data pointer, field name, elev, az: "
             << fld.getName() << ", "
             << ray.getElevationDeg() << ", "
             << ray.getAzimuthDeg() << endl;
        delete[] data;
        return -1;
      }
      for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
        int uu = *fd + 32768;
        *dd = (ui16) uu;
      }
    }
    beam.loadData(data, nPoints * sizeof(ui16), sizeof(ui16));
    delete[] data;
    
  } else {

    // 4-byte floats

    fl32 *data = new fl32[nPoints];
    for (int ifield = 0; ifield < nFields; ifield++) {
      fl32 *dd = data + ifield;
      const RadxField &fld = *ray.getFields()[ifield];
      const Radx::fl32 *fd = (Radx::fl32 *) fld.getData();
      if (fd == NULL) {
        cerr << "ERROR - Radx2Fmq::_writeBeam" << endl;
        cerr << "  NULL data pointer, field name, elev, az: "
             << fld.getName() << ", "
             << ray.getElevationDeg() << ", "
             << ray.getAzimuthDeg() << endl;
        delete[] data;
        return -1;
      }
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
    cerr << "ERROR - Radx2Fmq::_writeBeam()" << endl;
    cerr << "  Cannot write beam to FMQ" << endl;
    cerr << "  URL: " << _params.output_url << endl;
    return -1;
  }

  if (_params.do_simulate) {
    umsleep(_params.sim_delay_msecs);
  }

  return 0;

}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int Radx2Fmq::_getDsRadarType(Radx::PlatformType_t ptype)

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

int Radx2Fmq::_getDsScanMode(Radx::SweepMode_t mode)

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

int Radx2Fmq::_getDsFollowMode(Radx::FollowMode_t mode)

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

int Radx2Fmq::_getDsPolarizationMode(Radx::PolarizationMode_t mode)

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

int Radx2Fmq::_getDsPrfMode(Radx::PrtMode_t mode,
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

