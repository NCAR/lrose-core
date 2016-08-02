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
// StormIdent.hh
//
// StormIdent object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef StormIdent_H
#define StormIdent_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <titan/storm.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// forward declarations

class FileLock;
class Shmem;
class DataTimes;
class InputMdv;
class TimeList;
class StormFile;
class Identify;

////////////////////////////////
// general defines

// delay on starting storm_track

#define STORM_TRACK_START_SECS 1

// restart flag

#define EXIT_AND_RESTART -999

// MAX_EIG_DIM is the maximum dimension which will be used in
// eigenvalue routines

#define MAX_EIG_DIM 3

// missing floating point flag value

#define MISSING_VAL -9999.0

////////////////////////
// This class

class StormIdent {
  
public:

  // constructor

  StormIdent (int argc, char **argv);

  // destructor
  
  ~StormIdent();

  // run 

  int Run();

  // data members

  int OK;
  Shmem *shMem;

  time_t getNewStartTime() { return _newStartTime; }

  bool getDebug() { return _params->debug; }

protected:
  
private:

  char *_progName;
  char *_paramsPath;
  Args *_args;
  Params *_params;

  char *_headerFilePath;

  FileLock *_fileLock;
  DataTimes *_dataTimes;
  InputMdv *_inputMdv;
  TimeList *_timeList;
  StormFile *_stormFile;
  Identify *_identify;

  time_t _newStartTime;

  void _printStartupMessage();
  int _runArchive(int prev_scan_num,
		  time_t start_time, time_t end_time,
		  int time_list_start_posn);
  int _runRealtime(int prev_scan_num, time_t end_time,
		   int time_list_start_posn);
  void _loadHeaderFilePath(const time_t ftime);
  void _loadStormParams(storm_file_params_t *sparams);
  int _prepareRestart();
  int _prepareRestartStormFile(time_t restart_time,
			       time_t new_start_time);

};

#endif

