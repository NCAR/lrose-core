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
// OutputFmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// OutputFmq puts the data out at FMQ.
//
////////////////////////////////////////////////////////////////

#define _in_OutputFmq_cc
#include "OutputFmq.hh"
#undef _in_OutputFmq_cc

#include "Calibration.hh"

#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfMoments.hh>
#include <radar/MomentsFields.hh>
#include <radar/RadarCalib.hh>
#include <cmath>
#include <toolsa/uusleep.h>
#include <dsserver/DmapAccess.hh>
#include <toolsa/DateTime.hh>
#include <Radx/RadxMsg.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxStatusXml.hh>
#include <Radx/RadxEvent.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
using namespace std;

// Constructor

OutputFmq::OutputFmq(const string &prog_name,
		     const Params &params) :
  _progName(prog_name),
  _params(params)
 
{
  
  constructorOK = TRUE;
  _nFields = 0;
  _nBeamsWritten = 0;
  _dsrQueue = NULL;
  _radxQueue = NULL;

  if (_params.output_moments_in_radx_format) {
    _useRadx = true;
  } else {
    _useRadx = false;
  }

  // initialize the output queue

  if (_openFmq()) {
    constructorOK = FALSE;
    return;
  }

  // initialize the busy mutex

  pthread_mutex_init(&_busy, NULL);

}

// destructor

OutputFmq::~OutputFmq()
  
{

  if (_dsrQueue != NULL) {
    delete _dsrQueue;
  }

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(const Beam &beam)

{
  int iret = 0;
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    iret = _writePlatformRadx(beam);
  } else {
    iret = _writeParamsDsRadar(beam);
  }
  pthread_mutex_unlock(&_busy);
  return iret;
}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeCalib(const Beam &beam)

{
  int iret = 0;
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    iret = _writeCalibRadx(beam);
  } else {
    iret = _writeCalibDsRadar(beam);
  }
  pthread_mutex_unlock(&_busy);
  return iret;
}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeStatusXml(const Beam &beam)

{
  int iret = 0;
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    iret = _writeStatusXmlRadx(beam);
  } else {
    iret = _writeStatusXmlDsRadar(beam);
  }
  pthread_mutex_unlock(&_busy);
  return iret;
}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeBeam(const Beam &beam)

{
  int iret = 0;
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    iret = _writeBeamRadx(beam);
  } else {
    iret = _writeBeamDsRadar(beam);
  }
  pthread_mutex_unlock(&_busy);
  return iret;
}

////////////////////////////////////////
// put volume flags

void OutputFmq::putStartOfVolume(int volNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    _putStartOfVolumeRadx(volNum, beam);
  } else {
    _putStartOfVolumeDsRadar(volNum, beam);
  }
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfVolume(int volNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    _putEndOfVolumeRadx(volNum, beam);
  } else {
    _putEndOfVolumeDsRadar(volNum, beam);
  }
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putStartOfTilt(int tiltNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    _putStartOfTiltRadx(tiltNum, beam);
  } else {
    _putStartOfTiltDsRadar(tiltNum, beam);
  }
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfTilt(int tiltNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    _putEndOfTiltRadx(tiltNum, beam);
  } else {
    _putEndOfTiltDsRadar(tiltNum, beam);
  }
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putNewScanType(int scanType, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  if (_useRadx) {
    _putNewScanTypeRadx(scanType, beam);
  } else {
    _putNewScanTypeDsRadar(scanType, beam);
  }
  pthread_mutex_unlock(&_busy);
}

///////////////////////////////////////////////////////////
// Writing in DsRadar format
///////////////////////////////////////////////////////////

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::_writeParamsDsRadar(const Beam &beam)

