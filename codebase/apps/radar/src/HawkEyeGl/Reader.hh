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
// Reader.hh
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

#ifndef Reader_hh
#define Reader_hh

#include <deque>
#include <QThread>
#include <QMutex>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include "Params.hh"
using namespace std;

////////////////////////////
// Generic reader base class

class Reader : public QThread
{

  Q_OBJECT

public:

  Reader(const Params &params);

  // get next ray
  // return NULL if no ray is available
  // also fills in vol object

  RadxRay *getNextRay(RadxVol &vol);

protected:

  // start running

  virtual void run() = 0;

  // add a ray

  void addRay(RadxRay *ray);

  // data members

  QMutex _mutex;
  const Params &_params;
  
  RadxVol _vol;
  deque<RadxRay *> _rayQueue;
  int _maxQueueSize;

private:

};

////////////////////////////
// Simulated reader

class SimReader : public Reader
{

  Q_OBJECT

public:

  SimReader(const Params &params);

  class Field {
  public:
    string name;
    string units;
    double minVal;
    double maxVal;
  };

  void setFields(const vector<Field> fields) { _fields = fields; }

protected:

  // start running

  virtual void run();

private:

  vector<Field> _fields;
  void _simulateBeam(double elev, double az, int volNum, int sweepNum);

};

////////////////////////////
// Dsr FMQ reader

class DsrFmqReader : public Reader
{

  Q_OBJECT

public:

  DsrFmqReader(const Params &params);

protected:

  // start running

  virtual void run();

private:

  void _processBeam(DsRadarMsg &msg);
  void _loadVolumeParams(const DsRadarMsg &radarMsg);

  Radx::SweepMode_t _getRadxSweepMode(int dsrScanMode);
  Radx::PolarizationMode_t _getRadxPolarizationMode(int dsrPolMode);
  Radx::FollowMode_t _getRadxFollowMode(int dsrMode);
  Radx::PrtMode_t _getRadxPrtMode(int dsrMode);

};

#endif

