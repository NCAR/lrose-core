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
// RadxTimeStats.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2022
//
///////////////////////////////////////////////////////////////
//
// RadxTimeStats identifies persistent clutter in polar radar data,
// flags it, and writes out the statistics to a CfRadial file.
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Based on following paper:
// Lakshmanan V., J. Zhang, K. Hondl and C. Langston.
// A Statistical Approach to Mitigating Persistent Clutter in
// Radar Reflectivity Data.
// IEEE Journal of Selected Topics in Applied Earth Observations
// and Remote Sensing, Vol. 5, No. 2, April 2012.
////////////////////////////////////////////////////////////////

#ifndef RadxTimeStats_HH
#define RadxTimeStats_HH

#include "Args.hh"
#include "Params.hh"
#include "Histo.hh"
#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/TaArray2D.hh>
class RadxFile;
using namespace std;

class RadxTimeStats {
  
public:

  // constructor
  
  RadxTimeStats (int argc, char **argv);

  // destructor
  
  ~RadxTimeStats();

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
  string _readPath;

  /////////////////////////////////////////
  // input data

  RadxVol _readVol;
  bool _isRhi;
  vector<double> _fixedAngles;
  vector<double> _scanAngles;
  
  size_t _nGates;
  double _radxStartRange;
  double _radxGateSpacing;

  double _radarLatitude;
  double _radarLongitude;
  double _radarAltitude;
  
  size_t _nVols;
  
  // These are pointers into the input Radx object.
  // This memory is managed by the Radx class and should not be freed
  // by the calling class.

  const Radx::fl32 *_dbz;
  Radx::fl32 _dbzMiss;

  // stats volume, derived from the volume read in
  
  RadxVol _statsVol;
  size_t _nRaysStats;

  // analysis results - statistics

  bool _allocNeeded;
  
  TaArray2D<Radx::fl32> _sumArray;
  Radx::fl32 **_sum;

  TaArray2D<Radx::fl32> _sqSumArray;
  Radx::fl32 **_sqSum;

  TaArray2D<Radx::fl32> _countArray;
  Radx::fl32 **_count;
  
  TaArray2D<Radx::fl32> _meanArray;
  Radx::fl32 **_mean;

  TaArray2D<Radx::fl32> _sdevArray;
  Radx::fl32 **_sdev;
  
  TaArray2D<Radx::fl32> _modeArray;
  Radx::fl32 **_mode;
  
  TaArray2D<Radx::fl32> _medianArray;
  Radx::fl32 **_median;
  
  TaArray2D<Histo> _histoArray;
  Histo **_histo;

  // methods
  
  void _setupRead(RadxFile &file);
  int _readFile(const string &filePath);
  int _processFile(const string &filePath);

  void _initAngleList();
  
  int _checkGeom();
  int _initStatsVol();
  
  int _analyzeVol();
  
  void _setupWrite(RadxFile &file);
  int _writeStatsVol();
  
  RadxRay *_getStatsRay(RadxRay *ray);

  
};

#endif
