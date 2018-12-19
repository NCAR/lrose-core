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
// IwrfMomReader.cc
//
// IwrfMomReader object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////
//
// IwrfMomReader reads moments in iwrf format and the older
// DsRadar format.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <toolsa/DateTime.hh>
#include <dataport/swap.h>
#include <radar/IwrfMoments.hh>
#include <radar/IwrfMomReader.hh>
#include <radar/RadarCalib.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxGeoref.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

IwrfMomReader::IwrfMomReader()
  
{
  _opsInfo.setDebug(IWRF_DEBUG_OFF);
  _nonBlocking = false;
  _msecsNonblockingWait = 0;
  _msecsBlockingTimeout = -1;
  _timedOut = false;
  _endOfFile = false;
  _latestRay = NULL;
  _savedRay = NULL;
  _rayReady = false;
  _debug = IWRF_DEBUG_OFF;
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfMomReader::~IwrfMomReader()

{

  if (_savedRay) {
    delete _savedRay;
  }
  if (_latestRay) {
    delete _latestRay;
  }

}

//////////////////////////////////////////////////////////////////
// seekToStart - default is do nothing

void IwrfMomReader::seekToStart()

{
}

//////////////////////////////////////////////////////////////////
// seekToEnd - default is do nothing

void IwrfMomReader::seekToEnd()

{
}

//////////////////////////////////////////////////////////
// Read next ray
// Returns RadxRay pointer on success, NULL on failure.
//
// This method is used by the FMQ and TCP subclasses.
// It calls the virtual _getNextMsg() method, which will use the
// appropriate device.
//
// Caller is responsible for freeing the ray.
//
// Call the following for supplementary info on the ray:
//   getPlatform()
//   getScan()
//   getRCalibs()
//   getEvents()
//   getStatusXml()
  
RadxRay *IwrfMomReader::readNextRay()

{
  
  RadxRay *userRay = NULL;

  while (userRay == NULL) {

    // create new ray
    
    if (_latestRay) {
      delete _latestRay;
    }
    _latestRay = new RadxRay;
    _rayReady = false;
    
    // clear events and flags
    
    _latestEvents.clear();
    _latestFlags.clear();
    
    // get messages and process them until a ray is ready
    
    while (!_rayReady) {
      
      if (_getNextMsg()) {
        return NULL;
      }
      
      // try disassembling as a DsRadarMsg
      
      _dsContents = 0;
      if (_dsRadarMsg.disassemble(_msgBuf.getPtr(), 
                                  _msgBuf.getLen(), 
                                  &_dsContents) == 0) {
        if (_handleDsRadarMessage()) {
          continue;
        }
      } else {
        if (_handleIwrfMessage()) {
          continue;
        }
      }
      
    } // while
    
    // we return the previous ray to the user
    // the user is responsible for freeing it
    
    _events = _savedEvents;
    _savedEvents = _latestEvents;
    _latestEvents.clear();
    
    _flags = _savedFlags;
    _savedFlags = _latestFlags;
    _latestFlags.clear();
    
    userRay = _savedRay;
    _savedRay = _latestRay;
    _latestRay = NULL;

  } // while (userRay == NULL)

  return userRay;

}

/////////////////////////////////////////////////////////////////////
// handle input DsRadarMsg
//
// returns 0 on success, -1 on failure

int IwrfMomReader::_handleDsRadarMessage()
  
{
  
  if (_dsContents & DsRadarMsg::RADAR_PARAMS) {
    _decodeDsRadarParams();
  }

  if (_dsContents & DsRadarMsg::STATUS_XML) {
    _decodeDsStatusXml();
  }

  if (_dsContents & DsRadarMsg::RADAR_CALIB) {
    _decodeDsRadarCalib();
  }

  if (_dsContents & DsRadarMsg::RADAR_FLAGS) {
    _decodeDsRadarFlags();
  }
  
  if ((_dsContents & DsRadarMsg::RADAR_BEAM) && _dsRadarMsg.allParamsSet()) {
    _decodeDsRadarBeam();
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// decode radar params into platform

void IwrfMomReader::_decodeDsRadarParams()

{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  // _dsRadarMsg.radarParams.copy(radarMsg.getRadarParams());

  const DsRadarParams& rparams = _dsRadarMsg.getRadarParams();
  
  _platform.setInstrumentName(rparams.radarName);
  _platform.setSiteName(rparams.radarName);
  // _platform.setScanName(rparams.scanTypeName);

  switch (rparams.radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _platform.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _platform.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }

  _platform.setLatitudeDeg(rparams.latitude);
  _platform.setLongitudeDeg(rparams.longitude);
  _platform.setAltitudeKm(rparams.altitude);

  // check to make sure altitude is in km, not meters
  
  if (_platform.getAltitudeKm() > 8000.0 && _debug){
    cerr << "WARNING : Sensor altitude is "
         << _platform.getAltitudeKm() << "km" << endl;
    cerr << "  Are the correct units being used for altitude?" << endl;
    cerr << "  Incorrect altitude results in bad cart remapping." << endl;
  }
  
  _platform.setWavelengthCm(rparams.wavelength);
  _platform.setRadarBeamWidthDegH(rparams.horizBeamWidth);
  _platform.setRadarBeamWidthDegV(rparams.vertBeamWidth);
  _latestFlags.platformUpdated = true;

}

////////////////////////////////////////////////////////////////
// decode status XML from DsRadarMsg

void IwrfMomReader::_decodeDsStatusXml()
  
{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "=========>> got STATUS_XML" << endl;
  }

  _statusXml = _dsRadarMsg.getStatusXml();
  _latestFlags.statusXmlUpdated = true;

}

////////////////////////////////////////////////////////////////
// decode radar calib

void IwrfMomReader::_decodeDsRadarCalib()

{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_CALIB" << endl;
  }

  const DsRadarCalib dsCalib = _dsRadarMsg.getRadarCalib();

  // set platform properties

  if (dsCalib.getAntGainDbH() > -9990 &&
      _platform.getRadarAntennaGainDbH() < -9990) {
    _platform.setRadarAntennaGainDbH(dsCalib.getAntGainDbH());
  }
  if (dsCalib.getAntGainDbV() > -9990 &&
      _platform.getRadarAntennaGainDbV() < -9990) {
    _platform.setRadarAntennaGainDbV(dsCalib.getAntGainDbV());
  }
  if (dsCalib.getBeamWidthDegH() > -9990 &&
      _platform.getRadarBeamWidthDegH() < -9990) {
    _platform.setRadarBeamWidthDegH(dsCalib.getBeamWidthDegH());
  }
  if (dsCalib.getBeamWidthDegV() > -9990 &&
      _platform.getRadarBeamWidthDegV() < -9990) {
    _platform.setRadarBeamWidthDegV(dsCalib.getBeamWidthDegV());
  }
  _latestFlags.platformUpdated = true;

  // create RadxRcalib

  RadxRcalib rcal;
  RadarCalib::copyDsRadarToRadx(dsCalib, rcal);

  // remove cal with the same pulse width
  // if one already exists
  
  double thisPulseWidth = rcal.getPulseWidthUsec();
  for (vector<RadxRcalib>::iterator ii = _rcalibs.begin();
       ii != _rcalibs.end(); ii++) {
    double prevPulseWidth = ii->getPulseWidthUsec();
    if (fabs(thisPulseWidth - prevPulseWidth) < 0.0001) {
      // same pulse width, so remove existing one
      _rcalibs.erase(ii);
      break;
    }
  }

  // store it

  _rcalibs.push_back(rcal);
  _latestFlags.rcalibUpdated = true;
  
}

////////////////////////////////////////////////////////////////
// decode ds flags

void IwrfMomReader::_decodeDsRadarFlags()

{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_FLAGS" << endl;
  }

  const DsRadarFlags &flags = _dsRadarMsg.getRadarFlags();

  RadxEvent event;
  event.setTime(flags.time, 0);
  event.setStartOfSweep(flags.startOfTilt);
  event.setEndOfSweep(flags.endOfTilt);
  event.setStartOfVolume(flags.startOfVolume);
  event.setEndOfVolume(flags.endOfVolume);
  event.setVolumeNumber(flags.volumeNum);
  event.setSweepNumber(flags.tiltNum);
  if (_savedRay != NULL) {
    event.setSweepMode(_savedRay->getSweepMode());
    event.setFollowMode(_savedRay->getFollowMode());
    event.setCurrentFixedAngleDeg(_savedRay->getFixedAngleDeg());
  }

  _latestEvents.push_back(event);

}


