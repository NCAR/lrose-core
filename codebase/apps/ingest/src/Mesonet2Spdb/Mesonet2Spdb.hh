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
// Mesonet2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////
//
// Mesonet2Spdb reads mesonet weather station surface
// observations, and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#ifndef Mesonet2Spdb_H
#define Mesonet2Spdb_H

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <rapformats/WxObs.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Mesonet2Spdb {
  
public:

  // constructor

  Mesonet2Spdb (int argc, char **argv);

  // destructor
  
  ~Mesonet2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  class StationLoc {
  public:
    StationLoc() {
      lat = -9999;
      lon = -9999;
      alt = -9999;
    }
    inline void set(const string& name,
                    float latitude, 
                    float longitude,
                    float altitude) {
      id  = name;
      lat = latitude;
      lon = longitude;
      alt = altitude;
    }
    string id;
    float lat;
    float lon;
    float alt;
  };

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  map< string, StationLoc, less<string> > _locations;

  int _processFile(const char *file_path);
  int _readFile(const char *file_path, DsSpdb &out);  
  bool _holdsFieldLabels(const vector<string> &toks);
  void _setFieldTypes(const vector<string> &toks,
                      vector<Params::field_type_t> &fieldTypes);
  int _decodeObservation(const vector<string> &toks,
                         vector<Params::field_type_t> &fieldTypes,
                         const DateTime &baseTime,
                         bool baseTimeFound,
                         WxObs &obs);
  int _loadLocations(const char* station_location_path);
  string _fieldType2Str(Params::field_type_t ftype);
  
};

#endif

