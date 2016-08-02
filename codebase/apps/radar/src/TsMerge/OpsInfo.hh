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
// OpsInfo.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////
//
// Stores current ops info
//
////////////////////////////////////////////////////////////////

#ifndef OpsInfo_hh
#define OpsInfo_hh

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class OpsInfo {
  
public:

  // constructor

  OpsInfo (const Params &params);

  // destructor
  
  ~OpsInfo();

  // read in info

  int read(FILE *in);
  
  // printing
  
  void print(ostream &out) const;

  // fields

  int iVersion;
  int iMajorMode;
  int iPolarization;
  int iPhaseModSeq;
  int iSweep;
  int iAuxNum;
  string sTaskName;
  string sSiteName;
  int iAqMode;
  int iUnfoldMode;
  int iPWidthCode;
  double fPWidthUSec;
  double fDBzCalib;
  int iSampleSize;
  int iMeanAngleSync;
  int iFlags;
  int iPlaybackVersion;
  double fSyClkMHz;
  double fWavelengthCM;
  double fSaturationDBM;
  double fSaturationMult;
  double fRangeMaskRes;
  int iRangeMask[512];
  double fNoiseDBm[2];
  double fNoiseStdvDB[2];
  double fNoiseRangeKM;
  double fNoisePRFHz;
  int iGparmLatchSts[2];
  int iGparmImmedSts[6];
  int iGparmDiagBits[4];
  string sVersionString;

  // derived from info

  double startRange;  // km
  double maxRange;  // km
  double gateSpacing; // km

protected:
  
private:

  const Params &_params;

  // functions

  int _readPulseInfo(FILE *in);
  void _deriveFromPulseInfo();
  void _printPulseInfo(ostream &out) const;
  void _printDerived(ostream &out) const;

};

#endif

