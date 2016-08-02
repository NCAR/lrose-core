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
// QuadratureTest.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////
//
// QuadratureTest tests computing IQ from data samples
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <cmath>
#include <vector>
#include <radar/RadarComplex.hh>
#include "QuadratureTest.hh"

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

using namespace std;

// Constructor

QuadratureTest::QuadratureTest(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "QuadratureTest";

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

QuadratureTest::~QuadratureTest()

{

}

//////////////////////////////////////////////////
// Run

int QuadratureTest::Run ()
{

  if (_args.debug) {
    cerr << "Running QuadratureTest - debug mode" << endl;
  }

  int nSamples = _args.nSamples;
  double actualFreqMhz = _args.actualFreqMhz;
  double samplingFreqMhz = _args.samplingFreqMhz;
  double startPhaseDeg = _args.startPhaseDeg;
  double peakVal = _args.peakVal;
  double freqRatio = actualFreqMhz / samplingFreqMhz;

  cerr << "=======================================" << endl;
  cerr << "  nSamples: " << nSamples << endl;
  cerr << "  actualFreqMhz: " << actualFreqMhz << endl;
  cerr << "  samplingFreqMhz: " << samplingFreqMhz << endl;
  cerr << "  startPhaseDeg: " << startPhaseDeg << endl;
  cerr << "  peakVal: " << peakVal << endl;
  cerr << "  freqRatio: " << freqRatio << endl;
  cerr << "=======================================" << endl;

  // compute simulated samples

  vector<double> samp;
  vector<double> phase;
  for (int ii = 0; ii < nSamples; ii++) {
    double phaseDeg = startPhaseDeg + ii * 90.0 * freqRatio;
    double phaseRad = phaseDeg * DEG_TO_RAD;
    double val = peakVal * sin(phaseRad);
    samp.push_back(val);
    phase.push_back(phaseDeg);
  }

  cerr << "==========================================" << endl;

  for (size_t ii = 0; ii < samp.size(); ii++) {
    cerr << "ii, phase, val: "
	 << ii << ", " 
	 << fmod(phase[ii], 360.0) << ", " 
	 << samp[ii] << endl;
  }

  cerr << "==========================================" << endl;

  int niq = nSamples/ 2;
  int count = 0;
  RadarComplex_t *iq = new RadarComplex_t[niq];

  for (size_t ii = 0; ii < samp.size(); ii += 4) {

    double v1 = samp[ii];
    double v2 = samp[ii+1];
    double v3 = samp[ii+2];
    double v4 = samp[ii+3];

    double mag1 = sqrt(v1 * v1 + v2 * v2);
    double mag2 = sqrt(v3 * v3 + v4 * v4);

    double phase1 = asin(v1 / mag1) * RAD_TO_DEG;
    double phase2 = asin(v3 / mag2) * RAD_TO_DEG;

    double phase11;
    if (v1 >= 0 && v2 >= 0) {
      phase11 = phase1;
    } else if (v1 >= 0 && v2 < 0) {
      phase11 = 180 - phase1;
    } else if (v1 < 0 && v2 < 0) {
      phase11 = fabs(phase1) + 180.0;
    } else if (v1 < 0 && v2 >= 0) {
      phase11 = 360 + phase1;
    }

    double phase22;
    if (v3 >= 0 && v4 >= 0) {
      phase22 = fabs(phase2) + 180;
    } else if (v3 >= 0 && v4 < 0) {
      phase22 = 360 - phase2;
    } else if (v3 < 0 && v4 < 0) {
      phase22 = fabs(phase2);
    } else if (v3 < 0 && v4 >= 0) {
      phase22 = 180 + phase2;
    }

    cerr << "ii, mag1, mag2, phase1, phase11, phase2, phase22: "
	 << ii << ", " 
	 << mag1 << ", " 
	 << mag2 << ", " 
	 << phase1 << ", " 
	 << phase11 << ", "
	 << phase2 << ", " 
	 << phase22 << endl;

    RadarComplex_t iqVal;
    double sinVal, cosVal;

    sincos(phase11 * DEG_TO_RAD, &sinVal, &cosVal);
    iqVal.re = mag1 * sinVal;
    iqVal.im = mag1 * cosVal;
    iq[count] = iqVal;
    count++;

    sincos(phase22 * DEG_TO_RAD, &sinVal, &cosVal);
    iqVal.re = mag2 * sinVal;
    iqVal.im = mag2 * cosVal;
    iq[count] = iqVal;
    count++;
    
  }

  // compute mean phase difference

  RadarComplex_t meanDiff = RadarComplex::meanConjugateProduct(iq, iq + 1, niq - 1);
  double phaseDiffDeg = RadarComplex::argDeg(meanDiff);

  double estimatedFreq = samplingFreqMhz * ((180.0 + phaseDiffDeg) / 180.0);

  cerr << "====> mean phase diff: " << phaseDiffDeg << endl;
  cerr << "====> estimatedFreq: " << estimatedFreq << endl;

  cerr << "==========================================" << endl;

  return 0;

}

