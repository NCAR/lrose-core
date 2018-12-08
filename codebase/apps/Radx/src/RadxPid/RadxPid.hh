// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// RadxPid.hh
//
// RadxPid object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// RadxPid reads moments from Radx-supported format files, 
// computes the PID (and optionally KDP_ and writes out the
// results to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#ifndef RadxPid_H
#define RadxPid_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <deque>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class Worker;
class WorkerThread;
using namespace std;

class RadxPid {
  
public:

  // constructor
  
  RadxPid (int argc, char **argv);

  // destructor
  
  ~RadxPid();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  const NcarPidParams &getPidFiltParams() const { return _ncarPidParams; }
  double getRadarHtKm() const { return _radarHtKm; }
  double getWavelengthM() const { return _wavelengthM; }

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  NcarPidParams _ncarPidParams;
  KdpFiltParams _kdpFiltParams;
  vector<string> _readPaths;

  // radar volume container
  
  RadxVol _vol;

  // derived rays - after compute

  vector <RadxRay *> _derivedRays;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  //////////////////////////////////////////////////////////////
  // inner thread class for calling Moments computations
  
  pthread_mutex_t _debugPrintMutex;
  
  // instantiate thread pool for computations

  TaThreadPool _threadPool;
  vector<Worker *> _workers;

  // private methods
  
  void _printParamsPid();
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _writeVol();
  int _processFile(const string &filePath);
  void _encodeFieldsForOutput();
  
  int _compute();
  int _storeDerivedRay(WorkerThread *thread);

};

#endif
