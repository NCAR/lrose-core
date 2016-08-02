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
// Metar2Spdb.hh
//
// Metar2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2001
//
///////////////////////////////////////////////////////////////
//
// Metar2Spdb reads ASCII files containing METAR data, converts the
// data to station_report_t, and writes to SPDB.
//
///////////////////////////////////////////////////////////////////////

#ifndef Metar2Spdb_H
#define Metar2Spdb_H

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>
#include <rapformats/metar.h>
#include <rapformats/station_reports.h>
#include <rapformats/WxObs.hh>
#include "Args.hh"
#include "Params.hh"
#include "StationLoc.hh"
using namespace std;

////////////////////////
// This class

class Metar2Spdb {
  
public:

  // constructor

  Metar2Spdb (int argc, char **argv);

  // destructor
  
  ~Metar2Spdb();

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

  map< string, StationLoc, less<string> > _locations;

  int _processFile(const char *file_path, time_t file_time);

  int _loadLocations(const char* station_location_path);
  
  bool _startOfSABlock(const char *line,
		       int &blockHour, int &blockMin,
		       int &blockDate);

  int _decodeMetar(const char *file_path,
		   time_t file_time,
		   int blockHour,
		   int blockMin,
		   int blockDate,
		   const string &metarMessage,
		   const string &reportType,
                   string &stationName,
                   MemBuf &buf,
		   time_t &valid_time);
  
  int _loadReport(const Decoded_METAR &dcdMetar,
                  time_t valid_time,
                  double lat,
                  double lon,
                  double alt,
                  MemBuf &buf);
  
  int _loadWxObs(const string &metarText,
		 const string &reportType,
                 const string &stationName,
                 const Decoded_METAR &dcdMetar,
                 time_t valid_time,
                 double lat,
                 double lon,
                 double alt,
                 MemBuf &buf);

  int _readThompsonLocation(const char* line,
                            char *station,
                            double &lat, 
                            double &lon,
                            double &alt);
  
  int _doPut(DsSpdb &spdbDecoded, DsSpdb &spdbAscii);

};

#endif

