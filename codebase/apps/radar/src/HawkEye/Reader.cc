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
#include "AllocCheck.hh"
#include <cmath>
#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/mem.h>
#include <Radx/RadxGeoref.hh>
using namespace std;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// Base class Reader
 
Reader::Reader(const Params &params) :
        _params(params)

{
  _maxQueueSize = _params.beam_queue_size;
}

Reader::~Reader() 
{
}

///////////////////////////////////////////////////////
// get next ray
// return NULL if no ray is available
// also fills in platform object

RadxRay *Reader::getNextRay(RadxPlatform &platform)

{

  TaThread::LockForScope locker;

  if (_rayQueue.size() == 0) {
    return NULL;
  }

  RadxRay *ray = _rayQueue.back();
  _rayQueue.pop_back();
  platform = _platform;
  
  return ray;

}

///////////////////////////////////////////////////////
// add ray

void Reader::_addRay(RadxRay *ray)

{

  TaThread::LockForScope locker;

  // keep the queue below max size
  
  if ((int) _rayQueue.size() >= _maxQueueSize) {
    // pop the oldest ray
    RadxRay *ray = _rayQueue.back();
    delete ray;
    AllocCheck::inst().addFree();
    _rayQueue.pop_back();
  }

  _rayQueue.push_front(ray);
  AllocCheck::inst().addAlloc();

}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// Simlated Reader
 
SimReader::SimReader(const Params &params) :
        Reader(params)
        
{

  _latitude = 40.0;
  _longitude = -105.0;
  _altitude = 1.6;

  TaThread::LockForScope locker;
  _platform.setInstrumentName("SPOL");
  _platform.setSiteName("Marshall");
  _platform.setLatitudeDeg(_latitude);
  _platform.setLongitudeDeg(_longitude);
  _platform.setAltitudeKm(_altitude);
  _platform.addWavelengthM(0.10);
  _platform.setRadarBeamWidthDegH(1.0);
  _platform.setRadarBeamWidthDegV(1.0);

}

// set fields

void SimReader::run()

{

  if (_params.display_mode == Params::POLAR_DISPLAY) {
    _runSimPpi();
  } else {
    _runSimVert();
  }

}

// simulate in ppi mode

void SimReader::_runSimPpi()
  
{

  while (true) {

    // PPI

    double az = 0.0;
    double elev = 1.0;
    int sweepNum = 0;
    int volNum = 0;
    
    while (true) {
      _simulatePpiBeam(elev, az, volNum, sweepNum);
      umsleep(_params.sim_sleep_msecs);
      az += 1.0;
      if (az > 359.5) {
        az = 0.0;
        sweepNum++;
        elev += 2.0;
      }
      if (elev > 20) {
        volNum++;
        break;
      }
    } // while

    // RHI

    az = 0.0;
    elev = 1.0;
    sweepNum = 0;
    double maxElev = 89.5;
    if (_params.rhi_display_180_degrees) {
      maxElev = 179.5;
    }

    double increment = 1.0;
    while (true) {
      _simulateRhiBeam(elev, az, volNum, sweepNum);
      umsleep(_params.sim_sleep_msecs);
      elev += increment;
      if (elev > maxElev) {
        increment = -1.0;
        az += 30.0;
      } else if (elev < 0.5) {
        increment = 1.0;
        az += 13.0;
      }
      if (az > 359.5) {
        volNum++;
        break;
      }
    } // while

  } // while

}

// simulate in vert point mode

void SimReader::_runSimVert()

{
  
  double az = 0.0;
  double elev = 89.0;
  int sweepNum = 0;
  int volNum = 0;
  
  while (true) {

    _simulateVertBeam(elev, az, volNum, sweepNum);
    
    umsleep(_params.sim_sleep_msecs);
    
    az += 1.0;
    if (az > 359.5) {
      az = 0.0;
      sweepNum++;
      elev += 0.1;
      if (elev > 90) {
        elev = 89.0;
        sweepNum = 0;
        volNum++;
      }
    }

  }

}

// simulate a ppi beam

void SimReader::_simulatePpiBeam(double elev, double az,
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
  ray->setIsIndexed(false);
  ray->setAngleResDeg(1.0);
  ray->setNSamples(128);
  ray->setPulseWidthUsec(1.0);
  ray->setPrtSec(0.001);
  ray->setNyquistMps(25.0);
  ray->setUnambigRangeKm(150.0);
  ray->setMeasXmitPowerDbmH(84.0);
  ray->setMeasXmitPowerDbmV(84.1);

  int nGates = _params.sim_n_gates;
  double startRange = _params.sim_start_range_km;
  double gateSpacing = _params.sim_gate_spacing_km;

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
      // data[igate] = dataMin + igate * dataDelta + ifield * 2.0 + az * 0.01;
      data[igate] = dataMin + igate * dataDelta + ifield * 2.0;
    }

    ray->addField(field.name, field.units, nGates,
                  missing, data, true);

    delete[] data;

  } // ifield

  // add ray to queue

  _addRay(ray);

}

/////////////////////////
// simulate an RHI beam

void SimReader::_simulateRhiBeam(double elev, double az,
                                 int volNum, int sweepNum)
  
