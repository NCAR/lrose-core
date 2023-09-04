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
// Classes for reading in Ray data in thread
//
///////////////////////////////////////////////////////////////

#ifndef Reader_hh
#define Reader_hh

#include <deque>
#include <toolsa/TaThread.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxRay.hh>
#include <radar/IwrfMomReader.hh>
#include "Params.hh"
using namespace std;

////////////////////////////
// Generic reader base class

class Reader : public TaThread
{

public:

  Reader(const Params &params);
  virtual ~Reader();

  // get next ray
  // return NULL if no ray is available
  // also fills in platform object

  RadxRay *getNextRay(RadxPlatform &platform);

protected:

  // add a ray
  
  void _addRay(RadxRay *ray);
  
  // data members

  // QMutex _mutex;
  const Params &_params;
  
  RadxPlatform _platform;
  deque<RadxRay *> _rayQueue;
  int _maxQueueSize;

private:

};

////////////////////////////
// Simulated reader

class SimReader : public Reader
{

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
  
  double _latitude;
  double _longitude;
  double _altitude;

  void _runSimPpi();
  void _runSimVert();
  void _simulatePpiBeam(double elev, double az, int volNum, int sweepNum);
  void _simulateRhiBeam(double elev, double az, int volNum, int sweepNum);
  void _simulateVertBeam(double elev, double az, int volNum, int sweepNum);

};

////////////////////////////
// IWRF reader

class IwrfReader : public Reader
{

public:

  IwrfReader(const Params &params);
  virtual ~IwrfReader();

protected:

  // start running

  virtual void run();

private:

};

#endif

