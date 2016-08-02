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
// ShipObsIngest.hh
//
// ShipObsIngest object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2011
//
///////////////////////////////////////////////////////////////
//
// ShipObsIngest reads NOAA ship obs from ASCII files, decodes
// them and writes them out to SPDB.
//
////////////////////////////////////////////////////////////////

#ifndef ShipObsIngest_H
#define ShipObsIngest_H

#include <string>
#include <map>
#include <toolsa/MemBuf.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/metar.h>
#include <rapformats/station_reports.h>
#include <rapformats/WxObs.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class ShipObsIngest {
  
public:

  // constructor

  ShipObsIngest (int argc, char **argv);

  // destructor
  
  ~ShipObsIngest();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // data input

  DsInputPath *_input;
  
  // Spdb objects for writing

  DsSpdb _spdbDecoded;
  DsSpdb _spdbAscii;

  // date and time

  time_t _fileTime;
  time_t _obsTime;

  // fields

  string _callSign;
  bool _windSpeedIsMetric; // if false, in knots

  double _latitude;
  double _longitude;

  bool _precipOmitted;
  bool _wxIncluded;
  double _cloudHtMeters;
  double _visMeters;

  int _octas;
  double _windDirn;
  bool _windVariable;
  double _windSpeedKts;

  double _tempC;
  double _dewPtC;
  double _rh;
  double _pressureHpa;

  int _pressureTendCode; // 0 - 8
  int _pressureTendSign; // -1, 0, 1
  double _pressureTendHpa;

  int _presentWx;

  double _sstC;

  // buffer for report

  string _obsStr;
  MemBuf _buf;

  // methods

  int _processFile(const char *filePath);

  int _processMessage(const string &message);

  int _computeTimeFromHeader(const vector<string> &toks, size_t bbxxPos);
  int _computeTime(const string &tok);

  void _initReportVars();
  int _handleBlock(const vector<string> &toks);
  int _decodeObsBlock(const vector<string> &toks);
  int _decodeObsTime(const string &toks);
  int _decodeObsPos(const string &latTok, const string &lonTok);
  int _decodeObsVis(const string &tok);
  int _decodeObsWind(const string &tok);
  int _decodeObsHighWindSpeed(const string &tok);
  int _decodeObsTemp(const string &tok);
  int _decodeObsDewPt(const string &tok);
  int _decodeObsPressure(const string &tok);
  int _decodeObsPressureTendency(const string &tok);
  int _decodeObsWx(const string &tok);
  int _decodeObsSst(const string &tok);
  int _loadReport();
  int _loadWxObs();
  int _doPut();

};

#endif

