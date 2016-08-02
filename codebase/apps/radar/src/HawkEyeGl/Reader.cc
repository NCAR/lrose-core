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
/////////////////////////////////////////////////////////////
// Readers.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2010
//
///////////////////////////////////////////////////////////////
//
// Classes for reading in Beam data in thread
//
///////////////////////////////////////////////////////////////

#include "Reader.hh"
#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/mem.h>
#include <QMutexLocker>
using namespace std;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// Base class Reader
 
Reader::Reader(const Params &params) :
        _params(params)

{
  _maxQueueSize = _params.beam_queue_size;
}

///////////////////////////////////////////////////////
// get next ray
// return NULL if no ray is available
// also fills in vol object

RadxRay *Reader::getNextRay(RadxVol &vol)

{

  QMutexLocker locker(&_mutex);

  if (_rayQueue.size() == 0) {
    return NULL;
  }

  RadxRay *ray = _rayQueue.back();
  _rayQueue.pop_back();

  vol = _vol;

  return ray;

}

///////////////////////////////////////////////////////
// add ray

void Reader::addRay(RadxRay *ray)

{

  QMutexLocker locker(&_mutex);

  if ((int) _rayQueue.size() >= _maxQueueSize) {
    RadxRay *ray = _rayQueue.back();
    delete ray;
    _rayQueue.pop_back();
  }

  _rayQueue.push_front(ray);

}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// Simlated Reader
 
SimReader::SimReader(const Params &params) :
        Reader(params)

{

  _vol.setInstrumentName("SPOL");
  _vol.setSiteName("Marshall");
  _vol.setLatitudeDeg(40.0);
  _vol.setLongitudeDeg(-105.0);
  _vol.setAltitudeKm(1.6);
  _vol.addWavelengthM(0.10);
  _vol.setRadarBeamWidthDegH(1.0);
  _vol.setRadarBeamWidthDegV(1.0);

}

// set fields

void SimReader::run()

{
  
  double az = 0.0;
  double elev = 1.0;
  int sweepNum = 0;
  int volNum = 0;
  
  while (true) {
    _simulateBeam(elev, az, volNum, sweepNum);
    umsleep(20);
    az += 1.0;
    if (az > 359.5) {
      az = 0.0;
      sweepNum++;
      elev += 1.0;
      if (elev > 10.5) {
        elev = 1.0;
        sweepNum = 0;
        volNum++;
      }
    }
  }

}

/////////////////////
// simulate a beam

void SimReader::_simulateBeam(double elev, double az,
                              int volNum, int sweepNum)

{

  RadxRay *ray = new RadxRay;
  ray->setVolumeNumber(volNum);
  ray->setSweepNumber(sweepNum);
  ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  ray->setPolarizationMode(Radx::POL_MODE_HV_ALT);
  ray->setPrtMode(Radx::PRT_MODE_FIXED);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  ray->setTime(tv.tv_sec, tv.tv_usec * 1000);

  ray->setAzimuthDeg(az);
  ray->setElevationDeg(elev);
  ray->setFixedAngleDeg(elev);
  ray->setIsIndexed(true);
  ray->setAngleResDeg(1.0);
  ray->setNSamples(128);
  ray->setPulseWidthUsec(1.0);
  ray->setPrtSec(0.001);
  ray->setNyquistMps(25.0);
  ray->setUnambigRangeKm(150.0);
  ray->setMeasXmitPowerDbmH(84.0);
  ray->setMeasXmitPowerDbmV(84.1);

  int nGates = 1000;
  double startRange = 0.075;
  double gateSpacing = 0.150;

  ray->setNGates(nGates);
  ray->setRangeGeom(startRange, gateSpacing);

  Radx::fl32 missing = -9999.0;

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    const Field &field = _fields[ifield];
    Radx::fl32 *data = new Radx::fl32[nGates];

    double dataRange = (field.maxVal - field.minVal) / 2.0;
    double dataMin = field.minVal + (dataRange / 20.0) * elev;
    double dataDelta = dataRange / nGates;

    for (int igate = 0; igate < nGates; igate++) {
      data[igate] = dataMin + igate * dataDelta + ifield * 2.0;
    }

    ray->addField(field.name, field.units, nGates,
                  missing, data, true);

    delete[] data;

  } // ifield

  // add ray to queue

  addRay(ray);

}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// DSR FMQ Reader
 
DsrFmqReader::DsrFmqReader(const Params &params) :
        Reader(params)
        
{
  
}

///////////////////////////////
// run, reading data from FMQ

void DsrFmqReader::run()

{
  
  // Instantiate and initialize the DsRadar queue
  
  DsRadarQueue radarQueue;

  DsFmq::openPosition openPos = DsFmq::START;
  if (_params.seek_to_end_of_fmq) {
    openPos = DsFmq::END;
  }

  if (radarQueue.init(_params.input_fmq_url, "HawkEyeGl::DsFmqReader",
                      _params.debug,
                      DsFmq::BLOCKING_READ_WRITE, openPos)) {
    cerr << "ERROR - HawkEyeGl::DsFmqReader::run" << endl;
    cerr << "  Cannot initialize radar queue: "
         << _params.input_fmq_url << endl;
    return;
  }

  // read messages from queue

  DsRadarMsg radarMsg;
  while (true) {

    bool gotMsg = false;
    int contents;
    if (radarQueue.getDsMsg(radarMsg, &contents, &gotMsg) || !gotMsg) {
      umsleep(20);
      continue;
    }

    if ((contents & DsRadarMsg::RADAR_BEAM) && radarMsg.allParamsSet()) {
      _processBeam(radarMsg);
    }

  }

}

//////////////////////////
// process a beam

