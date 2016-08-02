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
// UAEMesonet2Spdb.hh
//
// Dan Megenhardt, RAL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2015
//
///////////////////////////////////////////////////////////////
//
// UAEMesonet2Spdb reads surface weather observations from the
// UAE Mesonet, converts them to station_report_t
// format and writes them to an SPDB data base
//
////////////////////////////////////////////////////////////////

#ifndef UAEMesonet2Spdb_H
#define UAEMesonet2Spdb_H

#include <string>
#include <dataport/port_types.h>
#include "Args.hh"
#include "Params.hh"
using namespace std;
class WxObs;

////////////////////////
// This class

class UAEMesonet2Spdb {
  
public:

  // constructor

  UAEMesonet2Spdb (int argc, char **argv);

  // destructor

  ~UAEMesonet2Spdb();

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

  // class for station location

  class StationLoc {
  public:
    string id;
    double latitude;
    double longitude;
  };

  vector<StationLoc> _stationLocs;

  int _processFile(const char *file_path);
  int _fillObs(const vector<string> &toks, WxObs &obs, si32 &stationId);
  int _decodeInt(const string &tok);
  double _decodeDouble(const string &tok);
  int _readStationLocationFile(const string &path);
  
};

#endif