{

  // initialize 

  const IwrfTsInfo &opsInfo = beam.getOpsInfo();
  const IwrfCalib &calib = beam.getCalib();
  iwrf_xmit_rcv_mode_t xmitRcvMode = beam.getXmitRcvMode();
                           
  int nGatesOut = beam.getNGatesOut();
  // int nSamples = beam.getNSamplesEffective();
  int nSamples = beam.getNSamples();
  double pulseWidthUs = beam.getPulseWidth() * 1.0e6;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::_writeParamsDsRadar, nGates: " << nGatesOut << endl;
  }
  
  // compute nyquist and unambig range

  double nyquistVel = beam.getNyquist();
  double unambigRange = beam.getUnambigRangeKm();

  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &field = _params._output_fields[ii];

    if (field.write_unfiltered) {
      _addField(field.name, field.units, field.scale, field.bias, fp);
      _nFields++;
    }
    
    if (field.write_filtered) {
      string filteredName = field.name;
      filteredName += "_F";
      _addField(filteredName, field.units, field.scale, field.bias, fp);
      _nFields++;
    }
    
  } // ii
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = nGatesOut;
  rp.samplesPerBeam = nSamples;
  rp.scanType = opsInfo.get_scan_mode();
  rp.scanMode = beam.getScanMode();
  rp.followMode = beam.getFollowMode();

  switch(xmitRcvMode) {
    case IWRF_SINGLE_POL:
      rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
      break;
    case IWRF_SINGLE_POL_V:
      rp.polarization = DS_POLARIZATION_VERT_TYPE;
      break;
    case IWRF_ALT_HV_CO_ONLY:
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_HV_ALT;
      break;
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_HV_SIM;
      break;
    case IWRF_H_ONLY_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_H_XMIT;
      break;
    case IWRF_V_ONLY_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_V_XMIT;
      break;
    default: {
      rp.polarization = DS_POLARIZATION_DUAL_TYPE;
    }
  }

  if (beam.getIsStagPrt()) {
    if (beam.getStagM() == 2) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_2_3;
    } else if (beam.getStagM() == 3) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_3_4;
    } else if (beam.getStagM() == 4) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_4_5;
    }
  }
  
  rp.radarConstant = calib.getRadarConstH();

  rp.altitude = opsInfo.get_radar_altitude_m() / 1000.0;
  rp.latitude = opsInfo.get_radar_latitude_deg();
  rp.longitude = opsInfo.get_radar_longitude_deg();

  // override if georefs active

  if (beam.getGeorefActive()) {
    const iwrf_platform_georef_t &georef = beam.getGeoref();
    rp.latitude = georef.latitude;
    rp.longitude = georef.longitude;
    rp.altitude = georef.altitude_msl_km;
  }

  rp.gateSpacing = opsInfo.get_proc_gate_spacing_km();
  rp.startRange = opsInfo.get_proc_start_range_km();

  rp.horizBeamWidth = calib.getBeamWidthDegH();
  rp.vertBeamWidth = calib.getBeamWidthDegV();
  
  rp.pulseWidth = pulseWidthUs;
  rp.pulseRepFreq = 1.0 / beam.getPrt();
  rp.prt = beam.getPrt();
  rp.prt2 = beam.getPrtLong();
  rp.wavelength = opsInfo.get_radar_wavelength_cm();
  
  rp.xmitPeakPower = pow(10.0, calib.getXmitPowerDbmH() / 10.0);
  rp.receiverGain = calib.getReceiverGainDbHc();
  rp.receiverMds = calib.getNoiseDbmHc() - rp.receiverGain;
  rp.antennaGain = calib.getAntGainDbH();
  rp.systemGain =  rp.receiverGain + rp.antennaGain;

  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.measXmitPowerDbmH = beam.getMeasXmitPowerDbmH();
  rp.measXmitPowerDbmV = beam.getMeasXmitPowerDbmV();

  rp.radarName = opsInfo.get_radar_name();
  rp.scanTypeName = opsInfo.get_scan_segment_name();
  
  // write the message
  
  if (_dsrQueue->putDsMsg(msg,
                       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - OutputFmq::_writeParamsDsRadar" << endl;
    cerr << "  Cannot put params to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int OutputFmq::_writeCalibDsRadar(const Beam &beam)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::_writeCalibDsRadar" << endl;
  }
  
  // create DsRadar message
  
  DsRadarMsg msg;

  // set calib in message to the beam calibration
  
  DsRadarCalib &calib = msg.getRadarCalib();
  IwrfCalib icalib = beam.getCalib();
  RadarCalib::copyIwrfToDsRadar(icalib, calib);

  // write the message
  
  if (_dsrQueue->putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - OutputFmq::_writeCalibDsRadar" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    string xml;
    beam.getCalib().convert2Xml(xml);
    cerr << "Writing out calibration:" << endl;
    cerr << xml;
  }
  
  return 0;

}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int OutputFmq::_writeStatusXmlDsRadar(const Beam &beam)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::_writeStatusXmlDsRadar" << endl;
  }
  
  // create DsRadar message
  
  DsRadarMsg msg;
  msg.setStatusXml(beam.getStatusXml());
  
  // write the message
  
  if (_dsrQueue->putDsMsg(msg, DsRadarMsg::STATUS_XML)) {
    cerr << "ERROR - OutputFmq::_writeCalibDsRadar" << endl;
    cerr << "  Cannot put status XML to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Writing out status xml: " << endl;
    cerr << beam.getStatusXml() << endl;
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::_writeBeamDsRadar(const Beam &beam)

{

  bool printBeamDebug = (_params.debug >= Params::DEBUG_VERBOSE);
  if (_params.debug &&
      ((_nBeamsWritten % _params.beam_count_for_debug_print) == 0)) {
    printBeamDebug = true;
  }
  if (printBeamDebug) {
    DateTime beamTime(beam.getTimeSecs(), true, beam.getNanoSecs() / 1.0e9);
    time_t now = time(NULL);
    int lateSecs = now - beam.getTimeSecs();
    if (_params.mode == Params::ARCHIVE) {
      fprintf(stderr,
              "-->> OutputFmq::_writeBeamDsRadar %s - %s, vol: %.3d, "
              "sweep: %.3d, "
              "az: %6.3f, el %5.3f, nSamples: %3d\n",
              beamTime.asString(3).c_str(),
              iwrf_scan_mode_to_short_str(beam.getScanMode()).c_str(), 
              beam.getVolNum(), beam.getSweepNum(),
              beam.getAz(), beam.getEl(),
              beam.getNSamples());
    } else {
      fprintf(stderr,
              "-->> OutputFmq::_writeBeamDsRadar %s (late %d) - %s, vol: %.3d, "
              "sweep: %.3d, "
              "az: %6.3f, el %5.3f, nSamples: %3d\n",
              beamTime.asString(3).c_str(), lateSecs, 
              iwrf_scan_mode_to_short_str(beam.getScanMode()).c_str(), 
              beam.getVolNum(), beam.getSweepNum(),
              beam.getAz(), beam.getEl(),
              beam.getNSamples());
    }
  }

  // put georeference if applicable

  int contents = 0;
  if (beam.getGeorefActive()) {
    DsPlatformGeoref &georef = _msg.getPlatformGeoref();
    ds_iwrf_platform_georef_t dsGeoref;
    // copy struct from iwrf to DsRadar
    // these are identical, stored in different libs
    // TODO - bring DsRadar into libs/radar
    memcpy(&dsGeoref, &beam.getGeoref(), sizeof(dsGeoref));
    georef.setGeoref(dsGeoref);
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  int nGatesOut = beam.getNGatesOut();
  
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  int ndata = nGatesOut * _nFields;
  ui16 *data = new ui16[ndata];
  memset(data, 0, ndata * sizeof(ui16));
  
  // params
  
  dsBeam.dataTime = beam.getTimeSecs();
  double dtime = beam.getDoubleTime();
  double partialSecs = fmod(dtime, 1.0);
  dsBeam.nanoSecs = (int) (partialSecs * 1.0e9 + 0.5);
  dsBeam.volumeNum = beam.getVolNum();
  dsBeam.tiltNum = beam.getSweepNum();
  dsBeam.elevation = beam.getEl();
  dsBeam.azimuth = beam.getAz();
  dsBeam.targetElev = beam.getTargetEl();
  dsBeam.targetAz = beam.getTargetAz();
  dsBeam.antennaTransition = beam.getAntennaTransition();
  dsBeam.scanMode = beam.getScanMode();
  dsBeam.beamIsIndexed = beam.getBeamIsIndexed();
  dsBeam.angularResolution = beam.getAngularResolution();
  dsBeam.nSamples = beam.getNSamples();
  // dsBeam.nSamples = beam.getNSamplesEffective();
  dsBeam.measXmitPowerDbmH = beam.getMeasXmitPowerDbmH();
  dsBeam.measXmitPowerDbmV = beam.getMeasXmitPowerDbmV();

  // load up vector of field offsets

  vector<int> offsets;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    int offset = _findFieldOffset(field.id);
    offsets.push_back(offset);
  }

  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  const MomentsFields *fieldsArray = beam.getFields();
  const MomentsFields *fieldsFArray = beam.getFieldsF();
  ui16 *dp = data;
  for (int igate = 0; igate < nGatesOut; igate++) {

    const MomentsFields &fields = fieldsArray[igate];
    const MomentsFields &fieldsF = fieldsFArray[igate];

    const double *start = &fields.start;
    const double *startF = &fieldsF.start;
  
    for (int ii = 0; ii < _params.output_fields_n; ii++) {
      
      const Params::output_field_t &field = _params._output_fields[ii];
      int offset = offsets[ii];
      
      if (field.write_unfiltered) {
        double val = *(start + offset);
        *dp = _convertDouble(val, field.scale, field.bias);
        dp++;
      }
      
      if (field.write_filtered) {
        double val = *(startF + offset);
        *dp = _convertDouble(val, field.scale, field.bias);
        dp++;
      }
      
    } // ii

  } // igate

  // load the data into the message

  dsBeam.loadData(data, ndata * sizeof(ui16), sizeof(ui16));
  delete[] data;
  
  if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }
  
  // write the message
  
  contents |= DsRadarMsg::RADAR_BEAM;
  if (_dsrQueue->putDsMsg(_msg, contents)) {
    cerr << "ERROR - OutputFmq::_writeBeamsDsRadar" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  _nBeamsWritten++;
  return 0;

}

////////////////////////////////////////
// put volume flags

void OutputFmq::_putStartOfVolumeDsRadar(int volNum, const Beam &beam)
{
  _dsrQueue->putStartOfVolume(volNum, beam.getTimeSecs());
}

void OutputFmq::_putEndOfVolumeDsRadar(int volNum, const Beam &beam)
{
  _dsrQueue->putEndOfVolume(volNum, beam.getTimeSecs());
}

void OutputFmq::_putStartOfTiltDsRadar(int tiltNum, const Beam &beam)
{
  _dsrQueue->putStartOfTilt(tiltNum, beam.getTimeSecs());
}

void OutputFmq::_putEndOfTiltDsRadar(int tiltNum, const Beam &beam)
{
  _dsrQueue->putEndOfTilt(tiltNum, beam.getTimeSecs());
}

void OutputFmq::_putNewScanTypeDsRadar(int scanType, const Beam &beam)
{
  _dsrQueue->putNewScanType(scanType, beam.getTimeSecs());
}

///////////////////////////////////////////////////////////
// Writing in Radx format
///////////////////////////////////////////////////////////

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::_writePlatformRadx(const Beam &beam)

{

  // initialize 

  const IwrfTsInfo &opsInfo = beam.getOpsInfo();
  const IwrfCalib &calib = beam.getCalib();

  // Set radar parameters
  
  RadxPlatform platform;

  platform.setInstrumentName(opsInfo.get_radar_name());
  platform.setSiteName(opsInfo.get_radar_name());
  platform.setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);

  switch (_params.platform_type) {
    case Params::PLATFORM_VEHICLE:
      platform.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    case Params::PLATFORM_SHIP:
      platform.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    case Params::PLATFORM_AIRCRAFT:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT);
      break;
    case Params::PLATFORM_AIRCRAFT_FORE:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    case Params::PLATFORM_AIRCRAFT_AFT:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_AFT);
      break;
    case Params::PLATFORM_AIRCRAFT_TAIL:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    case Params::PLATFORM_AIRCRAFT_BELLY:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    case Params::PLATFORM_AIRCRAFT_ROOF:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    case Params::PLATFORM_AIRCRAFT_NOSE:
      platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_NOSE);
      break;
    case Params::PLATFORM_SATELLITE_ORBIT:
      platform.setPlatformType(Radx::PLATFORM_TYPE_SATELLITE_ORBIT);
      break;
    case Params::PLATFORM_SATELLITE_GEOSTAT:
      platform.setPlatformType(Radx::PLATFORM_TYPE_SATELLITE_GEOSTAT);
      break;
    case Params::PLATFORM_FIXED:
    default:
      platform.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  }

  switch (_params.primary_axis) {
    case Params::PRIMARY_AXIS_Y:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_Y);
      break;
    case Params::PRIMARY_AXIS_Z:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
      break;
    case Params::PRIMARY_AXIS_X_PRIME:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_X_PRIME);
      break;
    case Params::PRIMARY_AXIS_Y_PRIME:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
      break;
    case Params::PRIMARY_AXIS_Z_PRIME:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_Z_PRIME);
      break;
    case Params::PRIMARY_AXIS_X:
    default:
      platform.setPrimaryAxis(Radx::PRIMARY_AXIS_X);
  }
                             
  platform.setAltitudeKm(opsInfo.get_radar_altitude_m() / 1000.0);
  platform.setLatitudeDeg(opsInfo.get_radar_latitude_deg());
  platform.setLongitudeDeg(opsInfo.get_radar_longitude_deg());

  // override if georefs active

  if (beam.getGeorefActive()) {
    const iwrf_platform_georef_t &georef = beam.getGeoref();
    platform.setAltitudeKm(georef.altitude_msl_km);
    platform.setLatitudeDeg(georef.latitude);
    platform.setLongitudeDeg(georef.longitude);
  }

  platform.setRadarBeamWidthDegH(calib.getBeamWidthDegH());
  platform.setRadarBeamWidthDegV(calib.getBeamWidthDegV());
  
  platform.setRadarAntennaGainDbH(calib.getAntGainDbH());
  platform.setRadarAntennaGainDbV(calib.getAntGainDbV());

  platform.setWavelengthCm(opsInfo.get_radar_wavelength_cm());

  // create message
  
  RadxMsg msg;
  platform.serialize(msg);

  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_writePlatformRadx" << endl;
    cerr << "  Cannot write platform to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int OutputFmq::_writeCalibRadx(const Beam &beam)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::_writeCalibRadx" << endl;
  }

  // copy IwrfCalib to RadxCalib
  
  IwrfCalib icalib = beam.getCalib();
  RadxRcalib xcalib;
  RadarCalib::copyIwrfToRadx(icalib, xcalib);

  // create message
  
  RadxMsg msg;
  xcalib.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    string xml;
    icalib.convert2Xml(xml);
    cerr << "Writing out calibration:" << endl;
    cerr << xml;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_writeCalibRadx" << endl;
    cerr << "  Cannot write calib to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int OutputFmq::_writeStatusXmlRadx(const Beam &beam)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::_writeStatusXmlRadx" << endl;
  }

  // create RadxStatusXml object

  RadxStatusXml status;
  status.setXmlStr(beam.getStatusXml());
  
  // create message
  
  RadxMsg msg;
  status.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "=========== Writing out status xml =============" << endl;
    cerr << status.getXmlStr() << endl;
    cerr << "================================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_writeCalibRadx" << endl;
    cerr << "  Cannot write status xml to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::_writeBeamRadx(const Beam &beam)

