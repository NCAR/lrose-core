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
// RadxClutter.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2023
//
///////////////////////////////////////////////////////////////
//
// RadxClutter identifies persistent clutter in polar radar data,
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

#ifndef RadxClutter_HH
#define RadxClutter_HH

#include "Args.hh"
#include "Params.hh"
#include "Histo.hh"
#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/TaArray2D.hh>
class RadxFile;
using namespace std;

class RadxClutter {
  
public:

  // constructor
  
  RadxClutter (int argc, char **argv);

  // destructor
  
  ~RadxClutter();

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

  // clutter volume, derived from the volume read in

  RadxVol _clutterVol;
  size_t _nRaysClutter;

  // analysis results - statistics

  TaArray2D<Radx::fl32> _dbzSumArray;
  Radx::fl32 **_dbzSum;

  TaArray2D<Radx::fl32> _dbzSqSumArray;
  Radx::fl32 **_dbzSqSum;

  TaArray2D<Radx::fl32> _dbzCountArray;
  Radx::fl32 **_dbzCount;

  TaArray2D<Radx::fl32> _dbzMeanArray;
  Radx::fl32 **_dbzMean;

  TaArray2D<Radx::fl32> _dbzSdevArray;
  Radx::fl32 **_dbzSdev;

  TaArray2D<Radx::fl32> _clutSumArray;
  Radx::fl32 **_clutSum;

  TaArray2D<Radx::fl32> _clutFreqArray;
  Radx::fl32 **_clutFreq;

  TaArray2D<Radx::fl32> _clutFlagArray;
  Radx::fl32 **_clutFlag;

  // clutter frequency histogram

  Histo _clutFreqHist;

  // methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _readFile(const string &filePath);
  int _processFile(const string &filePath);

  int _checkGeom();
  int _initClutterVol();
  void _initAngleList();

  int _analyzeClutter();

  void _setupWrite(RadxFile &file);
  int _writeClutterVol();


};

#endif
