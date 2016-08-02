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
// ProSensing2Dsr.cc
//
// ProSensing2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////
//
// ProSensing2Dsr reads radar data from the RDAS relay server and writes
// it to an FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <dataport/bigend.h>
#include <rapformats/DsRadarMsg.hh>
#include "ProSensing2Dsr.hh"
using namespace std;

// Constructor

ProSensing2Dsr::ProSensing2Dsr(int argc, char **argv)

{
  
  isOK = true;
  _rayCount = 0;
  _volNum = 0;
  _tiltNum = 0;
  _scanType = 0;

  _samplesPerBeam = 0;
  _startRange = -9999;
  _gateSpacing = -9999;
  _prf = -9999;

  _rCorrNGates = 0;
  _rCorrStartRange = 0;
  _rCorrGateSpacing = 0;
  _rangeCorrection = NULL;
  _range = NULL;

  _attenReady = false;
  _velSign = 1.0;

  // set programe name
  
  _progName = "ProSensing2Dsr";
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the output queue
  
  if (_rQueue.init(_params.output_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    if (_rQueue.init(_params.output_url,
		     _progName.c_str(),
		     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - ProSensing2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      isOK = false;
      return;
    }
  }
  
  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _xpol.setVerbose();
  } else if (_params.debug) {
    _xpol.setDebug();
  }
  _xpol.setServerHost(_params.xpold_host);
  _xpol.setServerPort(_params.xpold_port);
  if (_params.override_azimuth_offset) {
    _xpol.setOverrideAzOffset(_params.azimuth_offset_deg);
  }

  // read in calibration

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_file_xml_path, errStr)) {
    cerr << "ERROR - ProSensing2Dsr" << endl;
    cerr << "  Cannot read in calibration file: "
         << _params.cal_file_xml_path << endl;
    cerr << errStr << endl;
    isOK = false;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _calib.print(cerr);
  }

  _phidpPhaseLimitSim = -160;
  _systemPhidp = _calib.getSystemPhidpDeg();
  double offsetPhidp = _systemPhidp - _phidpPhaseLimitSim;
  _phidpOffset.re = cos(offsetPhidp * DEG_TO_RAD);
  _phidpOffset.im = sin(offsetPhidp * DEG_TO_RAD);
  
  // calibration

  _calNoiseDbmHc = _calib.getNoiseDbmHc();
  _calNoisePowerHc = pow(10.0, _calNoiseDbmHc / 10.0);
  _calNoiseDbmVc = _calib.getNoiseDbmVc();
  _calNoisePowerVc = pow(10.0, _calNoiseDbmVc / 10.0);

  _estNoisePowerHc = _calNoisePowerHc;
  _estNoisePowerVc = _calNoisePowerVc;

  _rxGainDbHc = _calib.getReceiverGainDbHc();
  _rxGainDbVc = _calib.getReceiverGainDbVc();

  _dbz0 = _calib.getBaseDbz1kmHc();
  _zdrCorrectionDb = _calib.getZdrCorrectionDb();
  
  _wavelengthM = _calib.getWavelengthCm() / 100.0;

  _minDetectableSnr = 0.01; // -20 dB

  return;

}

// destructor

ProSensing2Dsr::~ProSensing2Dsr()

