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
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2020
//
///////////////////////////////////////////////////////////////
//
// Classes for reading in Radx rays in realtime
// or simulated realtime
//
///////////////////////////////////////////////////////////////

#include "Reader.hh"
#include "AllocCheck.hh"
#include "DisplayManager.hh"
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
// add a manager to which the rays will be delivered

void Reader::addManager(DisplayManager *manager)

{
  _managers.push_back(manager);
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

  // add a client to the ray so it is not prematurely deleted
  
  ray->addClient();
  
  // add ray to each display manager

  for (size_t ii = 0; ii < _managers.size(); ii++) {
    _managers[ii]->addRay(ray, _platform);
  }

  // remove the client from the ray and delete if needed

  RadxRay::deleteIfUnused(ray);

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
  _platform.setInstrumentName("APAR");
  _platform.setSiteName("C130");
  _platform.setLatitudeDeg(_latitude);
  _platform.setLongitudeDeg(_longitude);
  _platform.setAltitudeKm(_altitude);
  _platform.addWavelengthM(0.05);
  _platform.setRadarBeamWidthDegH(2.0);
  _platform.setRadarBeamWidthDegV(2.0);

}

// set fields

void SimReader::run()

{

  int sweepNum = 0;
  int volNum = 0;

  while (true) {
    
    for (int iscan = 0; iscan < _params.sim_scans_n; iscan++) {
      
      const Params::sim_scan_t &scan = _params._sim_scans[iscan];

      if (scan.sim_type == Params::RHI_SIM) {
        
        Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_RHI;
        
        for (double az = scan.min_az;
             az <= scan.max_az;
             az += scan.delta_az) {
          for (int istride = 0; istride < scan.stride; istride++) {
            for (double el = scan.min_el + istride * scan.delta_el;
                 el <= scan.max_el;
                 el += scan.stride * scan.delta_el) {
              _simulateBeam(el, az, volNum, sweepNum, sweepMode);
              umsleep(_params.sim_sleep_msecs);
            } // el
          } // istride
        } // az

      } else if (scan.sim_type == Params::PPI_SIM) {

        Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_SECTOR;
        
        for (double el = scan.min_el;
             el <= scan.max_el;
             el += scan.delta_el) {
          for (int istride = 0; istride < scan.stride; istride++) {
            for (double az = scan.min_az + istride * scan.delta_az;
                 az <= scan.max_az;
                 az += scan.stride * scan.delta_az) {
              _simulateBeam(el, az, volNum, sweepNum, sweepMode);
              umsleep(_params.sim_sleep_msecs);
            } // az
          } // istride
        } // el
        
      } // if (scan.sim_type == Params::RHI_SIM)

      sweepNum++;
      
    } // iscan

    volNum++;

  } // while

}

// simulate a beam

void SimReader::_simulateBeam(double elev, double az,
                              int volNum, int sweepNum,
                              Radx::SweepMode_t sweepMode)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Sim beam: elev, az, volNum, sweepNum, sweepMode: "
         << elev << ", "
         << az << ", "
         << volNum << ", "
         << sweepNum << ", "
         << Radx::sweepModeToStr(sweepMode) << endl;
  }
  
  RadxRay *ray = new RadxRay;
  ray->setVolumeNumber(volNum);
  ray->setSweepNumber(sweepNum);
  ray->setSweepMode(sweepMode);
  ray->setPolarizationMode(Radx::POL_MODE_HV_ALT);
  ray->setPrtMode(Radx::PRT_MODE_FIXED);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  ray->setTime(tv.tv_sec, tv.tv_usec * 1000);
  double dsecs = ray->getTimeDouble();
  double frac = fmod(dsecs, 30.0) / 30.0;

  ray->setAzimuthDeg(az);
  ray->setElevationDeg(elev);
  if (sweepMode == Radx::SWEEP_MODE_RHI) {
    ray->setFixedAngleDeg(az);
  } else {
    ray->setFixedAngleDeg(elev);
  }
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
    double dataMin = field.minVal + dataRange * frac;
    double dataDelta = dataRange / nGates;

    for (int igate = 0; igate < nGates; igate++) {
      data[igate] = dataMin + igate * dataDelta;
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

  IwrfMomReader *reader = NULL;

  if (_params.input_mode == Params::FMQ_INPUT) {
    reader = new IwrfMomReaderFmq(_params.input_fmq_url,
                                  _params.seek_to_start_of_fmq);
  } else if (_params.input_mode == Params::TCP_INPUT) {
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
