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
// NetCDF2Spdb.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////
//
// NetCDF2Spdb reads met data in netCDF, interprets it and
// writes it to SPDB
//
////////////////////////////////////////////////////////////////

#ifndef NetCDF2Spdb_H
#define NetCDF2Spdb_H

#include <string>
#include <map>
#include <deque>
#include <vector>
#include <Spdb/DsSpdb.hh>
#include <Ncxx/NcxxDim.hh>
#include <didss/DsInputPath.hh>
#include "Args.hh"
#include "Params.hh"
#include "NcfWrapper.hh"
using namespace std;

////////////////////////
// This class

class NetCDF2Spdb {
  
public:
  
  // constructor
  
  NetCDF2Spdb(int argc, char **argv);

  // destructor
  
  ~NetCDF2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double _missing;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  NcfWrapper _ncf;
  DsInputPath *_input;

  NcxxDim _timeDim;
  size_t _nTimes;

  vector<time_t> _time;
  vector<double> _dtime;
  vector<double> _latitude;
  vector<double> _longitude;
  vector<double> _altitude;
  vector<double> _pressure;
  vector<double> _temp;
  vector<double> _dewpt;
  vector<double> _rh;
  vector<double> _mixingRatio;
  vector<double> _liquidWater;
  vector<double> _windDirn;
  vector<double> _windSpeed;
  vector<double> _windGust;
  vector<double> _trueAirspeed;
  vector<double> _groundSpeed;
  vector<double> _precipRate;
  vector<double> _precipAccum;
  vector<double> _visibility;
  vector<double> _ceiling;
  vector<double> _rvr;
  vector<double> _ltgAmplitude;
  vector<double> _ltgEllipseAngle;
  vector<double> _ltgSemiMajorAxis;
  vector<double> _ltgEccentricity;
  vector<double> _ltgChisq;

  int _processFile(const string &path);
  int _readTimes();
  int _readVars();
  int _readVar(const char *varName, vector<double> &loc);
  void _clearData();
  void _printData(ostream &out);
  
  int _writeSurface();
  int _writeSounding();
  int _writeLightning();
  int _writeAircraft();
  int _writeGenPt();

};

#endif

