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
// RadarConst.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// RadarConst compute radar cal info
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <cmath>
#include "RadarConst.hh"

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

using namespace std;

// Constructor

RadarConst::RadarConst(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "RadarConst";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  return;
  
}

// destructor

RadarConst::~RadarConst()

{

}

//////////////////////////////////////////////////
// Run

int RadarConst::Run ()
{

  if (_args.debug) {
    cerr << "Running RadarConst - debug mode" << endl;
  }

  double _piCubed = pow(M_PI, 3.0);
  double _lightSpeed = 299792458.0;
  double _kSquared = _args.kSquaredWater;

  double antGainLinear = pow(10.0, _args.antGainDb / 10.0);
  double gainSquared = antGainLinear * antGainLinear;
  double _wavelengthM = _args.wavelengthCm / 100.0;
  double lambdaSquared = _wavelengthM * _wavelengthM;
  double pulseMeters = _args.pulseWidthUs * 1.0e-6 * _lightSpeed;
  
  double hBeamWidthRad = _args.horizBeamWidthDeg * DEG_TO_RAD;
  double vBeamWidthRad = _args.vertBeamWidthDeg * DEG_TO_RAD;

  double peakPowerMilliW = pow(10.0, _args.peakPowerDbm / 10.0);

  double theoreticalG = (M_PI * M_PI) / (hBeamWidthRad * vBeamWidthRad);
  double theoreticalGdB = 10.0 * log10(theoreticalG);

  cerr << "  wavelengthCm: " << _args.wavelengthCm << endl;
  cerr << "  horizBeamWidthDeg: " << _args.horizBeamWidthDeg << endl;
  cerr << "  vertBeamWidthDeg: " << _args.vertBeamWidthDeg << endl;
  cerr << "  antGainDb: " << _args.antGainDb << endl;
  cerr << "  theoretical antGainDb: " << theoreticalGdB << endl;
  cerr << "  peakPowerDbm: " << _args.peakPowerDbm<< endl;
  cerr << "  peakPowerW: " << peakPowerMilliW / 1000.0 << endl;
  cerr << "  pulseWidthUs: " << _args.pulseWidthUs << endl;
  cerr << "  2-way waveguideLoss: " << _args.twoWayWaveguideLoss << endl;
  cerr << "  2-way radomeLoss: " << _args.twoWayRadomeLoss << endl;
  cerr << "  receiverMismatchLoss: " << _args.receiverMismatchLoss << endl;
  cerr << "  antGainLinear: " << antGainLinear << endl;
  cerr << "  gainSquared: " << gainSquared << endl;
  cerr << "  lambdaSquared: " << lambdaSquared << endl;
  cerr << "  pulseMeters: " << pulseMeters << endl;
  cerr << "  hBeamWidthRad: " << hBeamWidthRad << endl;
  cerr << "  vBeamWidthRad: " << vBeamWidthRad << endl;
  cerr << "  peakPowerMilliW: " << peakPowerMilliW << endl;
  cerr << "  piCubed: " << _piCubed << endl;
  cerr << "  kSquared: " << _kSquared << endl;

  double denom = (peakPowerMilliW * _piCubed * pulseMeters * gainSquared *
                  hBeamWidthRad * vBeamWidthRad * _kSquared * 1.0e-24);

  double num = (1024.0 * log(2.0) * lambdaSquared);
  
  double factor = num / denom;
  
  cerr << "  num: " << num << endl;
  cerr << "  denom: " << denom << endl;
  cerr << "  factor: " << factor << endl;
  
  double radarConst = (10.0 * log10(factor)
                       + _args.twoWayWaveguideLoss
                       + _args.twoWayRadomeLoss
                       + _args.receiverMismatchLoss);
  
  cerr << "--->> RadarConst: " << radarConst << endl;

  return 0;

}

