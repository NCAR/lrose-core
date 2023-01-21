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
///////////////////////////////////////////////////////////
// Calibration.hh
// 
// Singleton - read and store calibration data
//
// RAP, NCAR, Boulder CO
//
// August 2007
//
// Mike Dixon
//
///////////////////////////////////////////////////////////

#ifndef Calibration_HH
#define Calibration_HH

#include "Params.hh"
#include "Beam.hh"
#include <cstdio>
#include <radar/IwrfCalib.hh>
#include <map>
#include <pthread.h>

typedef multimap<time_t, string, less<time_t> > FileMap;
typedef pair<const time_t, string > FilePair;

using namespace std;

class Calibration
{

public:
  
  // Constructor

  Calibration(const Params &params);

  // Destructor
  
  ~Calibration();
  
  // Read calibration for a given file.
  // Returns 0 on success, -1 on failure
  
  int readCal(const string &filePath);

  // Load calibration appropriate to a given beam
  // Returns 0 on success, -1 on failure
  
  int loadCal(Beam *beam);
  
  // is calibration data available?

  bool isCalAvailable() const { return _calAvailable; }
  
  // get current cal values

  const IwrfCalib &getIwrfCalib() const;

private:

  // data

  const Params &_params;
  
  bool _calAvailable;
  string _calFilePath;

  double _prevPulseWidth;
  string _calDirForPulseWidth;

  iwrf_xmit_rcv_mode_t _prevXmitRcvMode;

  IwrfCalib _calib;
  string _radarName;
  time_t _calTime;

  time_t _noiseMonTime;
  double _noiseMonZdr;
  double _noiseMonDbmhc;
  double _noiseMonDbmvc;
  bool _useNoiseMonCalib;
  IwrfCalib _noiseMonCalib;
  
  // functions
  
  void _setCalFromTimeSeries(Beam *beam);
  int _checkPulseWidthAndRead(Beam *beam);
  int _readCal(time_t utime, const string &calDir);
  int _readCalFromFile(const string &calPath);
  int _compileFileList(const string &dirPath, FileMap &fileMap);
  iwrf_xmit_rcv_mode_t _getXmitRcvMode(Params::xmit_rcv_mode_t mode);
  void _applyCorrections();
  double _getValFromXml(const string &xml, const string &tag) const;
  int _adjustCalGainFromNoiseMon(Beam *beam);
  
};


#endif