{

  RadxRay *ray = new RadxRay;
  ray->setVolumeNumber(volNum);
  ray->setSweepNumber(sweepNum);
  ray->setSweepMode(Radx::SWEEP_MODE_RHI);
  ray->setPolarizationMode(Radx::POL_MODE_HV_ALT);
  ray->setPrtMode(Radx::PRT_MODE_FIXED);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  ray->setTime(tv.tv_sec, tv.tv_usec * 1000);

  ray->setAzimuthDeg(az);
  ray->setElevationDeg(elev);
  ray->setFixedAngleDeg(az);
  ray->setIsIndexed(true);
  ray->setAngleResDeg(1.0);
  ray->setNSamples(128);
  ray->setPulseWidthUsec(1.0);
  ray->setPrtSec(0.001);
  ray->setNyquistMps(25.0);
  ray->setUnambigRangeKm(150.0);
  ray->setMeasXmitPowerDbmH(84.0);
  ray->setMeasXmitPowerDbmV(84.1);

  int nGates = _params.sim_n_gates;
  double startRange = _params.sim_start_range_km;
  double gateSpacing = _params.sim_gate_spacing_km;

  ray->setNGates(nGates);
  ray->setRangeGeom(startRange, gateSpacing);

  Radx::fl32 missing = -9999.0;

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    const Field &field = _fields[ifield];
    Radx::fl32 *data = new Radx::fl32[nGates];

    double dataRange = (field.maxVal - field.minVal) / 2.0;
    double dataMin = field.minVal + (dataRange / 720.0) * az;
    double dataDelta = dataRange / nGates;

    for (int igate = 0; igate < nGates; igate++) {
      data[igate] = dataMin + igate * dataDelta + ifield * 2.0;
    }

    ray->addField(field.name, field.units, nGates,
                  missing, data, true);

    delete[] data;

  } // ifield

  // add ray to queue

  _addRay(ray);

}

// simulate a vert pointing beam

void SimReader::_simulateVertBeam(double elev, double az,
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
  ray->setIsIndexed(false);
  ray->setNSamples(128);
  ray->setPulseWidthUsec(1.0);
  ray->setPrtSec(0.001);
  ray->setNyquistMps(25.0);
  ray->setUnambigRangeKm(150.0);
  ray->setMeasXmitPowerDbmH(84.0);
  ray->setMeasXmitPowerDbmV(84.1);

  double modSecs = (tv.tv_sec % 60) + tv.tv_usec / 1.0e6;
  _altitude = sin((modSecs / 60.0) * 2.0 * M_PI) * 10.0 + 12.0;
  _latitude += 0.001;
  if (_latitude > 90) {
    _latitude = 0.0;
  }
  _longitude += 0.001;
  if (_longitude > 180) {
    _longitude = 0.0;
  }
  
  if (_altitude > 10.0) {
    ray->setElevationDeg(-elev);
  }

  RadxGeoref georef;
  georef.setTimeSecs(ray->getTimeSecs());
  georef.setNanoSecs(ray->getNanoSecs());
  georef.setLongitude(_longitude);
  georef.setLatitude(_latitude);
  georef.setAltitudeKmMsl(_altitude);
  ray->setGeoref(georef);
  
  int nGates = _params.sim_n_gates;
  double startRange = _params.sim_start_range_km;
  double gateSpacing = _params.sim_gate_spacing_km;

  ray->setNGates(nGates);
  ray->setRangeGeom(startRange, gateSpacing);

  Radx::fl32 missing = -9999.0;

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    const Field &field = _fields[ifield];
    Radx::fl32 *data = new Radx::fl32[nGates];

    double dataRange = (field.maxVal - field.minVal) / 2.0;
    double dataMin = field.minVal + (dataRange / 200.0) * elev;
    double dataDelta = dataRange / nGates;

    for (int igate = 0; igate < nGates; igate++) {
      data[igate] = dataMin + igate * dataDelta + ifield * 2.0 + az * 0.01;
    }

    ray->addField(field.name, field.units, nGates,
                  missing, data, true);

    delete[] data;

  } // ifield

  // add ray to queue

  _addRay(ray);

}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/// IWRF Reader
 
IwrfReader::IwrfReader(const Params &params) :
        Reader(params)
{
}

IwrfReader::~IwrfReader() 
{
}

///////////////////////////////
// run, reading data

void IwrfReader::run()

{

  // instatiate reader object

  IwrfMomReader *reader;

  if (_params.input_mode == Params::IWRF_FMQ_INPUT ||
      _params.input_mode == Params::DSR_FMQ_INPUT) {
    reader = new IwrfMomReaderFmq(_params.input_fmq_url,
                                  _params.seek_to_start_of_fmq);
  } else if (_params.input_mode == Params::IWRF_TCP_INPUT) {
    reader = new IwrfMomReaderTcp(_params.input_tcp_host,
                                  _params.input_tcp_port);
  } else {
    cerr << "ERROR - IwrfReader::run" << endl;
    cerr << "  incorrect input_mode: " << _params.input_mode << endl;
    assert(false);
  }

  // get data

  int count = 0;
  while (true) {

    // get new ray

    try {

      RadxRay *ray = reader->readNextRay();

      if (ray == NULL) {
        continue;
      }
      
      // add ray to queue
      
      if (ray) {
        {
          TaThread::LockForScope locker;
          _platform = reader->getPlatform();
        }
        _addRay(ray);
      } else {
        cerr << "ERROR - IwrfReader::run" << endl;
        cerr << "  Cannot read ray" << endl;
        return;
      }
      
    } catch (std::bad_alloc &a) {
      cerr << "==>> IwrfReader::run() - bad alloc: " << a.what() << endl;
    }

    count++;
    
  }
  
}