////////////////////////////////////////////////////////////////
// decode radar beam onto ray

void IwrfMomReader::_decodeDsRadarBeam()

{

  // input data

  const DsRadarBeam &rbeam = _dsRadarMsg.getRadarBeam();
  const DsRadarParams &rparams = _dsRadarMsg.getRadarParams();
  const vector<DsFieldParams *> &fparamsVec = _dsRadarMsg.getFieldParams();

  // set ray properties

  _latestRay->setTime(rbeam.dataTime, rbeam.nanoSecs);
  _latestRay->setVolumeNumber(rbeam.volumeNum);
  _latestRay->setSweepNumber(rbeam.tiltNum);

  if (rparams.scanTypeName.size() > 0) {
    _latestRay->setScanName(rparams.scanTypeName);
  }
  int scanMode = rparams.scanMode;
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }

  _latestRay->setSweepMode
    (IwrfMoments::getRadxSweepMode(scanMode));
  _latestRay->setPolarizationMode
    (IwrfMoments::getRadxPolarizationMode(rparams.polarization));
  _latestRay->setPrtMode
    (IwrfMoments::getRadxPrtMode(rparams.prfMode));
  _latestRay->setPrtRatio
    (IwrfMoments::getRadxPrtRatio(rparams.prfMode));
  _latestRay->setFollowMode
    (IwrfMoments::getRadxFollowMode(rparams.followMode));

  double elev = rbeam.elevation;
  if (elev > 180) {
    elev -= 360.0;
  }
  _latestRay->setElevationDeg(elev);

  double az = rbeam.azimuth;
  if (az < 0) {
    az += 360.0;
  }
  _latestRay->setAzimuthDeg(az);

  // range geometry

  int nGates = rparams.numGates;
  _latestRay->setRangeGeom(rparams.startRange, rparams.gateSpacing);

  if (scanMode == DS_RADAR_RHI_MODE ||
      scanMode == DS_RADAR_EL_SURV_MODE) {
    _latestRay->setFixedAngleDeg(rbeam.targetAz);
  } else {
    _latestRay->setFixedAngleDeg(rbeam.targetElev);
  }

  _latestRay->setIsIndexed(rbeam.beamIsIndexed);
  _latestRay->setAngleResDeg(rbeam.angularResolution);
  _latestRay->setAntennaTransition(rbeam.antennaTransition);
  _latestRay->setNSamples(rbeam.nSamples);
  
  _latestRay->setPulseWidthUsec(rparams.pulseWidth);
  double prt = 1.0 / rparams.pulseRepFreq;
  _latestRay->setPrtSec(prt);
  _latestRay->setNyquistMps(rparams.unambigVelocity);

  _latestRay->setUnambigRangeKm(Radx::missingMetaDouble);
  _latestRay->setUnambigRange();

  _latestRay->setMeasXmitPowerDbmH(rbeam.measXmitPowerDbmH);
  _latestRay->setMeasXmitPowerDbmV(rbeam.measXmitPowerDbmV);

  // platform georeference

  if (_dsContents & DsRadarMsg::PLATFORM_GEOREF) {
    const DsPlatformGeoref &platformGeoref = _dsRadarMsg.getPlatformGeoref();
    const ds_iwrf_platform_georef_t &dsGeoref = platformGeoref.getGeoref();
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
    georef.setTilt(dsGeoref.tilt_angle_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setRollRate(dsGeoref.roll_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    _latestRay->clearGeoref();
    _latestRay->setGeoref(georef);
  }

  // load up fields

  int byteWidth = rbeam.byteWidth;
  
  for (size_t iparam = 0; iparam < fparamsVec.size(); iparam++) {

    // is this an output field or censoring field?

    const DsFieldParams &fparams = *fparamsVec[iparam];
    string fieldName = fparams.name;

    // convert to floats
    
    Radx::fl32 *fdata = new Radx::fl32[nGates];

    if (byteWidth == 4) {

      fl32 *inData = (fl32 *) rbeam.data() + iparam;
      fl32 inMissing = (fl32) fparams.missingDataValue;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        fl32 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData;
        }
      } // igate

    } else if (byteWidth == 2) {

      ui16 *inData = (ui16 *) rbeam.data() + iparam;
      ui16 inMissing = (ui16) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui16 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + iparam;
      ui08 inMissing = (ui08) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui08 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } // if (byteWidth == 4)

    RadxField *field = new RadxField(fparams.name, fparams.units);
    field->copyRangeGeom(*_latestRay);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, fdata);

    _latestRay->addField(field);

    delete[] fdata;

  } // iparam

  _setEventFlags();
  _setRcalibIndex();
  _rayReady = true;

}

