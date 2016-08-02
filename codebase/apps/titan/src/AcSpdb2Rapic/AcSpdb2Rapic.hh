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
// AcSpdb2Rapic.hh
//
// AcSpdb2Rapic object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
/////////////////////////////////////////////////////////////
//
// AcSpdb2Rapic reads aircraft data from SPDB, and reformats it
// for use in RAPIC. RAPIC is a radar display tool from the
// Australian Bureau of Meteorology
//
/////////////////////////////////////////////////////////////

#ifndef AcSpdb2Rapic_H
#define AcSpdb2Rapic_H

#include <string>
#include <map>
#include <deque>
#include <set>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
#include "Args.hh"
#include "Params.hh"
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

class AcSpdb2Rapic {
  
public:

  // constructor

  AcSpdb2Rapic (int argc, char **argv);

  // destructor
  
  ~AcSpdb2Rapic();

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

  typedef struct {
    string callsign;
    time_t time;
    double lat;
    double lon;
    double alt;
    double heading;
    double computedHeading;
  } acLocation_t;

  typedef enum {
    LATEST_FILE, TODAY_FILE, DAILY_FILE
  } loc_file_type_t;
  
  vector<acLocation_t> _locArray;
  set<string> _callSigns;
  
  int _runRealtime();
  int _runArchive();
  int _writeDaily(time_t refTime);
  int _writeLatest(time_t refTime);

  int _writeDailyFile(int dayNum);
  int _writeFile(const string &callsign, time_t startThisDay, loc_file_type_t fileType);

  int _loadPosnData(time_t startTime, time_t endTime);

  void load_ac_posn(const Spdb::chunk_t &chunk);
  void load_ac_posn_wmod(const Spdb::chunk_t &chunk);
  void load_ac_data(const Spdb::chunk_t &chunk);

};

#endif

