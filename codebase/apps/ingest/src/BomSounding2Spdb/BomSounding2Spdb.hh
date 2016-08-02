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
// BomSounding2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2009
//
///////////////////////////////////////////////////////////////
//
// BomSounding2Spdb reads Australian BOM sounding data,
// converts them to sounding format and writes them
// to an SPDB data base.
//
////////////////////////////////////////////////////////////////

#ifndef BomSounding2Spdb_H
#define BomSounding2Spdb_H

#include <string>
#include <map>
#include <Spdb/SoundingPut.hh>
#include <physics/IcaoStdAtmos.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class BomSounding2Spdb {
  
public:

  // constructor

  BomSounding2Spdb (int argc, char **argv);

  // destructor
  
  ~BomSounding2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double MissingVal;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  IcaoStdAtmos _stdAtmos;

  // header info

  time_t _aifsTime;
  int _stationId;
  string _stationName;
  int _nLevels;
  int _nFields;
  int _wUnits;

  double _stationLat;
  double _stationLon;
  double _stationAlt;

  // data

  vector<double> _height;
  vector<double> _u;
  vector<double> _v;
  vector<double> _w;
  vector<double> _prs;
  vector<double> _relHum;
  vector<double> _temperature;

  int _processFile(const char *file_path);
  int _readHeader(FILE *in, SoundingPut &sounding);
  int _readData(FILE *in, SoundingPut &sounding);
  
  string _parseStringVal(const string &line);
  int _parseIntVal(const string &line);

};

#endif

