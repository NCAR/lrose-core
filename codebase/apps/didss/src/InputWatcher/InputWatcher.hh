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
// InputWatcher.hh
//
// InputWatcher object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////

#ifndef InputWatcher_H
#define InputWatcher_H

#include <string>
#include <map>
#include <dataport/port_types.h>
#include <didss/DsInputPath.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

// typedefs for map of active children

typedef pair<pid_t, time_t> activePair_t;
typedef map <pid_t, time_t, less<pid_t> > activeMap_t;

////////////////////////
// This class

class InputWatcher {
  
public:

  // constructor

  InputWatcher (int argc, char **argv);

  // destructor
  
  ~InputWatcher();

  // run 

  int Run();

  // data members

  bool isOK;

  // kill remaining children

  void killRemainingChildren();

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  activeMap_t _active;

  void _registerWithDataMapper(time_t modTime);

  int _writeLdataInfoFile(const char *inputPath, time_t modTime);
  
  int _copyFile(const char *inputPath, time_t modTime);

  void _callDataArrivedScript(const char *inputPath, time_t modTime);
  
  void _callDataLateScript();

  void _callScript(bool run_in_background,
		   const vector<string> &args,
		   const char *script_to_call);

  void _execScript(const vector<string> &args,
		   const char *script_to_call);

  void _reapChildren();

  void _killAsRequired(pid_t pid, time_t terminate_time);

};

#endif

