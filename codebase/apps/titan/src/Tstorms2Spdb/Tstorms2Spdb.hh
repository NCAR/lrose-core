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
// Tstorms2Spdb.hh
//
// Tstorms2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2002
//
///////////////////////////////////////////////////////////////
//
// Tstorms2Symprod reads native TITAN data files, converts the
// data into rapformats/tstorm_spdb.h style structs and writes
// the data out to SPDB.
//
////////////////////////////////////////////////////////////////

#ifndef Tstorms2Spdb_H
#define Tstorms2Spdb_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <didss/DsInputPath.hh>
#include <rapformats/tstorm_spdb.h>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
using namespace std;

////////////////////////
// This class

class Tstorms2Spdb {
  
public:

  // constructor

  Tstorms2Spdb (int argc, char **argv);

  // destructor
  
  ~Tstorms2Spdb();

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

  DsInputPath *_input;

  int _processTrackFile (const char *input_file_path);

  int _openFiles(const char *input_file_path,
		 TitanTrackFile &tFile,
		 TitanStormFile &sFile);

  int _loadScanTimes(TitanStormFile &sFile,
		     vector<time_t> &scanTimes);

  int _processScan(TitanStormFile &sFile,
		   TitanTrackFile &tFile,
		   int scan_num,
		   time_t valid_time,
		   time_t expire_time);

};

#endif