{

  const IwrfTsInfo &opsInfo = beam.getOpsInfo();

  // create ray
  
  RadxRay ray;
  
  ray.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  ray.setVolumeNumber(beam.getVolNum());
  ray.setSweepNumber(beam.getSweepNum());
  ray.setScanName(opsInfo.get_scan_segment_name());
  
  int dsrScanMode = beam.getScanMode();
  ray.setSweepMode(IwrfMoments::getRadxSweepMode(dsrScanMode));

  iwrf_xmit_rcv_mode_t xmitRcvMode = beam.getXmitRcvMode();
  switch(xmitRcvMode) {
    case IWRF_SINGLE_POL:
      ray.setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
      break;
    case IWRF_SINGLE_POL_V:
      ray.setPolarizationMode(Radx::POL_MODE_VERTICAL);
      break;
    case IWRF_ALT_HV_CO_ONLY:
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
      ray.setPolarizationMode(Radx::POL_MODE_HV_ALT);
      break;
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      ray.setPolarizationMode(Radx::POL_MODE_HV_SIM);
      break;
    case IWRF_H_ONLY_FIXED_HV:
      ray.setPolarizationMode(Radx::POL_MODE_HV_H_XMIT);
      break;
    case IWRF_V_ONLY_FIXED_HV:
      ray.setPolarizationMode(Radx::POL_MODE_HV_V_XMIT);
      break;
    default: {
      ray.setPolarizationMode(Radx::POL_MODE_HV_SIM);
    }
  }

  if (beam.getIsStagPrt()) {
    ray.setPrtMode(Radx::PRT_MODE_STAGGERED);
    ray.setPrtRatio(beam.getPrtShort() / beam.getPrtLong());
  } else {
    ray.setPrtMode(Radx::PRT_MODE_FIXED);
    ray.setPrtRatio(1.0);
  }
  ray.setFollowMode(Radx::FOLLOW_MODE_NONE);

  double elev = beam.getEl();
  if (elev > 180) {
    elev -= 360.0;
  }
  ray.setElevationDeg(elev);

  double az = beam.getAz();
  if (az < 0) {
    az += 360.0;
  }
  ray.setAzimuthDeg(az);
  
  ray.setRangeGeom(beam.getStartRangeKm(), beam.getGateSpacingKm());

  if (dsrScanMode == DS_RADAR_RHI_MODE ||
      dsrScanMode == DS_RADAR_EL_SURV_MODE) {
    ray.setFixedAngleDeg(beam.getTargetAz());
  } else {
    ray.setFixedAngleDeg(beam.getTargetEl());
  }

  ray.setIsIndexed(beam.getBeamIsIndexed());
  ray.setAngleResDeg(beam.getAngularResolution());
  ray.setAntennaTransition(beam.getAntennaTransition());
  ray.setNSamples(beam.getNSamples());
  
  ray.setPulseWidthUsec(beam.getPulseWidth() * 1.0e6);
  ray.setPrtSec(beam.getPrt());
  ray.setNyquistMps(beam.getNyquist());

  ray.setUnambigRangeKm(beam.getUnambigRangeKm());

  ray.setMeasXmitPowerDbmH(beam.getMeasXmitPowerDbmH());
  ray.setMeasXmitPowerDbmV(beam.getMeasXmitPowerDbmV());

  // platform georeference - set if georefs active

  if (beam.getGeorefActive()) {
    const iwrf_platform_georef_t &dsGeoref = beam.getGeoref();
    RadxGeoref georef;
    georef.setTimeSecs(dsGeoref.packet.time_secs_utc);
    georef.setNanoSecs(dsGeoref.packet.time_nano_secs);
    georef.setUnitNum(dsGeoref.unit_num);
    georef.setUnitId(dsGeoref.unit_id);
    georef.setLongitude(dsGeoref.longitude);
    georef.setLatitude(dsGeoref.latitude);
    georef.setAltitudeKmMsl(dsGeoref.altitude_msl_km);
    georef.setAltitudeKmAgl(dsGeoref.altitude_agl_km);
    georef.setEwVelocity(dsGeoref.ew_velocity_mps);
    georef.setNsVelocity(dsGeoref.ns_velocity_mps);
    georef.setVertVelocity(dsGeoref.vert_velocity_mps);
    georef.setHeading(dsGeoref.heading_deg);
    georef.setTrack(dsGeoref.track_deg);
    georef.setRoll(dsGeoref.roll_deg);
    georef.setPitch(dsGeoref.pitch_deg);
    georef.setDrift(dsGeoref.drift_angle_deg);
    georef.setRotation(dsGeoref.rotation_angle_deg);
    georef.setTilt(dsGeoref.tilt_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setRollRate(dsGeoref.roll_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    ray.setGeoref(georef);
  }

  // load up vector of field offsets

  vector<int> offsets;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    int offset = _findFieldOffset(field.id);
    offsets.push_back(offset);
  }

  // load up fields
  
  const MomentsFields *fieldsArray = beam.getFields(); // unfiltered
  const MomentsFields *fieldsFArray = beam.getFieldsF(); // filtered
  Radx::fl32 missingFl32 = MomentsFields::missingDouble;
  
  int nGatesOut = beam.getNGatesOut();
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    
    const Params::output_field_t &ofield = _params._output_fields[ifield];
    int offset = offsets[ifield];
    
    if (ofield.write_unfiltered) {

      // create field
      
      RadxField *unfiltFld = new RadxField(ofield.name, ofield.units);
      unfiltFld->setMissingFl32(missingFl32);

      // add moments data
      
      Radx::fl32 *fdata = new Radx::fl32[nGatesOut];
      for (int igate = 0; igate < nGatesOut; igate++) {
        const MomentsFields &flds = fieldsArray[igate];
        const double *start = &flds.start;
        double val = *(start + offset);
        fdata[igate] = val;
      }
      unfiltFld->addDataFl32(nGatesOut, fdata);
      delete[] fdata;

      // convert missing value to standard
      
      unfiltFld->setMissingFl32(Radx::missingFl32);

      // add field to output ray
      
      ray.addField(unfiltFld);

    }
      
    if (ofield.write_filtered) {

      // create field
      
      string filtName = ofield.name;
      filtName += "_F";
      RadxField *filtFld = new RadxField(filtName, ofield.units);
      filtFld->setMissingFl32(missingFl32);

      // add moments data
      
      Radx::fl32 *fdata = new Radx::fl32[nGatesOut];
      for (int igate = 0; igate < nGatesOut; igate++) {
        const MomentsFields &flds = fieldsFArray[igate];
        const double *start = &flds.start;
        double val = *(start + offset);
        fdata[igate] = val;
      }
      filtFld->addDataFl32(nGatesOut, fdata);
      delete[] fdata;

      // convert missing value to standard
      
      filtFld->setMissingFl32(Radx::missingFl32);

      // add field to output ray
      
      ray.addField(filtFld);

    } // if (ofield.write_filtered) 
      
  } // ifield
  
  bool printBeamDebug = (_params.debug >= Params::DEBUG_VERBOSE);
  if (_params.debug &&
      ((_nBeamsWritten % _params.beam_count_for_debug_print) == 0)) {
    printBeamDebug = true;
  }
  if (printBeamDebug) {
    RadxTime rayTime(ray.getRadxTime());
    time_t now = time(NULL);
    int lateSecs = now - ray.getTimeSecs();
    if (_params.mode == Params::ARCHIVE) {
      fprintf(stderr,
              "-->> OutputFmq::_writeBeamRadx %s - %s, vol: %.3d, "
              "sweep: %.3d, "
              "az: %6.3f, el %5.3f, nSamples: %3d\n",
              rayTime.asString(3).c_str(),
              Radx::sweepModeToShortStr(ray.getSweepMode()).c_str(), 
              ray.getVolumeNumber(), ray.getSweepNumber(),
              ray.getAzimuthDeg(), ray.getElevationDeg(),
              beam.getNSamples());
    } else {
      fprintf(stderr,
              "-->> OutputFmq::_writeBeamRadx %s (late %d) - %s, vol: %.3d, "
              "sweep: %.3d, "
              "az: %6.3f, el %5.3f, nSamples: %3d\n",
              rayTime.asString(3).c_str(), lateSecs,
              Radx::sweepModeToShortStr(ray.getSweepMode()).c_str(), 
              ray.getVolumeNumber(), ray.getSweepNumber(),
              ray.getAzimuthDeg(), ray.getElevationDeg(),
              beam.getNSamples());
    }
  }

  // create message
  
  RadxMsg msg;
  ray.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "=========== Writing out ray =============" << endl;
    cerr << "  el, az: "
         << ray.getElevationDeg() << ", "
         << ray.getAzimuthDeg() << endl;
    cerr << "=========================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_writeCalibRadx" << endl;
    cerr << "  Cannot write ray to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  _nBeamsWritten++;
  return 0;

}

