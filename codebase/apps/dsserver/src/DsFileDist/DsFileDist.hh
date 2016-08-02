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
// DsFileDist.h
//
// DsFileDist object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#ifndef DsFileDist_H
#define DsFileDist_H

#include <tdrp/tdrp.h>
#include <toolsa/fmq.h>
#include <string>
#include <map>
#include <set>
#include <list>
#include "Args.hh"
#include "Params.hh"
#include <dsserver/DsFileCopy.hh>

using namespace std;

class Watcher;
class PutArgs;

typedef pair<string, Watcher *> activePair_t;
typedef map <string, Watcher *, less<string> > activeMap_t;

typedef pair<string, Params> dirForLdataInfoPair_t;
typedef map <string, Params, less<string> > dirForLdataInfoMap_t;

class Args;
class Params;

class DsFileDist {
  
public:

  // constructor

  DsFileDist (int argc, char **argv);

  // destructor
  
  ~DsFileDist();

  // run 

  int Run();

  // data members

  int isOK;

  // static method to handle receipt of HUP singnal by setting flag
  // this will be acted on later at a convenient point
  
  static void handleHup(int sig) { _gotHup = true; }

  // static method to be called from child process
  // to put a file

  static void *put(void *args);

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  dirForLdataInfoMap_t _dirForLdataInfoMap;
  activeMap_t _activeMap;
  list<PutArgs *> _putList;
  int _childCount;
  double _lastDataCheckTime;

  FMQ_handle_t _errorFmq;
  string _errorFmqPath;

  static bool _gotHup;

  int _checkDirsForParams();
  void _checkDirsForLdataInfo();

  int _checkDirForParams(const string &dirPath, int level);
  int _checkDirForLdataInfo(const string &dirPath,
                            const string &paramsDir,
                            const Params &paramsInUse);

  bool _activateFromParams(const string &dirPath);

  bool _activateFromLdataInfo(const string &dirPath,
                              const string &paramsDir,
                              const Params &paramsInUse);
  
  int _checkForNewData();
  
  void _purgeActiveParamsMap();
  void _purgeCompletedPuts();
  void _purgeAgedPuts();
  void _checkErrorFmq();

  int _putUnthreaded();
  int _putThreaded();

  int _countSimulToHost(const string &host);

  // static functions for calls from child process

  static int _putForced(const PutArgs &putArgs,
			const string &dirName,
			DsFileCopy &copy,
                        string &errStr);

  static int _putWithEnquire(const PutArgs &putArgs,
			     const string &dirName,
			     DsFileCopy &copy,
                             string &errStr);

  static int _checkFileAge(const PutArgs &putArgs,
                           string &errStr);
  
  static void _log(const string &label,
		   const string &host,
		   int nbytes,
		   const string &file);
  
  static void _writeErrorFmq(const PutArgs &putArgs,
                             const string &errStr);
  
};

#endif
