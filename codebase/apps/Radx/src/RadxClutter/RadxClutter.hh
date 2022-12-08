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
#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
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
  
  const static double missingVal;
  
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
  
  int _nGates;
  double _radxStartRange;
  double _radxGateSpacing;

  string _radarName;
  double _radarLatitude;
  double _radarLongitude;
  double _radarAltitude;
  
  // These are pointers into the input Radx object.
  // This memory is managed by the Radx class and should not be freed
  // by the calling class.

  const Radx::fl32 *_dbz;
  Radx::fl32 _dbzMiss;

  // methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _readFile(const string &filePath);
  int _processFile(const string &filePath);

  int _initClutterVol();
  void _initAngleList();

  int _processDataSet();

  void _writeLdataInfo(const string &outputPath);


};

#endif