////////////////////////////////////////
// put volume flags

void OutputFmq::_putStartOfVolumeRadx(int volNum, const Beam &beam)
{

  // create event
  
  RadxEvent event;
  event.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  event.setStartOfVolume(true);
  event.setVolumeNumber(volNum);
  event.setSweepMode((Radx::SweepMode_t) beam.getScanMode());
  
  // create message

  RadxMsg msg;
  event.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "======= Writing out start of vol event =========" << endl;
    event.print(cerr);
    cerr << "================================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_putStartOfVolumeRadx" << endl;
    cerr << "  Cannot write start of vol event to queue" << endl;
    // reopen the queue
    _openFmq();
  }

}

void OutputFmq::_putEndOfVolumeRadx(int volNum, const Beam &beam)
{

  // create event
  
  RadxEvent event;
  event.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  event.setEndOfVolume(true);
  event.setVolumeNumber(volNum);
  event.setSweepMode((Radx::SweepMode_t) beam.getScanMode());

  // create message

  RadxMsg msg;
  event.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "======= Writing out end of vol event =========" << endl;
    event.print(cerr);
    cerr << "================================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_putEndOfVolumeRadx" << endl;
    cerr << "  Cannot write end of vol event to queue" << endl;
    // reopen the queue
    _openFmq();
  }

}

