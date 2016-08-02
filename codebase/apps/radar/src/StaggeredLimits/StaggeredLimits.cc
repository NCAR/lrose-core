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
// StaggeredLimits.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////
//
// StaggeredLimits computes folding range and velocity for 
// staggered PRT
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <ctime>
#include <cmath>
#include "StaggeredLimits.hh"
#include <radar/RadarMoments.hh>

using namespace std;

// Constructor

StaggeredLimits::StaggeredLimits(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "StaggeredLimits";

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

StaggeredLimits::~StaggeredLimits()

{

}

//////////////////////////////////////////////////
// Run

int StaggeredLimits::Run ()
{

  cerr << "  frequency (Hz): " << _args.frequencyHz << endl;
  double wavelengthM = Args::lightSpeed / _args.frequencyHz;
  cerr << "  wavelength (Cm): " << wavelengthM * 100.0 << endl;
  cerr << "  prt1 (sec): " << _args.prt1 << endl;
  cerr << "  prt2 (sec): " << _args.prt2 << endl;
  cerr << "  prf1 (/sec): " << 1.0 / _args.prt1 << endl;
  cerr << "  prf2 (/sec): " << 1.0 / _args.prt2 << endl;

  double maxRangeM = (_args.prt1 * _args.lightSpeed) / 2.0;
  cerr << "  maxRange (m): " << maxRangeM << endl;

  double prtRatio = _args.prt1 / _args.prt2;
  int ratio60 = (int) (prtRatio * 60.0 + 0.5);
  int stagM = 2;
  int stagN = 3;
  if (ratio60 == 40) {
    // 2/3
    stagM = 2;
    stagN = 3;
  } else if (ratio60 == 45) {
    // 3/4
    stagM = 3;
    stagN = 4;
  } else if (ratio60 == 48) {
    // 4/5
    stagM = 4;
    stagN = 5;
  } else {
    // assume 2/3
    cerr << "ERROR - StaggeredLimits" << endl;
    cerr << "  No support for prtRatio: " << prtRatio << endl;
    cerr << "  Support only for 2/3, 3/4 and 4/5" << endl;
    cerr << "  prt1 is short, prt2 is long" << endl;
    return -1;
  }

  // init staggered PRT moments module

  RadarMoments mom;
  mom.initStagPrt(_args.prt1,
                  _args.prt2,
                  stagM,
                  stagN,
                  1000,
                  1000,
                  wavelengthM,
                  0.5, 1.0);
  
  cerr << "  nyquist (m/s): " << mom.getNyquist() << endl;
  cerr << "  nyquistPrtShort (m/s): " << mom.getNyquistPrtShort() << endl;
  cerr << "  nyquistPrtLong  (m/s): " << mom.getNyquistPrtLong() << endl;

  return 0;

}

