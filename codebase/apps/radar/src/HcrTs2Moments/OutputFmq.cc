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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
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
  _nBeamsWritten = 0;
  _radxQueue = NULL;

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

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(const Beam &beam)

{
  int iret = 0;
  pthread_mutex_lock(&_busy);
  iret = _writePlatformRadx(beam);
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
  iret = _writeCalibRadx(beam);
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
  iret = _writeStatusXmlRadx(beam);
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
  iret = _writeBeamRadx(beam);
  pthread_mutex_unlock(&_busy);
  return iret;
}

////////////////////////////////////////
// put volume flags

void OutputFmq::putStartOfVolume(int volNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  _putStartOfVolumeRadx(volNum, beam);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfVolume(int volNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  _putEndOfVolumeRadx(volNum, beam);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putStartOfTilt(int tiltNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  _putStartOfTiltRadx(tiltNum, beam);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfTilt(int tiltNum, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  _putEndOfTiltRadx(tiltNum, beam);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putNewScanType(int scanType, const Beam &beam)
{
  pthread_mutex_lock(&_busy);
  _putNewScanTypeRadx(scanType, beam);
  pthread_mutex_unlock(&_busy);
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

  ray.setPrtMode(Radx::PRT_MODE_FIXED);
  ray.setPrtRatio(1.0);
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
  ray.setFixedAngleDeg(beam.getTargetEl());

  ray.setNSamples(beam.getNSamples());
  
  ray.setPulseWidthUsec(beam.getPulseWidth() * 1.0e6);
  ray.setPrtSec(beam.getPrt());
  ray.setNyquistMps(beam.getNyquist());

  ray.setUnambigRangeKm(beam.getUnambigRangeKm());

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
  Radx::fl32 missingFl32 = MomentsFields::missingDouble;
  
  int nGatesOut = beam.getNGatesOut();
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    
    const Params::output_field_t &ofield = _params._output_fields[ifield];
    int offset = offsets[ifield];
    
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
    
    // set folding attributes
    
    _setFoldingAttr(beam, ofield, *unfiltFld);
    
    // add field to output ray
    
    ray.addField(unfiltFld);

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
// set folding attributes

void OutputFmq::_setFoldingAttr(const Beam &beam,
                                const Params::output_field_t &ofld,
                                RadxField &field)

{
      
  if (ofld.id == Params::VEL ||
      ofld.id == Params::VEL_CORR_MOTION ||
      ofld.id == Params::VEL_CORR_VERT) {
    field.setFieldFolds(beam.getNyquist() * -1.0,
                        beam.getNyquist());
  }

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
  return _openRadxQueue();
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
    case Params::DBZHC:
      return (&_flds.dbzhc - start);
    case Params::DBZVC:
      return (&_flds.dbzvc - start);
      
      // velocity
      
    case Params::VEL:
      return (&_flds.vel - start);
    case Params::VEL_CORR_VERT:
      return (&_flds.vel_corr_vert - start);
    case Params::VEL_CORR_MOTION:
      return (&_flds.vel_corr_motion - start);
      
      // width

    case Params::WIDTH:
      return (&_flds.width - start);
    case Params::WIDTH_CORR_MOTION:
      return (&_flds.width_corr_motion - start);

      // normalized coherent power
      
    case Params::NCP:
      return (&_flds.ncp - start);
      
      // NOISE BIAS RELATIVE TO CALIBRATION

    case Params::NOISE_BIAS_DB_HC:
      return (&_flds.noise_bias_db_hc - start);
    case Params::NOISE_BIAS_DB_VC:
      return (&_flds.noise_bias_db_vc - start);
      
      // NOISE IDENTIFICATION
      
    case Params::NOISE_FLAG:
      return (&_flds.noise_flag - start);
    case Params::SIGNAL_FLAG:
      return (&_flds.signal_flag - start);
    case Params::NOISE_INTEREST:
      return (&_flds.noise_interest - start);
    case Params::SIGNAL_INTEREST:
      return (&_flds.signal_interest - start);

    case Params::NOISE_ACCUM_PHASE_CHANGE:
      return (&_flds.noise_accum_phase_change - start);
    case Params::NOISE_PHASE_CHANGE_ERROR:
      return (&_flds.noise_phase_change_error - start);
    case Params::NOISE_DBM_SDEV:
      return (&_flds.noise_dbm_sdev - start);
    case Params::NOISE_NCP_MEAN:
      return (&_flds.noise_ncp_mean - start);
      
      // SIGNAL-TO-NOISE RATIO

    case Params::SNR:
      return (&_flds.snr - start);
    case Params::SNRHC:
      return (&_flds.snrhc - start);
    case Params::SNRVC:
      return (&_flds.snrvc - start);
      
      // UNCALIBRATED POWER
      
    case Params::DBM:
      return (&_flds.dbm - start);
    case Params::DBMHC:
      return (&_flds.dbmhc - start);
    case Params::DBMVC:
      return (&_flds.dbmvc - start);
      
      // NOISE SUBTRACTED POWER

    case Params::DBMHC_NS:
      return (&_flds.dbmhc_ns - start);
    case Params::DBMVC_NS:
      return (&_flds.dbmvc_ns - start);
      
      // DUAL POL

    case Params::LDR:
      return (&_flds.ldr - start);
    case Params::LDRH:
      return (&_flds.ldrh - start);
    case Params::LDRV:
      return (&_flds.ldrv - start);
      
      // censoring flag
      
    case Params::CENSORING_FLAG:
      return (&_flds.censoring_flag - start);
      
      // DEBUG
      
    case Params::PRT:
      return (&_flds.prt - start);
    case Params::NUM_PULSES:
      return (&_flds.num_pulses - start);
    case Params::TEST0:
      return (&_flds.test0 - start);
    case Params::TEST1:
      return (&_flds.test1 - start);
    case Params::TEST2:
      return (&_flds.test2 - start);
    case Params::TEST3:
      return (&_flds.test3 - start);
    case Params::TEST4:
      return (&_flds.test4 - start);
    case Params::TEST5:
      return (&_flds.test5 - start);
    case Params::TEST6:
      return (&_flds.test6 - start);
    case Params::TEST7:
      return (&_flds.test7 - start);
    case Params::TEST8:
      return (&_flds.test8 - start);
    case Params::TEST9:
      return (&_flds.test9 - start);
  }

  return 0;

}

