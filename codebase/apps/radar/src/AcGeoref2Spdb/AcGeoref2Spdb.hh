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
// AcGeoref2Spdb.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////
//
// AcGeoref2Spdb reads aircraft georeference data
// (posn, attitude, motion etc) from IWRF time series and
// netcdf files, and writes the data to SPDB.
//
////////////////////////////////////////////////////////////////

#ifndef AcGeoref2Spdb_H
#define AcGeoref2Spdb_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/IwrfCalib.hh>
#include <radar/FindSurfaceVel.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxTime.hh>
#include <rapformats/ac_georef.hh>
#include <Spdb/DsSpdb.hh>
#include <NcUtils/Ncxx.hh>

using namespace std;
using namespace netCDF;

////////////////////////
// This class

class AcGeoref2Spdb {
  
public:

  // constructor

  AcGeoref2Spdb (int argc, char **argv);

  // destructor
  
  ~AcGeoref2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // reading iwrf pulses

  IwrfTsReader *_pulseReader;
  si64 _prevPulseSeqNum;
  ssize_t _totalPulseCount;
  ssize_t _pulseCount;

  // reading netcdf file
  
  NcxxFile _file;
  string _errStr;

  string _callSign, _projectName, _flightNum;
  string _timeCoordName, _latCoordName, _lonCoordName;

  NcxxVar _timeVar;
  DateTime _timeBase;
  TaArray<time_t> _times_;
  size_t _nTimes;
  time_t *_times;

  static double _missingDbl;

  TaArray<double> _lat;
  TaArray<double> _lon;
  TaArray<double> _altMsl;
  TaArray<double> _altPres;
  TaArray<double> _ewVel;
  TaArray<double> _nsVel;
  TaArray<double> _vertVel;
  TaArray<double> _ewWind;
  TaArray<double> _nsWind;
  TaArray<double> _vertWind;
  TaArray<double> _heading;
  TaArray<double> _drift;
  TaArray<double> _track;
  TaArray<double> _roll;
  TaArray<double> _pitch;
  TaArray<double> _temp;
  TaArray<double> _pressure;
  TaArray<double> _rh;
  TaArray<double> _aoa;
  TaArray<double> _ias;
  TaArray<double> _tas;
  TaArray<double> _accelNorm;
  TaArray<double> _accelLat;
  TaArray<double> _accelLon;
  vector<TaArray<double> > _customFields;

  time_t _iwg1FlightTimeStart;
  
  // writing out

  DsSpdb _spdb;

  // printing surface velocity data

  FindSurfaceVel _surfVel;
  RadxTime _timeLastVelPrint;
  double _velStatsCount;
  ac_georef_t _velStatsSum;
  
  // methods

  int _runIwrfMode();
  int _runCfradialMode();
  int _runRafNetcdfMode();
  int _runRafIwg1UdpMode();
  int _runRadxFmqMode();

  IwrfTsPulse *_getNextPulse();

  void _convertFromIwrfGeoref(const iwrf_platform_georef_t &iwrfGeoref,
                              ac_georef_t &acGeoref);

  void _convertFromRadxGeoref(const RadxGeoref &radxGeoref,
                              ac_georef_t &acGeoref);
  int _processCfRadialFile(const string &path);
  void _handleRay(const RadxRay &ray, double cmigitsTemp, double tailconeTemp);
  int _processRafNetcdfFile(const string &path);
  int _readGlobalAttr();
  int _readTimes();
  int _readTimeSeriesVars();
  int _readTimeSeriesVar(TaArray<double> &array,
                         string varName);
  void _setTimeSeriesVarToMissing(TaArray<double> &array);

  int _doWrite();

  void _getHcrTempsFromStatusXml(const string &statusXml, 
                                 double &cmigitsTemp, 
                                 double &tailconeTemp);

  double _computeSurfaceVel(const RadxRay &ray);

  int _handleIwg1(const char *buf, int bufLen);

  double _decodeIwg1Field(const vector<string> &toks,
                          int tokNum);

  void _printSurfVelStats(const RadxRay &ray, const ac_georef_t &georef);

};

#endif