void OutputFmq::_putStartOfTiltRadx(int tiltNum, const Beam &beam)
{

  // create event
  
  RadxEvent event;
  event.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  event.setStartOfSweep(true);
  event.setSweepNumber(tiltNum);
  event.setSweepMode((Radx::SweepMode_t) beam.getScanMode());

  // create message

  RadxMsg msg;
  event.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "====== Writing out start of sweep event ========" << endl;
    event.print(cerr);
    cerr << "================================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_putStartOfTiltRadx" << endl;
    cerr << "  Cannot write start of sweep event to queue" << endl;
    // reopen the queue
    _openFmq();
  }

}

void OutputFmq::_putEndOfTiltRadx(int tiltNum, const Beam &beam)
{

  // create event
  
  RadxEvent event;
  event.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  event.setEndOfSweep(true);
  event.setSweepNumber(tiltNum);
  event.setSweepMode((Radx::SweepMode_t) beam.getScanMode());

  // create message

  RadxMsg msg;
  event.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "======= Writing out end of sweep event =========" << endl;
    event.print(cerr);
    cerr << "================================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_putEndOfTiltRadx" << endl;
    cerr << "  Cannot write start of vol event to queue" << endl;
    // reopen the queue
    _openFmq();
  }

}

void OutputFmq::_putNewScanTypeRadx(int scanType, const Beam &beam)
{

  // create event
  
  RadxEvent event;
  event.setTime(beam.getTimeSecs(), beam.getNanoSecs());
  event.setSweepMode((Radx::SweepMode_t) scanType);

  // create message

  RadxMsg msg;
  event.serialize(msg);
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "======= Writing out new scan type =========" << endl;
    event.print(cerr);
    cerr << "===========================================" << endl;
  }
  
  // write the message
  
  if (_radxQueue->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - OutputFmq::_putNewScanTypeRadx" << endl;
    cerr << "  Cannot write new scan mode event to queue" << endl;
    // reopen the queue
    _openFmq();
  }

}

