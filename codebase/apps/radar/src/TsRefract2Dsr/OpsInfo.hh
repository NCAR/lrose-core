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

  // get methods

  double getStartRange() const { return _startRange; }
  double getGateSpacing() const { return _gateSpacing; }

  int getSweepNum() const { return _iSweep; }
  const string &getSiteName() const { return _sSiteName; }
  double getPulseWidthUs() const { return _fPWidthUSec; }
  double getClockMhz() const { return _fSyClkMHz; }
  double getRadarWavelengthCm() const { return _fWavelengthCM; }
  double getDbz0() const { return _fDBzCalib; }
  double getNoiseDbm0() const { return _fNoiseDBm[0]; }
  double getNoiseDbm1() const { return _fNoiseDBm[1]; }
  double getNoiseSdev0() const { return _fNoiseStdvDB[0]; }
  double getNoiseSdev1() const { return _fNoiseStdvDB[1]; }
  double getSaturationDbm() const { return _fSaturationDBM; }
  double getSaturationMult() const { return _fSaturationMult; }
  double getNoiseDbzAt1km() const { return _fDBzCalib; }

  // polarization type: H, V, Dual_alt, Dual_simul

  const string &getPolarizationType() const
  { return _polarizationType; }

protected:
  
private:

  const Params &_params;

  // RVP8 pulse info fields

  int _iVersion;
  int _iMajorMode;
  int _iPolarization;
  int _iPhaseModSeq;
  int _iSweep;
  int _iAuxNum;
  string _sTaskName;
  string _sSiteName;
  int _iAqMode;
  int _iUnfoldMode;
  int _iPWidthCode;
  double _fPWidthUSec;
  double _fDBzCalib;
  int _iSampleSize;
  int _iMeanAngleSync;
  int _iFlags;
  int _iPlaybackVersion;
  double _fSyClkMHz;
  double _fWavelengthCM;
  double _fSaturationDBM;
  double _fSaturationMult;
  double _fRangeMaskRes;
  int _iRangeMask[512];
  double _fNoiseDBm[2];
  double _fNoiseStdvDB[2];
  double _fNoiseRangeKM;
  double _fNoisePRFHz;
  int _iGparmLatchSts[2];
  int _iGparmImmedSts[6];
  int _iGparmDiagBits[4];
  string _sVersionString;

  // derived from info

  double _startRange;  // km
  double _maxRange;  // km
  double _gateSpacing; // km
  string _polarizationType; // H, V, HV_alternating, HV_simultaneous

  // functions

  int _readPulseInfo(FILE *in);
  void _deriveFromPulseInfo();
  void _printPulseInfo(ostream &out) const;
  void _printDerived(ostream &out) const;

};

#endif

