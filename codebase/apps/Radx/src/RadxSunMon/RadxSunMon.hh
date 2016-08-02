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
// RadxSunMon.hh
//
// RadxSunMon object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////
//
// Searches for sun-spikes in radar rays, compute sun stats
// and saves to SPDB
//
////////////////////////////////////////////////////////////////

#ifndef RadxSunMon_HH
#define RadxSunMon_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <radar/InterestMap.hh>
#include <radar/NoiseLocator.hh>
#include <radar/RadarMoments.hh>
#include <radar/AtmosAtten.hh>
#include <radar/IwrfCalib.hh>
#include <euclid/SunPosn.hh>
class RadxFile;
class RadxRay;
using namespace std;

class RadxSunMon {
  
public:

  // constructor
  
  RadxSunMon (int argc, char **argv);

  // destructor
  
  ~RadxSunMon();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  string _momentsPath;
  RadxVol _momentsVol;

  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth, _prevAz;
  double _elevation, _prevEl;
  
  // calibration
  
  IwrfCalib _calib;

  // noise finding

  NoiseLocator _noise;

  // sun location

  SunPosn _sunPosn;
  double _elSun, _azSun;

  // moments computations object

  RadarMoments _rmom;
  AtmosAtten _atmosAtten;
  
  // moments field data

  RadxTime _rayTime;
  size_t _nGates;
  double _radarHtKm;
  double _startRangeKm, _gateSpacingKm;
  TaArray<MomentsFields> _momFields;
  double _wavelengthM;
  double _nyquist;
  double _radarConstDb;

  // angular error stats

  double _elOffset, _azOffset, _angleOffset;
  double _offsetWeight;
  double _sumOffsetWeights;
  double _sumElOffset;
  double _sumAzOffset;
  double _meanElOffset;
  double _meanAzOffset;

  // measured and estimated powers

  double _measuredDbmHc;
  double _measuredDbmVc;
  double _measuredDbmHx;
  double _measuredDbmVx;

  double _correctedDbmHc;
  double _correctedDbmVc;
  double _correctedDbmHx;
  double _correctedDbmVx;
  
  double _oneWayAtmosAttenDb;
  double _offSunPowerCorrectionDb;
  double _appliedPowerCorrectionDb;
  
  double _SS, _S1S2;

  int _nXpolRatio;
  double _sumXpolRatioDb;
  double _meanXpolRatioDb;
  double _zdrmDb;

  double _siteTempC;
  time_t _timeForSiteTemp;

  // methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  int _readMoments(const string &filePath);
  
  int _processVol();
  int _processRay(const RadxRay &ray);
  
  int _noiseInit();

  int _convertInterestParamsToVector(const string &label,
                                     const Params::interest_map_point_t *map,
                                     int nPoints,
                                     vector<InterestMap::ImPoint> &pts);
    
  double _getPhaseFromVel(double vel);
  double _estimateNcpFromWidth(double width);
  double _estimatePowerFromDbz(double dbz, double elev, double rangeKm);

  double _computeDwellPowerCorrection();
  double _getPowerCorrection(double angleOffset);

  int _computeXpolRatio();
  int _processRayForXpolRatio(RadxRay *ray);

  int _retrieveSiteTempFromSpdb(const RadxVol &vol,
                                double &tempC,
                                time_t &timeForTemp);

  int _writeResultsToSpdb();

};

#endif