////////////////////////////////////////
// open the FMQ

int OutputFmq::_openFmq()
  
{
  int iret = 0;
  if (_useRadx) {
    iret = _openRadxQueue();
  } else {
    iret = _openDsRadarQueue();
  }
  return iret;
}

int OutputFmq::_openDsRadarQueue()
  
{

  if (_dsrQueue != NULL) {
    delete _dsrQueue;
  }

  _dsrQueue = new DsRadarQueue;

  // initialize the output queue
  
  if (_dsrQueue->init(_params.output_fmq_url,
		    _progName.c_str(),
		    _params.debug >= Params::DEBUG_VERBOSE,
		    DsFmq::READ_WRITE, DsFmq::END,
		    _params.output_fmq_compress,
		    _params.output_fmq_nslots,
		    _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open dsradar fmq, URL: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _dsrQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.write_blocking) {
    _dsrQueue->setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _dsrQueue->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _dsrQueue->setSingleWriter();

  return 0;

}

int OutputFmq::_openRadxQueue()
  
{

  if (_radxQueue != NULL) {
    delete _radxQueue;
  }

  _radxQueue = new DsFmq;

  // initialize the output queue
  
  if (_radxQueue->init(_params.output_fmq_url,
                       _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::READ_WRITE, DsFmq::END,
                       _params.output_fmq_compress,
                       _params.output_fmq_nslots,
                       _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open radx fmq, URL: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _radxQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.write_blocking) {
    _radxQueue->setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _radxQueue->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _radxQueue->setSingleWriter();

  return 0;

}

////////////////////////////////////////////////////////////////////
// get the offset for a given field ID
// This is the offset of the data element in the MomentsFields object
// realtive to the ncp field.

int OutputFmq::_findFieldOffset(Params::field_id_t fieldId)

{

  const double *start = &_flds.start;

  switch (fieldId) {
    
    // reflectivity
      
    case Params::DBZ:
      return (&_flds.dbz - start);
    case Params::DBZ_NO_ATMOS_ATTEN:
      return (&_flds.dbz_no_atmos_atten - start);
    case Params::DBZHC:
      return (&_flds.dbzhc - start);
    case Params::DBZVC:
      return (&_flds.dbzvc - start);
    case Params::DBZHX:
      return (&_flds.dbzhx - start);
    case Params::DBZVX:
      return (&_flds.dbzvx - start);

      // velocity

    case Params::VEL:
      return (&_flds.vel - start);
    case Params::VEL_ALT:
      return (&_flds.vel_alt - start);
    case Params::VEL_HV:
      return (&_flds.vel_hv - start);
    case Params::VEL_H_ONLY:
      return (&_flds.vel_h_only - start);
    case Params::VEL_V_ONLY:
      return (&_flds.vel_v_only - start);
    case Params::VEL_ALT_FOLD_INTERVAL:
      return (&_flds.vel_alt_fold_interval - start);
    case Params::VEL_ALT_FOLD_CONFIDENCE:
      return (&_flds.vel_alt_fold_confidence - start);
    case Params::VEL_CORR_VERT:
      return (&_flds.vel_corr_vert - start);
    case Params::VEL_CORR_MOTION:
      return (&_flds.vel_corr_motion - start);
      
      // STAGGERED PRT VEL
      
    case Params::VEL_PRT_SHORT:
      return (&_flds.vel_prt_short - start);
    case Params::VEL_PRT_LONG:
      return (&_flds.vel_prt_long - start);
    case Params::VEL_DIFF:
      return (&_flds.vel_diff - start);
    case Params::VEL_UNFOLD_INTERVAL:
      return (&_flds.vel_unfold_interval - start);

      // width

    case Params::WIDTH:
      return (&_flds.width - start);
    case Params::WIDTH_R0R1:
      return (&_flds.width_r0r1 - start);
    case Params::WIDTH_R1R2:
      return (&_flds.width_r1r2 - start);
    case Params::WIDTH_R1R3:
      return (&_flds.width_r1r3 - start);
    case Params::WIDTH_PPLS:
      return (&_flds.width_ppls - start);
    case Params::WIDTH_H_ONLY:
      return (&_flds.width_h_only - start);
    case Params::WIDTH_V_ONLY:
      return (&_flds.width_v_only - start);
    case Params::WIDTH_PRT_LONG:
      return (&_flds.width_prt_long - start);
    case Params::WIDTH_PRT_SHORT:
      return (&_flds.width_prt_short - start);
    case Params::WIDTH_CORR_MOTION:
      return (&_flds.width_corr_motion - start);

      // normalized coherent power
      
    case Params::NCP:
      return (&_flds.ncp - start);
    case Params::NCP_H_ONLY:
      return (&_flds.ncp_h_only - start);
    case Params::NCP_V_ONLY:
      return (&_flds.ncp_v_only - start);
    case Params::NCP_H_MINUS_V:
      return (&_flds.ncp_h_minus_v - start);
    case Params::NCP_TRIP1:
      return (&_flds.ncp_trip1 - start);
    case Params::NCP_TRIP2:
      return (&_flds.ncp_trip2 - start);
    case Params::NCP_TRIP3:
      return (&_flds.ncp_trip3 - start);
    case Params::NCP_TRIP4:
      return (&_flds.ncp_trip4 - start);
    case Params::NCP_PRT_SHORT:
      return (&_flds.ncp_prt_short - start);
    case Params::NCP_PRT_LONG:
      return (&_flds.ncp_prt_long - start);
    case Params::NCP_TRIP_FLAG:
      return (&_flds.ncp_trip_flag - start);

      // NOISE BIAS RELATIVE TO CALIBRATION

    case Params::NOISE_BIAS_DB_HC:
      return (&_flds.noise_bias_db_hc - start);
    case Params::NOISE_BIAS_DB_HX:
      return (&_flds.noise_bias_db_hx - start);
    case Params::NOISE_BIAS_DB_VC:
      return (&_flds.noise_bias_db_vc - start);
    case Params::NOISE_BIAS_DB_VX:
      return (&_flds.noise_bias_db_vx - start);

      // SIGNAL-TO-NOISE RATIO

    case Params::SNR:
      return (&_flds.snr - start);
    case Params::SNRHC:
      return (&_flds.snrhc - start);
    case Params::SNRHX:
      return (&_flds.snrhx - start);
    case Params::SNRVC:
      return (&_flds.snrvc - start);
    case Params::SNRVX:
      return (&_flds.snrvx - start);

      // UNCALIBRATED POWER
      
    case Params::DBM:
      return (&_flds.dbm - start);
    case Params::DBMHC:
      return (&_flds.dbmhc - start);
    case Params::DBMHX:
      return (&_flds.dbmhx - start);
    case Params::DBMVC:
      return (&_flds.dbmvc - start);
    case Params::DBMVX:
      return (&_flds.dbmvx - start);

      // NOISE SUBTRACTED POWER

    case Params::DBMHC_NS:
      return (&_flds.dbmhc_ns - start);
    case Params::DBMHX_NS:
      return (&_flds.dbmhx_ns - start);
    case Params::DBMVC_NS:
      return (&_flds.dbmvc_ns - start);
    case Params::DBMVX_NS:
      return (&_flds.dbmvx_ns - start);
      
      // DUAL POL

    case Params::ZDRM:
      return (&_flds.zdrm - start);
    case Params::ZDR:
      return (&_flds.zdr - start);
    case Params::ZDR_BIAS:
      return (&_flds.zdr_bias - start);
    case Params::LDR:
      return (&_flds.ldr - start);
    case Params::LDRHM:
      return (&_flds.ldrhm - start);
    case Params::LDRH:
      return (&_flds.ldrh - start);
    case Params::LDRVM:
      return (&_flds.ldrvm - start);
    case Params::LDRV:
      return (&_flds.ldrv - start);
    case Params::LDR_DIFF:
      return (&_flds.ldr_diff - start);
    case Params::LDR_MEAN:
      return (&_flds.ldr_mean - start);
    case Params::RHOHV:
      return (&_flds.rhohv - start);
    case Params::RHOHV_NNC:
      return (&_flds.rhohv_nnc - start);
    case Params::PHIDP0:
      return (&_flds.phidp0 - start);
    case Params::PHIDP:
      return (&_flds.phidp - start);
    case Params::PHIDP_COND:
      return (&_flds.phidp_cond - start);
    case Params::PHIDP_FILT:
      return (&_flds.phidp_filt - start);
    case Params::PHIDP_SDEV_4KDP:
      return (&_flds.phidp_sdev_4kdp - start);
    case Params::PHIDP_JITTER_4KDP:
      return (&_flds.phidp_jitter_4kdp - start);
    case Params::ZDR_SDEV_4KDP:
      return (&_flds.zdr_sdev_4kdp - start);
    case Params::KDP:
      return (&_flds.kdp - start);
    case Params::PSOB:
      return (&_flds.psob - start);
    case Params::KDP_HB:
      return (&_flds.kdp_hb - start);

      // co-cross correlation

    case Params::RHO_HC_VX:
      return (&_flds.rho_hcvx - start);
    case Params::RHO_VC_HX:
      return (&_flds.rho_vchx - start);
    case Params::RHO_VX_HX:
      return (&_flds.rho_vxhx - start);
    case Params::RHO_PHIDP:
      return (&_flds.rho_phidp - start);

      // cross polar ratio - CPR

    case Params::CPR_MAG:
      return (&_flds.cpr_mag - start);
    case Params::CPR_PHASE:
      return (&_flds.cpr_phase - start);
    case Params::CPR_LDR:
      return (&_flds.cpr_ldr - start);

      // CMD

    case Params::CPA:
      return (&_flds.cpa - start);
    case Params::TDBZ:
      return (&_flds.tdbz - start);
    case Params::SPIN:
      return (&_flds.spin - start);
    case Params::MAX_TDBZ_SPIN:
      return (&_flds.max_tdbz_spin - start);
    case Params::ZDR_SDEV:
      return (&_flds.zdr_sdev - start);
    case Params::PHIDP_SDEV:
      return (&_flds.phidp_sdev - start);
    case Params::DBZ_DIFF_SQ:
      return (&_flds.dbz_diff_sq - start);
    case Params::DBZ_SPIN_CHANGE:
      return (&_flds.dbz_spin_change - start);
    case Params::CMD:
      return (&_flds.cmd - start);
    case Params::CMD_FLAG:
      return (&_flds.cmd_flag - start);
    case Params::TDBZ_INTEREST:
      return (&_flds.tdbz_interest - start);
    case Params::SPIN_INTEREST:
      return (&_flds.spin_interest - start);
    case Params::CPA_INTEREST:
      return (&_flds.cpa_interest - start);
    case Params::ZDR_SDEV_INTEREST:
      return (&_flds.zdr_sdev_interest - start);
    case Params::PHIDP_SDEV_INTEREST:
      return (&_flds.phidp_sdev_interest - start);

      // CLUTTER FILTERED
      
    case Params::CLUT:
      return (&_flds.clut - start);
    case Params::CLUT_2_WX_RATIO:
      return (&_flds.clut_2_wx_ratio - start);
    case Params::SPECTRAL_NOISE:
      return (&_flds.spectral_noise - start);
    case Params::SPECTRAL_SNR:
      return (&_flds.spectral_snr - start);

      // NOISE
      
    case Params::NOISE_FLAG:
      return (&_flds.noise_flag - start);
    case Params::NOISE_INTEREST:
      return (&_flds.noise_interest - start);
    case Params::SIGNAL_FLAG:
      return (&_flds.signal_flag - start);
    case Params::SIGNAL_INTEREST:
      return (&_flds.signal_interest - start);

      // REFRACT

    case Params::AIQ_HC:
      return (&_flds.aiq_hc - start);
    case Params::NIQ_HC:
      return (&_flds.niq_hc - start);
    case Params::AIQ_VC:
      return (&_flds.aiq_vc - start);
    case Params::NIQ_VC:
      return (&_flds.niq_vc - start);

      // SZ8-64

    case Params::SZ_TRIP_FLAG:
      return (&_flds.sz_trip_flag - start);
    case Params::SZ_LEAKAGE:
      return (&_flds.sz_leakage - start);

      // censoring flag

    case Params::CENSORING_FLAG:
      return (&_flds.censoring_flag - start);

      // covariances

    case Params::LAG0_HC_DB:
      return (&_flds.lag0_hc_db - start);
    case Params::LAG0_HX_DB:
      return (&_flds.lag0_hx_db - start);
    case Params::LAG0_VC_DB:
      return (&_flds.lag0_vc_db - start);
    case Params::LAG0_VX_DB:
      return (&_flds.lag0_vx_db - start);
    case Params::LAG0_HC_SHORT_DB:
      return (&_flds.lag0_hc_short - start);

    case Params::LAG0_VC_SHORT_DB:
      return (&_flds.lag0_vc_short - start);
    case Params::LAG0_HC_LONG_DB:
      return (&_flds.lag0_hc_long - start);
    case Params::LAG0_VC_LONG_DB:
      return (&_flds.lag0_vc_long - start);
    case Params::LAG0_VCHX_DB:
      return (&_flds.lag0_vchx_db - start);
    case Params::LAG0_VCHX_PHASE:
      return (&_flds.lag0_vchx_phase - start);

    case Params::LAG0_HCVX_DB:
      return (&_flds.lag0_hcvx_db - start);
    case Params::LAG0_HCVX_PHASE:
      return (&_flds.lag0_hcvx_phase - start);

    case Params::LAG1_HC_DB:
      return (&_flds.lag1_hc_db - start);
    case Params::LAG1_HC_PHASE:
      return (&_flds.lag1_hc_phase - start);
    case Params::LAG1_VC_DB:
      return (&_flds.lag1_vc_db - start);
    case Params::LAG1_VC_PHASE:
      return (&_flds.lag1_vc_phase - start);

    case Params::LAG1_HC_LONG_DB:
      return (&_flds.lag1_hc_long_db - start);
    case Params::LAG1_HC_LONG_PHASE:
      return (&_flds.lag1_hc_long_phase - start);
    case Params::LAG1_VC_LONG_DB:
      return (&_flds.lag1_vc_long_db - start);
    case Params::LAG1_VC_LONG_PHASE:
      return (&_flds.lag1_vc_long_phase - start);

    case Params::LAG1_HC_SHORT_DB:
      return (&_flds.lag1_hc_short_db - start);
    case Params::LAG1_HC_SHORT_PHASE:
      return (&_flds.lag1_hc_short_phase - start);
    case Params::LAG1_VC_SHORT_DB:
      return (&_flds.lag1_vc_short_db - start);
    case Params::LAG1_VC_SHORT_PHASE:
      return (&_flds.lag1_vc_short_phase - start);

    case Params::LAG1_HC_LONG_TO_SHORT_PHASE:
      return (&_flds.lag1_hc_long_to_short_phase - start);
    case Params::LAG1_VC_LONG_TO_SHORT_DB:
      return (&_flds.lag1_vc_long_to_short_db - start);
    case Params::LAG1_VC_LONG_TO_SHORT_PHASE:
      return (&_flds.lag1_vc_long_to_short_phase - start);
    case Params::LAG1_HC_LONG_TO_SHORT_DB:
      return (&_flds.lag1_hc_long_to_short_db - start);

    case Params::LAG1_HC_SHORT_TO_LONG_PHASE:
      return (&_flds.lag1_hc_short_to_long_phase - start);
    case Params::LAG1_VC_SHORT_TO_LONG_DB:
      return (&_flds.lag1_vc_short_to_long_db - start);
    case Params::LAG1_VC_SHORT_TO_LONG_PHASE:
      return (&_flds.lag1_vc_short_to_long_phase - start);
    case Params::LAG1_HC_SHORT_TO_LONG_DB:
      return (&_flds.lag1_hc_short_to_long_db - start);

    case Params::LAG1_HCVC_DB:
      return (&_flds.lag1_hcvc_db - start);
    case Params::LAG1_HCVC_PHASE:
      return (&_flds.lag1_hcvc_phase - start);
    case Params::LAG1_VCHC_DB:
      return (&_flds.lag1_vchc_db - start);
    case Params::LAG1_VCHC_PHASE:
      return (&_flds.lag1_vchc_phase - start);
    case Params::LAG1_VXHX_DB:
      return (&_flds.lag1_vxhx_db - start);
    case Params::LAG1_VXHX_PHASE:
      return (&_flds.lag1_vxhx_phase - start);

    case Params::LAG2_HC_DB:
      return (&_flds.lag2_hc_db - start);
    case Params::LAG2_HC_PHASE:
      return (&_flds.lag2_hc_phase - start);
    case Params::LAG2_VC_DB:
      return (&_flds.lag2_vc_db - start);
    case Params::LAG2_VC_PHASE:
      return (&_flds.lag2_vc_phase - start);

    case Params::LAG3_HC_DB:
      return (&_flds.lag3_hc_db - start);
    case Params::LAG3_HC_PHASE:
      return (&_flds.lag3_hc_phase - start);
    case Params::LAG3_VC_DB:
      return (&_flds.lag3_vc_db - start);
    case Params::LAG3_VC_PHASE:
      return (&_flds.lag3_vc_phase - start);

    case Params::RVVHH0_DB:
      return (&_flds.rvvhh0_db - start);
    case Params::RVVHH0_PHASE:
      return (&_flds.rvvhh0_phase - start);

    case Params::RVVHH0_LONG_DB:
      return (&_flds.rvvhh0_long_db - start);
    case Params::RVVHH0_LONG_PHASE:
      return (&_flds.rvvhh0_long_phase - start);
    case Params::RVVHH0_SHORT_DB:
      return (&_flds.rvvhh0_short_db - start);
    case Params::RVVHH0_SHORT_PHASE:
      return (&_flds.rvvhh0_short_phase - start);

      // sdev of V-V covariance

    case Params::SDEV_VV:
      return (&_flds.sdev_vv - start);

      // attenuation correction

    case Params::DBZ_ATTEN_CORRECTION:
      return (&_flds.dbz_atten_correction - start);
    case Params::ZDR_ATTEN_CORRECTION:
      return (&_flds.zdr_atten_correction - start);
    case Params::DBZ_ATTEN_CORRECTED:
      return (&_flds.dbz_atten_corrected - start);
    case Params::ZDR_ATTEN_CORRECTED:
      return (&_flds.zdr_atten_corrected - start);

      // DEBUG

    case Params::PRT:
      return (&_flds.prt - start);
    case Params::NUM_PULSES:
      return (&_flds.num_pulses - start);
    case Params::TEST:
      return (&_flds.test - start);
    case Params::TEST2:
      return (&_flds.test2 - start);
    case Params::TEST3:
      return (&_flds.test3 - start);
    case Params::TEST4:
      return (&_flds.test4 - start);
    case Params::TEST5:
      return (&_flds.test5 - start);
  }

  return 0;

}

