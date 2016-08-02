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
// NcarAcPosn2Spdb.hh
//
// NcarAcPosn2Spdb object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2015
//
/////////////////////////////////////////////////////////////
//
// NcarAcPosn2Spdb reads aircraft position data from ASCII file in
// JSON format. These files updated frequently and are
// overwritten. They are read each time they update, and the position
// data is then stored in SPDB.
//
/////////////////////////////////////////////////////////////

#ifndef NcarAcPosn2Spdb_H
#define NcarAcPosn2Spdb_H

#include <string>
#include <map>
#include <deque>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
#include "Args.hh"
#include "Params.hh"
#include "Input.hh"
using namespace std;

// the following maps are used to keep track of the number
// of flares which are fired between data arrivals

typedef map< string, int, less<string> > flare_count_map_t;
typedef flare_count_map_t::iterator count_iter;
typedef pair< string, int > flare_pair_t;

// the following queue type is used to keep track of
// how many burn-in-place flares are burning for
// each aircraft

typedef pair< time_t, string > burn_pair_t;
typedef deque< burn_pair_t > burn_count_deque_t;
typedef burn_count_deque_t::iterator burn_iter;

class Socket;

////////////////////////
// This class

class NcarAcPosn2Spdb {
  
public:

  // constructor

  NcarAcPosn2Spdb (int argc, char **argv);

  // destructor
  
  ~NcarAcPosn2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const float POLCAST2_MISSING_VALUE;
  
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  int _runRealtimeJsonMode();
  int _runSimMode();

  int _checkForNewData(size_t index,
                       const string &callSign,
                       const string &inputPath,
                       vector<time_t> &prevModTimes,
                       vector<time_t> &prevRepeatTimes);

  int _decodeAndWrite(string callSign,
                      const char *line,
                      bool useNow,
                      time_t now);

};

#endif

