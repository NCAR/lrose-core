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
// DoeSounding2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// DoeSounding2Spdb reads US DOE sounding data,
// converts them to sounding format and writes them
// to an SPDB data base.
//
////////////////////////////////////////////////////////////////

#ifndef DoeSounding2Spdb_H
#define DoeSounding2Spdb_H

#include <string>
#include <map>
#include <Ncxx/Nc3File.hh>
#include <Spdb/SoundingPut.hh>
#include <physics/IcaoStdAtmos.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class DoeSounding2Spdb {
  
public:

  // constructor

  DoeSounding2Spdb (int argc, char **argv);

  // destructor
  
  ~DoeSounding2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double _missingDouble;
  static const float _missingFloat;
  static const int _missingInt;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  IcaoStdAtmos _stdAtmos;

  // netcdf handles
  
  Nc3File *_ncFile;
  Nc3Error *_ncErr;
  
  // header info

  double _stationLat;
  double _stationLon;
  double _stationAlt;

  // dimensions

  Nc3Dim *_timeDim;
  int _nTimesInFile;

  // global attributes

  string _siteId;
  string _facilityId;
  string _inputSource;
  string _soundingNumber;
  string _serialNumber;

  // data

  time_t _baseTime;
  vector<time_t> _time;
  vector<double> _dtime;

  vector<double> _pressure;
  vector<double> _tempDry;
  vector<double> _dewPoint;
  vector<double> _rh;
  vector<double> _windSpeed;
  vector<double> _windDirn;
  vector<double> _uWind;
  vector<double> _vWind;
  vector<double> _ascentRate;
  vector<double> _latitude;
  vector<double> _longitude;
  vector<double> _altitude;

  int _processFile(const char *file_path);
  
  int _openNc(const string &path);
  void _closeNc();

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTime();
  
  int _readIntScalar(const string &name,
                     int &val, bool required = true);
  
  int _readFloatScalar(const string &name,
                       float &val, bool required = true);
  
  int _readDoubleScalar(const string &name,
                        double &val, bool required = true);
  
  int _readVector(const string &name,
                  Nc3Dim *dim,
                  vector<double> &val);

  int _readField(const string &name,
                 vector<double> &val);

  void _fillVectorMissing(Nc3Dim *dim,
                          vector<double> &val);

  string _asString(const Nc3TypedComponent *component,
                   int index = 0);
  
};

#endif