/////////////////////////////////////////////////////////////////////
// set the event flags by comparing the latest ray to the saved ray

void IwrfMomReader::_setEventFlags()

{

  if (_latestRay && _savedRay) {

    if (_latestRay->getVolumeNumber() != _savedRay->getVolumeNumber()) {
      _savedRay->setEndOfVolumeFlag(true);
      _latestRay->setStartOfVolumeFlag(true);
    }
    
    if (_latestRay->getSweepNumber() != _savedRay->getSweepNumber()) {
      _savedRay->setEndOfSweepFlag(true);
      _latestRay->setStartOfSweepFlag(true);
    }

  }

}

/////////////////////////////////////////////////////////////////////
// set index for calibration, based on pulse width

void IwrfMomReader::_setRcalibIndex()

{
  
  double rayPulseWidth = _latestRay->getPulseWidthUsec();

  size_t index = 0;
  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _rcalibs.size(); ii++) {
    double calibPulseWidth = _rcalibs[ii].getPulseWidthUsec();
    double diff = fabs(calibPulseWidth - rayPulseWidth);
    if (diff < minDiff) {
      index = ii;
      minDiff = diff;
    }
  }

  _latestRay->setCalibIndex(index);

}

/////////////////////////////////////////////////////////////////////
// handle input IWRF message
//
// returns 0 on success, -1 on failure

