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
// Taf2Spdb.hh
//
// Taf2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#ifndef Taf2Spdb_H
#define Taf2Spdb_H

#include <string>
#include <dataport/port_types.h>
#include <didss/DsInputPath.hh>
#include <Spdb/StationLoc.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class Input;
class Location;
class DsSpdb;

////////////////////////
// This class

class Taf2Spdb {
  
public:

  // constructor

  Taf2Spdb (int argc, char **argv);

  // destructor
  
  ~Taf2Spdb();

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

  // station location

  StationLoc _stationLoc;
  
  // Input objects

  DsInputPath *_inputPath;
  Input *_input;

  int _processFile (const char *file_path, time_t input_file_time);

  void _addTafChunk(const string &tafStr,
		    const string &name,
		    time_t store_time,
		    DsSpdb &spdb);
  
  int _writeTafs2Spdb(DsSpdb &spdb);

  void _tokenize(const string &str, const string &spacer,
		 vector<string> &toks);

};

#endif