{

  if (_rangeCorrection != NULL) {
    delete[] _rangeCorrection;
  }
  if (_range != NULL) {
    delete[] _range;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ProSensing2Dsr::Run()
{
  
  PMU_auto_register("Run");

  // wait until server is up

  while (_xpol.pingServer()) {
    PMU_auto_register("Waiting for xpold server");
    cerr << "ERROR - ProSensing2Dsr::Run" << endl;
    cerr << " Cannot ping xpold server" << endl;
    cerr << " host: " << _params.xpold_host << endl;
    cerr << " port: " << _params.xpold_port << endl;
    umsleep(1000);
  }
  
  while (true) {
    
    PMU_auto_register("reading data");

    if (_xpol.readData(XpolComms::FUSED_PRODUCTS_PROC_DATA)) {
      cerr << "ERROR - ProSensing2Dsr::Run" << endl;
      continue;
    }

    // do we have new metadata?

    if (_xpol.getMetaDataHasChanged()) {
      _writeRadarAndFieldParams();
    }

    if (_params.debug == Params::DEBUG_VERBOSE) {
      _xpol.printDataResponse(cerr);
    } else if (_params.debug) {
      cerr << "Block index: " << _xpol.getDataResponse().blockIndex << endl;
    }

    if (_params.debug == Params::DEBUG_EXTRA) {
      const fl32 *fdata = (const fl32 *) _xpol.getDataPtr();
      int nGates = _xpol.getConf().nGates;
      for (int ii = 0; ii < nGates; ii++) {
        cerr << "  ii, data: " << ii << ", " << 10.0 * log10(fdata[ii]) << endl;
      }
    }

    // process the ray

    _processRay();
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// process a ray from the radar

int ProSensing2Dsr::_processRay()
  
{
  
  PMU_auto_register("Processing ray");

  _nGates = _xpol.getNGates();
  if (_nGates < 1) {
    _nGates = 1;
  }
  _rayTimeSecs = _xpol.getRayTimeSecs();
  _rayNanoSecs = _xpol.getRayNanoSecs();
  _elDeg = _xpol.getRayElDeg();
  _azDeg = _xpol.getRayAzDeg();

  if (_params.override_time) {
    _rayTimeSecs = time(NULL);
    _rayNanoSecs = 0;
  }
  
  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Processing ray, time secs, nanosecs, el, az: " 
         << DateTime::strm(_rayTimeSecs) << ", "
         << _rayNanoSecs << ", "
         << _elDeg << ", " 
         << _azDeg << endl;
  }

  // send params every 90 rays
  
  if ((_rayCount % 90) == 0) {
    _writeRadarAndFieldParams();
  }
  _rayCount++;

  // compute range correction as required

  _computeRangeCorrTable();

  // compute the moments

  const XpolComms::XpolConf &conf = _xpol.getConf();
  if (_xpol.getServerMode() == XpolComms::SERVER_MODE_PP) {
    if (conf.sumPowers) {
      _computeMomentsPpSumPowers();
    } else {
      _computeMomentsPp();
    }
  } else if (_xpol.getServerMode() == XpolComms::SERVER_MODE_DUAL_PP) {
    if (conf.sumPowers) {
      _computeMomentsStagPpSumPowers();
    } else {
      _computeMomentsStagPp();
    }
  }
  
  // write ray to queue
  
  _writeRay();

  return 0;
  
}

////////////////////////////////////////
// write out the radar and field params


int ProSensing2Dsr::_writeRadarAndFieldParams()
{

  // load up radar params and field params message
  
  const XpolComms::XpolConf &conf = _xpol.getConf();
  const XpolComms::XpolStatus &status = _xpol.getStatus();
  const XpolComms::DataResponse &response = _xpol.getDataResponse();

  _gateSpacing = conf.rangeResMPerGate / 1000.0; // km
  _startRange = _gateSpacing / 2.0;

  // radar params

  DsRadarMsg msg;
  DsRadarParams &rParams = msg.getRadarParams();
  
  rParams.radarId = 0;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numGates = _xpol.getNGates();
  _samplesPerBeam = conf.nGroupPulses * conf.totAveragingInterval;
  rParams.samplesPerBeam = _samplesPerBeam;
  rParams.scanType = _scanType;
  switch(status.scanMode) {
    case XpolComms::SCAN_MODE_VOLUME:
    case XpolComms::SCAN_MODE_SLEW:
      rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
      rParams.scanTypeName = "az_surveillance";
      break;
    case XpolComms::SCAN_MODE_RHI:
    case XpolComms::SCAN_MODE_EL_RASTER:
      rParams.scanMode = DS_RADAR_RHI_MODE;
      rParams.scanTypeName = "rhi";
      break;
    case XpolComms::SCAN_MODE_PPI:
    case XpolComms::SCAN_MODE_AZ_RASTER:
      rParams.scanMode = DS_RADAR_SECTOR_MODE;
      rParams.scanTypeName = "az_sector";
      break;
    case XpolComms::SCAN_MODE_POINT:
      rParams.scanMode = DS_RADAR_POINTING_MODE;
      rParams.scanTypeName = "pointing";
      break;
    case XpolComms::SCAN_MODE_PAUSED:
      rParams.scanMode = DS_RADAR_IDLE_MODE;
      rParams.scanTypeName = "idle";
      break;
    default:
      rParams.scanMode = DS_RADAR_UNKNOWN_MODE;
      rParams.scanTypeName = "unknown";
  }
  rParams.polarization = (si32) DS_POLARIZATION_DUAL_HV_SIM;
  rParams.radarConstant = _calib.getRadarConstH();

  if (_params.override_radar_location) {
    rParams.altitude = _params.radar_altitude_meters / 1000.0; // km
    rParams.latitude = _params.radar_latitude_deg;
    rParams.longitude = _params.radar_longitude_deg;
  } else {
    rParams.altitude = response.georef.altMeters / 1000.0; // km
    rParams.latitude = response.georef.latDeg;
    rParams.longitude = response.georef.lonDeg;
  }

  rParams.gateSpacing = _gateSpacing; // km
  rParams.startRange = _startRange;
  rParams.horizBeamWidth = _calib.getBeamWidthDegH();
  rParams.vertBeamWidth = _calib.getBeamWidthDegV();
  rParams.pulseWidth = _calib.getPulseWidthUs();
  rParams.prt = conf.priUsecUnit1 * 1.0e-6;
  rParams.prt2 = conf.priUsecUnit2 * 1.0e-6;
  _prf = 1.0 / rParams.prt;
  rParams.pulseRepFreq = _prf;
  rParams.wavelength = _calib.getWavelengthCm();
  rParams.xmitPeakPower = pow(10.0, _calib.getXmitPowerDbmH() / 10.0) * 1.0e-3;
  rParams.receiverMds = -9999.0;
  rParams.receiverGain = _calib.getReceiverGainDbHc();
  rParams.antennaGain = _calib.getAntGainDbH(); 
  rParams.systemGain = -9999.0;
  rParams.unambigVelocity =
    ((rParams.wavelength / 100.0) * rParams.pulseRepFreq) / 4.0;
  rParams.unambigRange = (3.0e8 / (2.0 *  rParams.pulseRepFreq)) / 1000.0;
  
  if (_params.override_radar_name) {
    rParams.radarName = _params.radar_name;
  } else {
    rParams.radarName = conf.siteInfo;
  }
  
  // field params
  
  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  fieldParams.push_back
    (new DsFieldParams("DBMHC", "dBm", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("DBMVC", "dBm", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("SNRHC", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("SNRVC", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("DBZ_NAA", "dBZ", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("DBZ", "dBZ", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("VEL", "m/s", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("WIDTH", "m/s", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("ZDRM", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("ZDR", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("PHIDP", "deg", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("PHIDP0", "deg", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("RHOHV", "", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("NCP", "", 1.0, 0.0, 4, -9999));

  fieldParams.push_back
    (new DsFieldParams("LAG1_HC_DB", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("LAG1_HC_PHASE", "deg", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("LAG1_VC_DB", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("LAG1_VC_PHASE", "deg", 1.0, 0.0, 4, -9999));

  fieldParams.push_back
    (new DsFieldParams("RVVHH0_DB", "dB", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("RVVHH0_PHASE", "deg", 1.0, 0.0, 4, -9999));

  fieldParams.push_back
    (new DsFieldParams("VEL_DIFF", "m/s", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("VEL_PRT_SHORT", "m/s", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("VEL_PRT_LONG", "m/s", 1.0, 0.0, 4, -9999));
  fieldParams.push_back
    (new DsFieldParams("VEL_UNFOLD_INTERVAL", "", 1.0, 0.0, 4, -9999));

  _nFields = (int) fieldParams.size();
  rParams.numFields = _nFields;

  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DS_DATA_TYPE_RADAR_PARAMS | DS_DATA_TYPE_RADAR_FIELD_PARAMS)) {
    cerr << "ERROR - ProSensing2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  if (_writeCalib()) {
    return -1;
  }

  // prepare atmos atten

  if (!_attenReady) {
    _atten.setAttenCrpl(_calib.getWavelengthCm());
  }

  return 0;

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int ProSensing2Dsr::_writeCalib()

{

  // create DsRadar message
  
  DsRadarMsg msg;

  // set calib in message to the beam calibration
  
  DsRadarCalib &calib = msg.getRadarCalib();
  calib = _calib;

  // write the message
  
  if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - ProSensing2Dsr::_writeCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    string xml;
    _calib.convert2Xml(xml);
    cerr << "Writing out calibration:" << endl;
    cerr << xml;
  }
  
  return 0;

}

////////////////////////////////////////
// write out a ray

int ProSensing2Dsr::_writeRay()
  
{

#ifdef NOTYET  
  if(hdr.end_of_vol_flag) {
    _rQueue.putEndOfVolume(_volNum, rayTime.utime());
    _volNum++;
    _tiltNum = 0;
    _rQueue.putStartOfVolume(_volNum, rayTime.utime());
  }
  
  if(hdr.end_of_tilt_flag) {
    _rQueue.putEndOfTilt(_tiltNum, rayTime.utime());
    _tiltNum++;
    _rQueue.putStartOfTilt(_tiltNum, rayTime.utime());
  }

#endif

  // prepare data array, gate-by-gate
  
  TaArray<fl32> outData_;
  fl32 *outData = outData_.alloc(_nGates * _nFields);
  for (int ii = 0, jj = 0; ii < _nGates; ii++) {
    outData[jj++] = _moments[ii].dbmhc;
    outData[jj++] = _moments[ii].dbmvc;
    outData[jj++] = _moments[ii].snrhc;
    outData[jj++] = _moments[ii].snrvc;
    outData[jj++] = _moments[ii].dbz_no_atmos_atten;
    outData[jj++] = _moments[ii].dbz;
    outData[jj++] = _moments[ii].vel;
    outData[jj++] = _moments[ii].width;
    outData[jj++] = _moments[ii].zdrm;
    outData[jj++] = _moments[ii].zdr;
    outData[jj++] = _moments[ii].phidp;
    outData[jj++] = _moments[ii].phidp0;
    outData[jj++] = _moments[ii].rhohv;
    outData[jj++] = _moments[ii].ncp;
    outData[jj++] = _moments[ii].lag1_hc_db;
    outData[jj++] = _moments[ii].lag1_hc_phase;
    outData[jj++] = _moments[ii].lag1_vc_db;
    outData[jj++] = _moments[ii].lag1_vc_phase;
    outData[jj++] = _moments[ii].rvvhh0_db;
    outData[jj++] = _moments[ii].rvvhh0_phase;
    outData[jj++] = _moments[ii].vel_diff;
    outData[jj++] = _moments[ii].vel_prt_short;
    outData[jj++] = _moments[ii].vel_prt_long;
    outData[jj++] = _moments[ii].vel_unfold_interval;
  }
  int nBytesOut = _nGates * _nFields * sizeof(fl32);

  DsRadarMsg msg;
  DsRadarBeam &radarBeam = msg.getRadarBeam();
  radarBeam.loadData(outData, nBytesOut, sizeof(fl32));
  radarBeam.dataTime = _rayTimeSecs;
  radarBeam.nanoSecs = _rayNanoSecs;
  radarBeam.azimuth = _xpol.getRayAzDeg();
  radarBeam.elevation = _xpol.getRayElDeg();
  radarBeam.targetElev = -99.0;
  radarBeam.tiltNum = -1;
  radarBeam.volumeNum = -1;
  
  // send the params
  
  if (_rQueue.putDsBeam
      (msg, DS_DATA_TYPE_RADAR_BEAM_DATA)) {
    cerr << "ERROR - ProSensing2Dsr::_writeBeam" << endl;
    cerr << "  Cannot put beam message to FMQ" << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////
// compute range correction table

void ProSensing2Dsr::_computeRangeCorrTable()

{

  // check if we need a new table
  
  if (_rangeCorrection != NULL &&
      _nGates == _rCorrNGates &&
      _startRange == _rCorrStartRange &&
      _gateSpacing == _rCorrGateSpacing) {
    return;
  }

  // allocate table as required

  if (_rangeCorrection == NULL) {
    _rangeCorrection = new double[_nGates];
    _range = new double[_nGates];
  } else {
    if (_nGates != _rCorrNGates) {
      delete[] _rangeCorrection;
      _rangeCorrection = new double[_nGates];
      delete[] _range;
      _range = new double[_nGates];
    }
  }

  // compute table

  for (int i = 0; i < _nGates; i++) {
    _range[i] = _startRange + i * _gateSpacing;
    double rangeMeters = _range[i] * 1000.0;
    double log10Range = log10(rangeMeters) - 3.0;
    _rangeCorrection[i] = 20.0 * log10Range;
  }
  
  // save state

  _rCorrNGates = _nGates;
  _rCorrStartRange = _startRange;
  _rCorrGateSpacing = _gateSpacing;
  
}
				      
//////////////////////////////////////////////////
// compute the moments - PP mode, all gates

void ProSensing2Dsr::_computeMomentsPp()
  
{

  const XpolComms::XpolConf &conf = _xpol.getConf();

  // alloc space

  _moments.reserve(_nGates);

  // nyquist

  _prt = conf.priUsecUnit1 * 1.0e-6;
  _nyquist = (_wavelengthM / _prt) / 4.0;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  PP, _nyquist: " << _nyquist << endl;
  }

  // covariances

  const double *lag0_V0 = _xpol.getRecvPwrV0();
  const double *lag0_V1 = _xpol.getRecvPwrV1();

  const double *lag0_H0 = _xpol.getRecvPwrH0();
  const double *lag0_H1 = _xpol.getRecvPwrH1();
  
  const RadarComplex_t *lag1_V0V1 = _xpol.getDopPpV0V1();
  const RadarComplex_t *lag1_H0H1 = _xpol.getDopPpH0H1();
  const RadarComplex_t *rvvhh0 = _xpol.getCrossCorrVH();

  // loop through gates

  double range = _startRange;
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacing) {
    
    MomentsFields &mom = _moments[igate];
    double lag0_hc0 = lag0_H0[igate];
    double lag0_hc1 = lag0_H1[igate];
    double lag0_hc = (lag0_hc0 + lag0_hc1) / 2.0;

    double lag0_vc0 = lag0_V0[igate];
    double lag0_vc1 = lag0_V1[igate];
    double lag0_vc = (lag0_vc0 + lag0_vc1) / 2.0;

    const RadarComplex_t Rvvhh0 = rvvhh0[igate];
    RadarComplex_t lag1_vc = lag1_V0V1[igate];
    RadarComplex_t lag1_hc = lag1_H0H1[igate];
    
    _computeMomentsPp(igate, range,
                      lag0_hc, lag0_vc,
                      lag1_hc, lag1_vc,
                      Rvvhh0, mom);
    
  } // igate
  
}

////////////////////////////////////////////////////////
// compute the moments - PP mode, sum powers, all gates

void ProSensing2Dsr::_computeMomentsPpSumPowers()
  
{

  const XpolComms::XpolConf &conf = _xpol.getConf();

  // alloc space

  _moments.reserve(_nGates);

  // nyquist

  _prt = conf.priUsecUnit1 * 1.0e-6;
  _nyquist = (_wavelengthM / _prt) / 4.0;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  PP sum powers, _nyquist: " << _nyquist << endl;
  }

  // covariances

  const double *lag0_V = _xpol.getTotPwrV();
  const double *lag0_H = _xpol.getTotPwrH();

  const RadarComplex_t *lag1_V0V1 = _xpol.getDopPpV0V1();
  const RadarComplex_t *lag1_H0H1 = _xpol.getDopPpH0H1();
  const RadarComplex_t *rvvhh0 = _xpol.getCrossCorrVH();

  // loop through gates

  double range = _startRange;
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacing) {
    
    MomentsFields &mom = _moments[igate];
    double lag0_hc = lag0_H[igate];
    double lag0_vc = lag0_V[igate];
    
    const RadarComplex_t Rvvhh0 = rvvhh0[igate];
    RadarComplex_t lag1_vc = lag1_V0V1[igate];
    RadarComplex_t lag1_hc = lag1_H0H1[igate];
    
    _computeMomentsPp(igate, range,
                      lag0_hc, lag0_vc,
                      lag1_hc, lag1_vc,
                      Rvvhh0, mom);
    
  } // igate
  
}

//////////////////////////////////////////////////
// compute the moments - PP - single gate

void ProSensing2Dsr::_computeMomentsPp(int gateNum,
                                       double range,
                                       double lag0_hc,
                                       double lag0_vc,
                                       const RadarComplex_t &lag1_hc,
                                       const RadarComplex_t &lag1_vc,
                                       const RadarComplex_t &Rvvhh0,
                                       MomentsFields &mom)
  
  
{

  mom.init();

  // powers

  double lag0_hc_ns = lag0_hc - _calNoisePowerHc;
  double lag0_vc_ns = lag0_vc - _calNoisePowerVc;
  
  mom.dbmhc = 10.0 * log10(lag0_hc) - _rxGainDbHc;
  mom.dbmvc = 10.0 * log10(lag0_vc) - _rxGainDbVc;
  mom.dbm = (mom.dbmhc + mom.dbmvc) / 2.0;
  mom.dbm_for_noise = mom.dbm;
  
  // check snr OK
  
  bool snrHcOk = false;
  if (lag0_hc_ns > 0) {
    snrHcOk = true;
  }
  bool snrVcOk = false;
  if (lag0_vc_ns > 0) {
    snrVcOk = true;
  }
  
  // snr
  
  if (snrHcOk) {
    mom.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _rxGainDbHc;
    double snr_hc = lag0_hc_ns / _calNoisePowerHc;
    mom.snrhc = 10.0 * log10(snr_hc);
  }
  
  if (snrVcOk) {
    mom.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _rxGainDbVc;
    double snr_vc = lag0_vc_ns / _calNoisePowerVc;
    mom.snrvc = 10.0 * log10(snr_vc);
  }
  
  if (snrHcOk && snrVcOk) {
    mom.snr = (mom.snrhc + mom.snrvc) / 2.0;
  } else if (snrHcOk) {
    mom.snr = mom.snrhc;
  } else if (snrVcOk) {
    mom.snr = mom.snrvc;
  }
  
  // dbz
  
  if (snrHcOk) {
    mom.dbz_no_atmos_atten =
      mom.snrhc + _dbz0 + _rangeCorrection[gateNum];
    mom.dbz =
      mom.dbz_no_atmos_atten + _atten.getAtten(_elDeg, range);
  }

  // zdr

  if (snrHcOk && snrVcOk) {
    mom.zdrm = 10.0 * log10(lag0_hc_ns / lag0_vc_ns);
    mom.zdr = mom.zdrm + _zdrCorrectionDb;
  }
  
  // ncp
  
  RadarComplex_t lag1_sum = RadarComplex::complexSum(lag1_hc, lag1_vc);
  double ncp = RadarComplex::mag(lag1_sum) / (lag0_hc + lag0_vc);
  ncp = CONSTRAIN(ncp, 0.0, 1.0);
  mom.ncp = ncp;
  
  ////////////////////////////
  // phidp, rhohv
  
  RadarComplex_t phidp =
    RadarComplex::conjugateProduct(Rvvhh0, _phidpOffset);
  double phidpRad = RadarComplex::argRad(phidp);
  mom.phidp = phidpRad * RAD_TO_DEG;
  
  double phidpRad0 = RadarComplex::argRad(Rvvhh0);
  mom.phidp0 = phidpRad0 * RAD_TO_DEG;
  
  double Rvvhh0Mag = RadarComplex::mag(Rvvhh0);
  if (snrHcOk && snrVcOk) {
    double rhohv = Rvvhh0Mag / sqrt(lag0_hc_ns * lag0_vc_ns);
    rhohv = CONSTRAIN(rhohv, 0.0, 1.0);
    mom.rhohv = rhohv;
  }
  
  mom.rvvhh0_db = 20.0 * log10(Rvvhh0Mag);
  mom.rvvhh0_phase = RadarComplex::argDeg(Rvvhh0);
  
  /////////////////////
  // Doppler velocity
  
  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double arg_vel_hc = RadarComplex::argRad(lag1_hc);
  
  double lag1_vc_mag = RadarComplex::mag(lag1_vc);
  double arg_vel_vc = RadarComplex::argRad(lag1_vc);
  
  mom.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
  mom.lag1_hc_phase = arg_vel_hc * RAD_TO_DEG;
  
  mom.lag1_vc_db = 20.0 * log10(lag1_vc_mag);
  mom.lag1_vc_phase = arg_vel_vc * RAD_TO_DEG;
  
  double arg_vel = RadarComplex::argRad(lag1_sum);
  
  double vel = (arg_vel / M_PI) * _nyquist;
  mom.vel = vel * _velSign * -1.0;
  
  mom.phase_for_noise = lag1_sum;
  
  // spectrum width - use R0/R1
  
  double R0_hc = lag0_hc_ns;
  double R1_hc = RadarComplex::mag(lag1_hc);
  double width_R0R1_hc = _computeWidth(R0_hc, R1_hc, 0, 1, 1.0);
  
  double R0_vc = lag0_vc_ns;
  double R1_vc = RadarComplex::mag(lag1_vc);
  double width_R0R1_vc = _computeWidth(R0_vc, R1_vc, 0, 1, 1.0);
  
  double R0R1 = (width_R0R1_hc + width_R0R1_vc) / 2.0;
  mom.width = R0R1 * _nyquist;

}

//////////////////////////////////////////////////////
// compute the moments - staggered PP mode, all gates

void ProSensing2Dsr::_computeMomentsStagPp()
  
{

  // alloc space

  _moments.reserve(_nGates);

  // initialize staggerd PRT comps

  _initStaggered();

  // covariances

  const double *lag0_V0 = _xpol.getRecvPwrV0();
  const double *lag0_V1 = _xpol.getRecvPwrV1();
  // const double *lag0_V2 = _xpol.getRecvPwrV2();

  const double *lag0_H0 = _xpol.getRecvPwrH0();
  const double *lag0_H1 = _xpol.getRecvPwrH1();
  // const double *lag0_H2 = _xpol.getRecvPwrH2();
  
  const RadarComplex_t *lag1_V0V1 = _xpol.getDopPpV0V1();
  const RadarComplex_t *lag1_V1V2 = _xpol.getDopPpV1V2();
  const RadarComplex_t *lag1_H0H1 = _xpol.getDopPpH0H1();
  const RadarComplex_t *lag1_H1H2 = _xpol.getDopPpH1H2();
  const RadarComplex_t *rvvhh0 = _xpol.getCrossCorrVH();

  // loop through gates

  double range = _startRange;
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacing) {
    
    MomentsFields &mom = _moments[igate];
    double lag0_hc_short = lag0_H0[igate];
    double lag0_hc_long = lag0_H1[igate];

    double lag0_vc_short = lag0_V0[igate];
    double lag0_vc_long = lag0_V1[igate];
    
    RadarComplex_t lag1_hc_short = lag1_H0H1[igate];
    RadarComplex_t lag1_hc_long = lag1_H1H2[igate];
    
    RadarComplex_t lag1_vc_short = lag1_V0V1[igate];
    RadarComplex_t lag1_vc_long = lag1_V1V2[igate];
    
    const RadarComplex_t Rvvhh0 = rvvhh0[igate];
    
    _computeMomentsStagPp(igate, range,
                          lag0_hc_short, lag0_hc_long,
                          lag0_vc_short, lag0_vc_long,
                          lag1_hc_short, lag1_hc_long,
                          lag1_vc_short, lag1_vc_long,
                          Rvvhh0, mom);
    
  } // igate
  
}

//////////////////////////////////////////////////
// compute the moments - staggered PP - single gate

void ProSensing2Dsr::_computeMomentsStagPp(int gateNum,
                                           double range,
                                           double lag0_hc_short,
                                           double lag0_hc_long,
                                           double lag0_vc_short,
                                           double lag0_vc_long,
                                           const RadarComplex_t &lag1_hc_short,
                                           const RadarComplex_t &lag1_hc_long,
                                           const RadarComplex_t &lag1_vc_short,
                                           const RadarComplex_t &lag1_vc_long,
                                           const RadarComplex_t &Rvvhh0,
                                           MomentsFields &mom)

{

  mom.init();

  // power-related fields
  
  double lag0_hc_ns = lag0_hc_long - _calNoisePowerHc;
  double lag0_vc_ns = lag0_vc_long - _calNoisePowerVc;
  
  mom.dbmhc = 10.0 * log10(lag0_hc_long) - _rxGainDbHc;
  mom.dbmvc = 10.0 * log10(lag0_vc_long) - _rxGainDbVc;
  mom.dbm = (mom.dbmhc + mom.dbmvc) / 2.0;
  mom.dbm_for_noise = mom.dbm;
  
  // check SNR OK
  
  bool snrHcOk = false;
  if (lag0_hc_ns > 0) {
    snrHcOk = true;
  }
  bool snrVcOk = false;
  if (lag0_vc_ns > 0) {
    snrVcOk = true;
  }
  
  // compute snr
  
  if (snrHcOk) {
    mom.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _rxGainDbHc;
    double snr_hc = lag0_hc_ns / _calNoisePowerHc;
    mom.snrhc = 10.0 * log10(snr_hc);
  }
  if (snrVcOk) {
    mom.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _rxGainDbVc;
    double snr_vc = lag0_vc_ns / _calNoisePowerVc;
    mom.snrvc = 10.0 * log10(snr_vc);
  }
  
  if (snrHcOk && snrVcOk) {
    mom.snr = (mom.snrhc + mom.snrvc) / 2.0;
  } else if (snrHcOk) {
    mom.snr = mom.snrhc;
  } else if (snrVcOk) {
    mom.snr = mom.snrvc;
  }
  
  // dbz

  if (snrHcOk) {
    mom.dbz_no_atmos_atten =
      mom.snrhc + _dbz0 + _rangeCorrection[gateNum];
    mom.dbz =
      mom.dbz_no_atmos_atten + _atten.getAtten(_elDeg, range);
  }
    
  // zdr

  if (snrHcOk && snrVcOk) {
    mom.zdrm = 10.0 * log10(lag0_hc_ns / lag0_vc_ns);
    mom.zdr = mom.zdrm + _zdrCorrectionDb;
  }

  // ncp
  
  RadarComplex_t lag1_sum = RadarComplex::complexSum(lag1_hc_long,
                                                     lag1_vc_long);
  double ncp = RadarComplex::mag(lag1_sum) / (lag0_hc_long + lag0_vc_long);
  ncp = CONSTRAIN(ncp, 0.0, 1.0);
  mom.ncp = ncp;
  
  ////////////////////////////
  // phidp, rhohv
  
  RadarComplex_t phidp =
    RadarComplex::conjugateProduct(Rvvhh0, _phidpOffset);
  double phidpRad = RadarComplex::argRad(phidp);
  mom.phidp = phidpRad * RAD_TO_DEG;
  
  double phidpRad0 = RadarComplex::argRad(Rvvhh0);
  mom.phidp0 = phidpRad0 * RAD_TO_DEG;
  
  double Rvvhh0Mag = RadarComplex::mag(Rvvhh0);
  if (snrHcOk && snrVcOk) {
    double rhohv = Rvvhh0Mag / sqrt(lag0_hc_ns * lag0_vc_ns);
    rhohv = CONSTRAIN(rhohv, 0.0, 1.0);
    mom.rhohv = rhohv;
  }
  
  mom.rvvhh0_db = 20.0 * log10(Rvvhh0Mag);
  mom.rvvhh0_phase = RadarComplex::argDeg(Rvvhh0);

  //////////////////////////////////
  // velocity

  // compute velocity short PRT data
  
  RadarComplex_t lag1_sum_short =
    RadarComplex::complexSum(lag1_hc_short, lag1_vc_short);
  double argVelShort = RadarComplex::argRad(lag1_sum_short);
  double velShort = (argVelShort / M_PI) * _nyquistPrtShort;
  mom.vel_prt_short = velShort * _velSign * -1.0;

  mom.phase_for_noise = lag1_sum_short;

  // compute velocity long PRT data
  
  RadarComplex_t lag1_sum_long =
    RadarComplex::complexSum(lag1_hc_long, lag1_vc_long);
  double argVelLong = RadarComplex::argRad(lag1_sum_long);
  double velLong = (argVelLong / M_PI) * _nyquistPrtLong;
  mom.vel_prt_long = velLong * _velSign * -1.0;

  // compute the unfolded velocity

  double vel_diff = mom.vel_prt_short - mom.vel_prt_long;
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;
  double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
  int ll = (int) floor(nyquistIntervalShort + 0.5);
  if (ll < -_LL) {
    ll = -_LL;
  } else if (ll > _LL) {
    ll = _LL;
  }
  double unfoldedVel = mom.vel_prt_short + _PP[ll] * _nyquistPrtShort * 2;
  mom.vel = unfoldedVel;
  mom.vel_unfold_interval = _PP[ll];
  mom.vel_diff = vel_diff;

  ////////////////////////////////////////////////////////////////////////////////
  // use a hybrid estimator for spectrum width

  // spectrum width using R(0)/R(m)
  
  double R0_hc = lag0_hc_long;
  double Rm_hc = RadarComplex::mag(lag1_hc_long);
  double width_R0Rm_hc = _computeWidth(R0_hc, Rm_hc, 0, _staggeredM, 1.0);
  
  double R0_vc = lag0_vc_long;
  double Rm_vc = RadarComplex::mag(lag1_vc_long);
  double width_R0Rm_vc = _computeWidth(R0_vc, Rm_vc, 0, _staggeredM, 1.0);

  double width_R0Rm = (width_R0Rm_hc + width_R0Rm_vc) / 2.0;
  
  mom.width = width_R0Rm * _nyquist;

}

///////////////////////////////////////////////////////////////
// compute the moments - staggered PP sum powers mode, all gates

void ProSensing2Dsr::_computeMomentsStagPpSumPowers()
{

  // alloc space

  _moments.reserve(_nGates);

  // initialize staggerd PRT comps

  _initStaggered();

  // covariances

  const double *lag0_V = _xpol.getTotPwrV();
  const double *lag0_H = _xpol.getTotPwrH();
  
  const RadarComplex_t *lag1_V0V1 = _xpol.getDopPpV0V1();
  const RadarComplex_t *lag1_V1V2 = _xpol.getDopPpV1V2();
  const RadarComplex_t *lag1_H0H1 = _xpol.getDopPpH0H1();
  const RadarComplex_t *lag1_H1H2 = _xpol.getDopPpH1H2();
  const RadarComplex_t *rvvhh0 = _xpol.getCrossCorrVH();
  
  // loop through gates

  double range = _startRange;
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacing) {
    
    MomentsFields &mom = _moments[igate];

    double lag0_hc = lag0_H[igate];
    double lag0_vc = lag0_V[igate];

    RadarComplex_t lag1_hc_short = lag1_H0H1[igate];
    RadarComplex_t lag1_hc_long = lag1_H1H2[igate];
    
    RadarComplex_t lag1_vc_short = lag1_V0V1[igate];
    RadarComplex_t lag1_vc_long = lag1_V1V2[igate];
    
    const RadarComplex_t Rvvhh0 = rvvhh0[igate];
    
    _computeMomentsStagPp(igate, range,
                          lag0_hc, lag0_hc,
                          lag0_vc, lag0_vc,
                          lag1_hc_short, lag1_hc_long,
                          lag1_vc_short, lag1_vc_long,
                          Rvvhh0, mom);
    
  } // igate
  
}

/////////////////////////////////////////////////
// compute width using R0R1 method
// from Greg Meymaris

double ProSensing2Dsr::_computeR0R1Width(double r0,
                                         double r1,
                                         double nyquist)
{ 
  
  double r0r1 = 0;
  if (r0 > r1) {
    r0r1 = sqrt(log(r0 / r1) * 2.0) / M_PI;
  }

  return r0r1 * nyquist;
}

/////////////////////////////////////////////////
// compute width from lags a and b
// using an rA/rB estimator

double ProSensing2Dsr::_computeWidth(double rA,
                                     double rB,
                                     int lagA,
                                     int lagB,
                                     double nyquist)

{
  
  double factor =
    nyquist / (M_PI * sqrt((lagB * lagB - lagA * lagA) / 2.0));
  
  double rArB = 0;
  if (rA > rB) {
    rArB = sqrt(log(rA / rB));
  }
  double width = rArB * factor;

  return width;

}

//////////////////////////////////////////////////////
// initialize staggered PRT computations

void ProSensing2Dsr::_initStaggered()
  
{

  const XpolComms::XpolConf &conf = _xpol.getConf();
  
  // nyquist

  _prtShort = conf.priUsecUnit1 * 1.0e-6;
  _prtLong = conf.priUsecUnit2 * 1.0e-6;

  double prtRatio = _prtShort / _prtLong;
  int ratio60 = (int) (prtRatio * 60.0 + 0.5);
  if (ratio60 == 40) {
    // 2/3
    _staggeredM = 2;
    _staggeredN = 3;
  } else if (ratio60 == 45) {
    // 3/4
    _staggeredM = 3;
    _staggeredN = 4;
  } else if (ratio60 == 48) {
    // 4/5
    _staggeredM = 4;
    _staggeredN = 5;
  } else {
    // assume 2/3
    cerr << "ProSensing2Dsr::_computeMomentsStagPp" << endl;
    cerr << "  No support for prtRatio: " << prtRatio << endl;
    cerr << "  Assuming 2/3 stagger" << endl;
    _staggeredM = 2;
    _staggeredN = 3;
  }

  _nyquistPrtShort = ((_wavelengthM / _prtShort) / 4.0);
  _nyquistPrtLong = ((_wavelengthM / _prtLong) / 4.0);
  _nyquist = _nyquistPrtShort * _staggeredM;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  PP stag, _nyquist: " << _nyquist << endl;
    cerr << "  _nyquistPrtShort: " << _nyquistPrtShort << endl;
    cerr << "  _nyquistPrtLong: " << _nyquistPrtLong << endl;
    cerr << "===>> staggered PRT, ratio: "
         << _staggeredM << "/" << _staggeredN << " <<===" << endl;
    cerr << "  _staggeredM: " << _staggeredM << endl;
    cerr << "  _staggeredN: " << _staggeredN << endl;
  }

  // compute the P lookup table for dealiasing
  // See Torres et al, JTech, Sept 2004

  _LL = (_staggeredM + _staggeredN - 1) / 2;
  if (_LL > 5) {
    _LL = 2; // set to 2/3
  }
  _PP = _PP_ + _LL;

  int cc = 0;
  int pp = 0;
  _PP[0] = 0;
  for (int ll = 1; ll <= _LL; ll++) {
    if ((ll / 2 * 2) == ll) {
      // even - va1 transition
      cc -= _staggeredN;
      pp++;
    } else {
      // odd - va2 transition
      cc += _staggeredM;
    }
    _PP[cc] = pp;
    _PP[-cc] = -pp;
  }

}