int IwrfMomReader::_handleIwrfMessage()

{
  return 0;
}

/////////////////////////////////////
// get next message
// no-op for abstract base class
// Returns 0 on success, -1 on failure

int IwrfMomReader::_getNextMsg()
{
  return 0;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from FILE
// Derived class

// REALTIME mode, read files as they arrive
// Specify input directory to watch.
//
// Blocks on read.
// Calls heartbeat_func when blocked, if non-null.

IwrfMomReaderFile::IwrfMomReaderFile(const char *input_dir,
                                     int max_realtime_age_secs,
                                     DsInput_heartbeat_t heartbeat_func,
                                     bool use_ldata_info) :
        IwrfMomReader()
        
{
  
  _input = new DsInputPath("IwrfMomReaderFile",
                           false,
                           input_dir,
                           max_realtime_age_secs,
                           heartbeat_func,
                           use_ldata_info);
  
  _rayIndex = 0;

}

// ARCHIVE mode - specify list of files to be read

IwrfMomReaderFile::IwrfMomReaderFile(const vector<string> &fileList) :
        IwrfMomReader(),
        _fileList(fileList)
        
{

  _input = new DsInputPath("IwrfMomReaderFile", false, _fileList);
  _rayIndex = 0;

}

//////////////////////////////////////////////////////////////////
// destructor

IwrfMomReaderFile::~IwrfMomReaderFile()

{

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////////////////////
// set debug

void IwrfMomReaderFile::setDebug(IwrfDebug_t debug)

{
  
  _debug = debug;
  
  if (_debug >= IWRF_DEBUG_VERBOSE) {
    _input->setDebug(true);
  } else {
    _input->setDebug(false);
  }

}

///////////////////////////////////////////////////////
// Read next ray - override base class to read from file
// Returns RadxRay pointer on success, NULL on failure.
//
// Caller is responsible for freeing the ray.
//
// Call the following for supplementary info on the ray:
//   getPlatform()
//   getScan()
//   getRCalibs()
//   getEvents()
//   getStatusXml()
  
RadxRay *IwrfMomReaderFile::readNextRay()

{

  _events = _savedEvents; // copy over any saved events
  _savedEvents.clear();
  
  // check the next ray is available in the current file
  
  if (_rayIndex >= _vol.getRays().size()) {
    // need new file - flags will be set
    if (_readNextFile()) {
      return NULL;
    }
    _rayIndex = 0;
  } else {
    // clear flags, since the metadata is only updated
    // for a new file
    _flags.clear();
  }
  
  const RadxRay &thisRay = *_vol.getRays()[_rayIndex];
  RadxEvent event;
  event.setFromRayFlags(thisRay);
  
  if (_rayIndex == 0) {
    // start of volume
    _events.push_back(event);
  } else if (_rayIndex == _vol.getRays().size() - 1) {
    // end of volume - save for next ray
    _savedEvents.push_back(event);
  } else if (thisRay.getStartOfSweepFlag()) {
    // start of sweep
    _events.push_back(event);
  } else if (thisRay.getEndOfSweepFlag()) {
    // end of sweep - save for next ray
    _savedEvents.push_back(event);
  }

  RadxRay *ray = new RadxRay(thisRay);
  _rayIndex++;
  
  return ray;

}

///////////////////////////////
// read in next available file
//
// Returns 0 on success, -1 on failure

int IwrfMomReaderFile::_readNextFile()

{
  
  PMU_auto_register("Reading next file");
  
  while (true) {

    const char *inputPath = _input->next();
    if (inputPath == NULL) {
      // no more files
      return -1;
    }
    _inputPath = inputPath;
    
    if (_debug) {
      cerr << "Reading file: " << _inputPath << endl;
    }
    
    // set up file object for reading
    
    RadxFile file;
    if (_debug >= IWRF_DEBUG_VERBOSE) {
      file.setDebug(true);
    }
    if (_debug >= IWRF_DEBUG_EXTRA) {
      file.setVerbose(true);
    }
    
    // read in file
    
    _vol.clear();
    if (file.readFromPath(_inputPath, _vol)) {
      cerr << "ERROR - IwrfMomReaderFile::_readNextFile()" << endl;
      cerr << "  reading next file: " << _inputPath << endl;
      cerr << file.getErrStr() << endl;
      continue;
    }

    if (_vol.getRays().size() < 1) {
      cerr << "WARNING - IwrfMomReaderFile::_readNextFile()" << endl;
      cerr << "  no rays in file: " << _inputPath << endl;
      continue;
    }

    break;;

  } // while

  // set the metadata

  _platform = _vol.getPlatform();
  _statusXml = _vol.getStatusXml();
  _rcalibs.clear();
  for (size_t ii = 0; ii < _vol.getRcalibs().size(); ii++) {
    _rcalibs.push_back(*_vol.getRcalibs()[ii]);
  }

  _flags.clear();
  _flags.platformUpdated = true;
  _flags.statusXmlUpdated = true;
  _flags.rcalibUpdated = true;

  // clear events

  _events.clear();

  return 0;

}

//////////////////////////////////////////////////////////////////
// seek to start of fmq

void IwrfMomReaderFile::seekToStart()

{
  if (_input) {
    _input->reset();
  }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from FMQ
// Derived class

IwrfMomReaderFmq::IwrfMomReaderFmq(const char *input_fmq,
                                   bool position_at_start /*=false*/) :
        IwrfMomReader(),
        _inputFmq(input_fmq),
        _positionFmqAtStart(position_at_start)
        
{
  _nParts = 0;
  _pos = 0;
  _fmqIsOpen = false;
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfMomReaderFmq::~IwrfMomReaderFmq()

{

}

/////////////////////////////////////
// get next message
//
// Returns 0 on success, -1 on failure

int IwrfMomReaderFmq::_getNextMsg()
  
{

  PMU_auto_register("Get next msg");
  
  // check we have an open FMQ
  
  if (!_fmqIsOpen) {
    
    // initialize FMQ
    
    Fmq::openPosition initPos = Fmq::END;
    if (_positionFmqAtStart) {
      initPos = Fmq::START;
    }
    _fmq.setHeartbeat(PMU_auto_register);
    
    int iret = 0;
    if (_nonBlocking) {
      iret = _fmq.initReadOnly(_inputFmq.c_str(),
                               "IwrfMomReader",
                               _debug >= IWRF_DEBUG_NORM,
                               initPos, 
                               _msecsNonblockingWait);
    } else {
      iret = _fmq.initReadBlocking(_inputFmq.c_str(),
                                   "IwrfMomReader",
                                   _debug >= IWRF_DEBUG_NORM,
                                   initPos);
      if (_msecsBlockingTimeout > 0) {
        _fmq.setBlockingReadTimeout(_msecsBlockingTimeout);
      }
    }
    
    if (iret) {
      cerr << "ERROR - IwrfMomReaderFmq::_getNextMsg" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _inputFmq << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;
    
  } // if

  // read a new message
  // blocking read registers with Procmap while waiting

  if (_nonBlocking) {
    bool gotOne = false;
    _timedOut = false;
    if (_fmq.readMsg(&gotOne, -1, _msecsNonblockingWait)) {
      cerr << "ERROR - IwrfMomReaderFmq::_getNextMsg" << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      _fmqIsOpen = false;
      return -1;
    }
    if (!gotOne) {
      _timedOut = true;
      return -1;
    }
  } else {
    if (_fmq.readMsgBlocking()) {
      cerr << "ERROR - IwrfMomReaderFmq::_getNextMsg" << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      _fmqIsOpen = false;
      return -1;
    }
  }

  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "--->> Got FMQ message" << endl;
  }

  _msgBuf.reset();
  _msgBuf.add(_fmq.getMsg(), _fmq.getMsgLen());
  _msgId = 0;

  return 0;

}

//////////////////////////////////////////////////////////////////
// seek to start of FMQ

void IwrfMomReaderFmq::seekToStart()

{
  _fmq.seek(Fmq::FMQ_SEEK_START);
}

//////////////////////////////////////////////////////////////////
// seek to end of FMQ

void IwrfMomReaderFmq::seekToEnd()

{
  _fmq.seek(Fmq::FMQ_SEEK_END);
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from TCP socket
// Derived class

IwrfMomReaderTcp::IwrfMomReaderTcp(const char *server_host,
                                   int server_port) :
        IwrfMomReader(),
        _serverHost(server_host),
        _serverPort(server_port)
        
{

  char serverDetails[1024];
  sprintf(serverDetails, "%s:%d", server_host, server_port);
  _serverDetails = serverDetails;

}

//////////////////////////////////////////////////////////////////
// destructor

IwrfMomReaderTcp::~IwrfMomReaderTcp()

{
  _sock.close();
}

//////////////////////////////////////////
// open the socket to the server
// Returns 0 on success, -1 on failure

int IwrfMomReaderTcp::_openSocket()
  
{

  if (_sock.isOpen()) {
    return 0;
  }

  while (true) {

    PMU_auto_register("Connecting to socket");

    if (_sock.open(_serverHost.c_str(), _serverPort, 5000) == 0) {
      return 0;
    }

    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      cerr << "ERROR - IwrfMomReaderTcp::_openSocket()" << endl;
      cerr << "     host: " << _serverHost << endl;
      cerr << "     port: " << _serverPort << endl;
    } else {
      cerr << "ERROR - IwrfMomReaderTcp::_openSocket()" << endl;
      cerr << "  Connecting to server" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
    cerr << "  Waiting for server to come up ..." << endl;
    umsleep(2000);

  } // while

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int IwrfMomReaderTcp::_getNextMsg()

{

  if (_openSocket()) {
    return -1;
  }

  bool have_good_header = false;
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];

  do {

    PMU_auto_register("Reading data");
    
    // read the first 8 bytes (id, len)

    if (_nonBlocking) {
      _timedOut = false;
      if (_sock.readBuffer(packetTop, sizeof(packetTop),
                           _msecsNonblockingWait)) {
        if (_sock.getErrNum() == SockUtil::TIMED_OUT) {
          _timedOut = true;
          return -1;
        }
        cerr << "ERROR - IwrfMomReaderTcp::_getNextMsg" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
        return -1;
      }
    } else {
      if (_sock.readBufferHb(packetTop, sizeof(packetTop),
                             sizeof(packetTop), PMU_auto_register, 1000)) {
        cerr << "ERROR - IwrfMomReaderTcp::_getNextMsg" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
        return -1;
      }
    }

    // check ID for packet, and get its length
    packetId = packetTop[0];
    packetLen = packetTop[1];

    if (iwrf_check_packet_id(packetId, packetLen)) {
      // read bytes to re-synchronize data stream
      if (_reSync()) {
        cerr << "ERROR - IwrfMomReaderTcp::_getNextMsg" << endl;
        cerr << " Cannot re-sync incoming data stream from socket";
        cerr << endl;
        return -1;
      }
    } else {
      have_good_header = true;
    }

  } while (!have_good_header);
    
  // make the buffer large enough

  _msgBuf.reserve(packetLen);
  _msgId = packetId;

  // copy the packet top into the start of the buffer

  memcpy(_msgBuf.getPtr(), packetTop, sizeof(packetTop));
  
  // read in the remainder of the buffer

  char *startPtr = (char *) _msgBuf.getPtr() + sizeof(packetTop);
  int nBytesLeft = packetLen - sizeof(packetTop);

  if (_sock.readBufferHb(startPtr, nBytesLeft, 1024, 
                         PMU_auto_register, 1000)) {
    cerr << "ERROR - IwrfMomReaderTcp::_getNextMsg" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int IwrfMomReaderTcp::_reSync()
  
{
  int sync_count = 0;

  if (_debug) {
    cerr << "Trying to resync ....." << endl;
  }
  
  unsigned int  check[2];

  while (true) {
    
    // peek at the next 8 bytes
    
    if (_peekAtBuffer(check, sizeof(check))) {
      cerr << "ERROR - IwrfMomReaderTcp::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }

    if((check[0] == IWRF_RADAR_INFO_ID &&
        check[1] == sizeof(iwrf_radar_info_t)) ||
       (check[0] == IWRF_XMIT_SAMPLE_ID &&
        check[1] == sizeof(iwrf_xmit_sample_t))) {
      return 0; // We've found a legitimate IWRF packet header
    } 

    // Search for the sync packet 
    if (check[0] == IWRF_SYNC_VAL_00 && check[1] == IWRF_SYNC_VAL_01) {
      // These are "sync packet" bytes read the 8 sync bytes and move on
      if (_debug) {
	cerr << "Found sync packet, back in sync" << endl;
      }
      if (_sock.readBufferHb(check, sizeof(check), sizeof(check),
                             PMU_auto_register, 1000)) {
	cerr << "ERROR - IwrfMomReader::_reSync" << endl;
	cerr << "  " << _sock.getErrStr() << endl;
	return -1;
      }
      return 0;
    }
    
    // no sync yet, read 1 byte and start again

    char byteVal;
    if (_sock.readBufferHb(&byteVal, 1, 1, PMU_auto_register, 1000)) {
      cerr << "ERROR - IwrfMomReader::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
    sync_count++;

  } // while

  return -1;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure

int IwrfMomReaderTcp::_peekAtBuffer(void *buf, int nbytes)

{

  int count = 0;

  while (true) {
    PMU_auto_register("peekAtBuffer");
    if (_sock.peek(buf, nbytes, 1000) == 0) {
      return 0;
    } else {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
	count++;
        continue;
      }
      cerr << "ERROR - IwrfMomReader::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  return -1;

}

