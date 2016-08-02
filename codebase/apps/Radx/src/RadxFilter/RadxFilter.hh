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
// RadxFilter.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2013
//
///////////////////////////////////////////////////////////////
//
// RadxFilter applies a filter to data in a Radx-supported file
// and writes out the filtered result.
//
///////////////////////////////////////////////////////////////

#ifndef RadxFilter_H
#define RadxFilter_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <deque>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class ComputeEngine;
class ComputeThread;
using namespace std;

class RadxFilter {
  
public:

  // constructor
  
  RadxFilter (int argc, char **argv);

  // destructor
  
  ~RadxFilter();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  pthread_mutex_t *getDebugPrintMutex() { return &_debugPrintMutex; }

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  // computations object

  ComputeEngine *_engine;

  // output rays - after compute

  vector <RadxRay *> _outputRays;

  // transitions - same size as outputRays

  vector<bool> _transitionFlags;

  // threading
  
  deque<ComputeThread *> _activeThreads;
  deque<ComputeThread *> _availThreads;
  pthread_mutex_t _debugPrintMutex;
  
  // private methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _processFile(const string &filePath);
  
  int _compute(RadxVol &vol);
  int _computeSingleThreaded(RadxVol &vol);
  int _computeMultiThreaded(RadxVol &vol);
  static void *_computeInThread(void *thread_data);
  
  void _findTransitions(vector<RadxRay *> &rays);

};

#endif