void DsrFmqReader::_processBeam(DsRadarMsg &msg)

{

  const DsRadarBeam &rbeam = msg.getRadarBeam();
  const DsRadarParams &rparams = msg.getRadarParams();
  vector<DsFieldParams *> &fparams = msg.getFieldParams();
  const DsRadarBeam &beam = msg.getRadarBeam();

  // convert the field data to floats
  // order changes from gate-by-gate to field-by-field
  
  int byteWidth = rbeam.byteWidth;
  int nGates = rparams.numGates;
  int nFields = rparams.numFields;
  fl32 fmiss = -9999.0f;
  fl32 **fieldData = (fl32 **) ucalloc2(nFields, nGates, sizeof(fl32));
  
  for (int ifield = 0; ifield < nFields; ifield++) {

    const DsFieldParams &field = *fparams[ifield];
    int imiss = field.missingDataValue;
    double scale = field.scale;
    double bias = field.bias;
    
    if (byteWidth == 4) {

      fl32 *inData = (fl32 *) rbeam.data() + ifield;
      fl32 *fldData = (fl32 *) fieldData[ifield];
      for (int igate = 0; igate < nGates;
           igate++, inData += nFields, fldData++) {
        fl32 inVal = *inData;
        if (inVal == (fl32) imiss) {
          *fldData = fmiss;
        } else {
          *fldData = inVal;
        }
      } // igate

    } else if (byteWidth == 2) {

      ui16 *inData = (ui16 *) rbeam.data() + ifield;
      fl32 *fldData = (fl32 *) fieldData[ifield];
      for (int igate = 0; igate < nGates;
           igate++, inData += nFields, fldData++) {
        ui16 inVal = *inData;
        if (inVal == imiss) {
          *fldData = fmiss;
        } else {
          *fldData = inVal * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + ifield;
      fl32 *fldData = (fl32 *) fieldData[ifield];
      for (int igate = 0; igate < nGates;
           igate++, inData += nFields, fldData++) {
        ui08 inVal = *inData;
        if (inVal == imiss) {
          *fldData = fmiss;
        } else {
          *fldData = inVal * scale + bias;
        }
      } // igate

    }
    
  } // ifield

  // set volume info

  _loadVolumeParams(msg);

  // create new ray

  RadxRay *ray = new RadxRay;

  ray->setVolumeNumber(beam.volumeNum);
  ray->setSweepNumber(beam.tiltNum);
  ray->setNGates(rparams.numGates);
  ray->setRangeGeom(rparams.startRange, rparams.gateSpacing);
  
  if (rparams.scanMode == DS_RADAR_RHI_MODE ||
      rparams.scanMode == DS_RADAR_EL_SURV_MODE) {
    ray->setFixedAngleDeg(beam.targetAz);
  } else {
    ray->setFixedAngleDeg(beam.targetElev);
  }

  ray->setSweepMode(_getRadxSweepMode(rparams.scanMode));
  ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
  ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
  ray->setFollowMode(_getRadxFollowMode(rparams.followMode));
  
  ray->setTime(beam.dataTime, beam.nanoSecs);
  ray->setAzimuthDeg(beam.azimuth);
  ray->setElevationDeg(beam.elevation);
  
  ray->setIsIndexed(beam.beamIsIndexed);
  ray->setAngleResDeg(beam.angularResolution);
  ray->setAntennaTransition(beam.antennaTransition);
  ray->setNSamples(beam.nSamples);

  ray->setPulseWidthUsec(rparams.pulseWidth);
  ray->setPrtSec(rparams.prt);
  ray->setPrtRatio(rparams.prt2 / rparams.prt);
  ray->setNyquistMps(rparams.unambigVelocity);
  ray->setUnambigRangeKm(rparams.unambigRange);

  ray->setMeasXmitPowerDbmH(rparams.measXmitPowerDbmH);
  ray->setMeasXmitPowerDbmV(rparams.measXmitPowerDbmV);

  Radx::fl32 missing = fmiss;
  Radx::fl32 *rayData = new Radx::fl32[nGates];
  for (int ifield = 0; ifield < nFields; ifield++) {
    const DsFieldParams &field = *fparams[ifield];
    fl32 *fldData = (fl32 *) fieldData[ifield];
    memcpy(rayData, fldData, nGates * sizeof(fl32));
    ray->addField(field.name, field.units, nGates,
                  missing, rayData, true);
  } // ifield

  // clean up memory

  delete[] rayData;
  ufree2((void **) fieldData);

  // add ray to queue

  addRay(ray);

}

/////////////////////////////////////////////////////////////////
// radar the volume params

void DsrFmqReader::_loadVolumeParams(const DsRadarMsg &radarMsg)

{

  DsRadarParams *rparams = new DsRadarParams(radarMsg.getRadarParams());
  
  _vol.setInstrumentName(rparams->radarName);
  _vol.setSiteName("");
  _vol.setScanName(rparams->scanTypeName);

  switch (rparams->radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }

  _vol.setLatitudeDeg(rparams->latitude);
  _vol.setLongitudeDeg(rparams->longitude);
  _vol.setAltitudeKm(rparams->altitude);

  _vol.addWavelengthM(rparams->wavelength / 100.0);
  _vol.setRadarBeamWidthDegH(rparams->horizBeamWidth);
  _vol.setRadarBeamWidthDegV(rparams->vertBeamWidth);

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t DsrFmqReader::_getRadxSweepMode(int dsrScanMode)

{
  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t DsrFmqReader::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t DsrFmqReader::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t DsrFmqReader::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
      return Radx::PRT_MODE_STAGGERED;
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
      return Radx::PRT_MODE_STAGGERED;
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}
